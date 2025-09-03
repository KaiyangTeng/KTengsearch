#include "DocumentStore.h"
#include <sstream>
#include <iostream>

// —————— 构造 / 析构 ————————————————————————
DocumentStore::DocumentStore(const std::string& host,
                             const std::string& user,
                             const std::string& passwd,
                             const std::string& db_name,
                             unsigned int       port,
                             size_t             cache_cap)
    : conn_(mysql_init(nullptr)), cap_(cache_cap)
{
    if (!conn_) {
        std::cerr << "[DocumentStore] mysql_init 失败\n";
        return;
    }
    if (!mysql_real_connect(conn_, host.c_str(), user.c_str(), passwd.c_str(),
                            db_name.c_str(), port, nullptr, 0))
    {
        std::cerr << "[DocumentStore] 连接 MySQL 失败: "
                  << mysql_error(conn_) << '\n';
        mysql_close(conn_);
        conn_ = nullptr;
    } else {
        mysql_query(conn_, "SET NAMES utf8mb4;");
    }
}

DocumentStore::~DocumentStore() {
    if (conn_) mysql_close(conn_);
}

// —————— 写接口 ————————————————————————————
bool DocumentStore::addDocument(const std::string& url,
                                const std::string& status,
                                const std::string& title,
                                const std::string& content)
{
    if (!conn_) return false;
    std::ostringstream sql;
    sql << "INSERT IGNORE INTO url_log(url,status,title,content,timestamp) VALUES('"
        << escape(url)     << "','"
        << escape(status)  << "','"
        << escape(title)   << "','"
        << escape(content) << "',NOW());";
    return mysql_query(conn_, sql.str().c_str()) == 0;
}

// —————— 读接口 ————————————————————————————
DocHeader DocumentStore::getHeader(int docID)
{
    std::lock_guard<std::mutex> lock(mtx_);

    // 1) 命中缓存
    auto it = cache_.find(docID);
    if (it != cache_.end()) {
        touchLRU(docID);
        return it->second.first.header;
    }

    // 2) 未命中 → 查询数据库
    DocHeader h{};
    if (!conn_) return h;

    std::ostringstream q;
    q << "SELECT url,status,title,timestamp,CHAR_LENGTH(content) "
      << "FROM url_log WHERE id=" << docID << " LIMIT 1;";
    if (mysql_query(conn_, q.str().c_str()) != 0) {
        std::cerr << "[DocumentStore] 查询失败: " << mysql_error(conn_) << '\n';
        return h;
    }
    MYSQL_RES* res = mysql_store_result(conn_);
    if (!res) return h;
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row) {
        h.docID          = docID;
        h.url            = row[0] ? row[0] : "";
        h.status         = row[1] ? row[1] : "";
        h.title          = row[2] ? row[2] : "";
        h.timestamp      = row[3] ? row[3] : "";
        h.content_length = row[4] ? std::stoul(row[4]) : 0;
    }
    mysql_free_result(res);

    // 3) 放入缓存（正文懒加载，此处留空）
    CacheEntry ce{h, ""};
    lru_.push_front(docID);
    cache_[docID] = {std::move(ce), lru_.begin()};
    evictIfNeeded();
    return h;
}

// std::string DocumentStore::getContent(int docID)
// {
//     std::lock_guard<std::mutex> lock(mtx_);

//     // 1) 如果正文已在缓存
//     auto it = cache_.find(docID);
//     if (it != cache_.end() && !it->second.first.content.empty()) {
//         touchLRU(docID);
//         return it->second.first.content;
//     }

//     // 2) 查询正文
//     std::ostringstream q;
//     q << "SELECT content FROM url_log WHERE id=" << docID << " LIMIT 1;";
//     if (mysql_query(conn_, q.str().c_str()) != 0) {
//         std::cerr << "[DocumentStore] 查询正文失败: " << mysql_error(conn_) << '\n';
//         return "";
//     }
//     MYSQL_RES* res = mysql_store_result(conn_);
//     if (!res) return "";
//     MYSQL_ROW row = mysql_fetch_row(res);
//     std::string body = (row && row[0]) ? row[0] : "";
//     mysql_free_result(res);

//     // 3) 写入缓存
//     if (it == cache_.end()) {
//         // 头数据尚未缓存 → 填充 header
//         DocHeader h = getHeader(docID);    // 内部含锁；若失败也能返回默认
//         CacheEntry ce{h, body};
//         lru_.push_front(docID);
//         cache_[docID] = {std::move(ce), lru_.begin()};
//     } else {
//         it->second.first.content = body;
//         touchLRU(docID);
//     }
//     evictIfNeeded();
//     return body;
// }
std::string DocumentStore::getContent(int docID) 
{
    /********** ① 先尝试命中缓存：持锁、若命中直接返回 **********/
    {
        std::lock_guard<std::mutex> lock(mtx_);
        auto it = cache_.find(docID);
        if (it != cache_.end() && !it->second.first.content.empty()) {
            touchLRU(docID);
            return it->second.first.content;
        }
    }   // —— 作用域结束，mutex 自动释放 ——


    /********** ② 未命中 → 无锁访问 MySQL 读取正文 **********/
    if (!conn_) return "";

    std::ostringstream q;
    q << "SELECT content, url, status, title, timestamp, "
         "CHAR_LENGTH(content) "
      << "FROM url_log WHERE id=" << docID << " LIMIT 1;";

    if (mysql_query(conn_, q.str().c_str()) != 0) {
        std::cerr << "[DocumentStore] 查询正文失败: "
                  << mysql_error(conn_) << '\n';
        return "";
    }
    MYSQL_RES* res = mysql_store_result(conn_);
    if (!res) return "";

    MYSQL_ROW row = mysql_fetch_row(res);
    std::string body    = (row && row[0]) ? row[0] : "";
    std::string url     = (row && row[1]) ? row[1] : "";
    std::string status  = (row && row[2]) ? row[2] : "";
    std::string title   = (row && row[3]) ? row[3] : "";
    std::string ts      = (row && row[4]) ? row[4] : "";
    size_t      len     = (row && row[5]) ? std::stoul(row[5]) : body.size();
    mysql_free_result(res);


    /********** ③ 重新上锁，把正文与头信息写回缓存 **********/
    {
        std::lock_guard<std::mutex> lock(mtx_);

        auto it = cache_.find(docID);
        if (it == cache_.end()) {
            // 头信息还不在缓存 → 新建条目
            DocHeader h{docID, url, title, status, ts, len};
            CacheEntry ce{std::move(h), body};

            lru_.push_front(docID);
            cache_[docID] = {std::move(ce), lru_.begin()};
            evictIfNeeded();
        } else {
            // 头已在缓存，只需补正文
            it->second.first.content        = body;
            it->second.first.header.url     = url;
            it->second.first.header.title   = title;
            it->second.first.header.status  = status;
            it->second.first.header.timestamp = ts;
            it->second.first.header.content_length = len;
            touchLRU(docID);
        }
    }

    return body;
}

size_t DocumentStore::getDocLength(int docID)
{
    return getHeader(docID).content_length;
}

size_t DocumentStore::numDocs()
{
    if (!conn_) return 0;
    if (mysql_query(conn_, "SELECT COUNT(*) FROM url_log;") != 0) {
        std::cerr << "[DocumentStore] COUNT 失败: " << mysql_error(conn_) << '\n';
        return 0;
    }
    MYSQL_RES* res = mysql_store_result(conn_);
    if (!res) return 0;
    MYSQL_ROW row = mysql_fetch_row(res);
    size_t cnt = (row && row[0]) ? std::stoull(row[0]) : 0;
    mysql_free_result(res);
    return cnt;
}

// —————— LRU & util —————————————————————————
void DocumentStore::touchLRU(int docID)
{
    auto it = cache_.find(docID);
    if (it == cache_.end()) return;
    lru_.erase(it->second.second);
    lru_.push_front(docID);
    it->second.second = lru_.begin();
}

void DocumentStore::evictIfNeeded()
{
    while (cache_.size() > cap_) {
        int victim = lru_.back();
        lru_.pop_back();
        cache_.erase(victim);
    }
}

std::string DocumentStore::escape(const std::string& s)
{
    if (!conn_) return "";
    char* buf = new char[s.size() * 2 + 1];
    unsigned long len = mysql_real_escape_string(conn_, buf, s.c_str(), s.size());
    std::string ret(buf, len);
    delete[] buf;
    return ret;
}
