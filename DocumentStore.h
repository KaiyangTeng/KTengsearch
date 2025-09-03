#pragma once
#include <string>
#include <unordered_map>
#include <list>
#include <mutex>
#include <mysql.h>

/**
 * 轻量级文档头信息（展示或排序常用字段）
 */
struct DocHeader {
    int         docID{};
    std::string url;
    std::string title;
    std::string status;
    std::string timestamp;      // MySQL DATETIME -> “YYYY‑MM‑DD HH:MM:SS”
    size_t      content_length; // 字符长度（行数或词数可自行扩展）
};

/**
 * DocumentStore：负责 docID ⇌ 原始文档／元数据 的持久化与随机读
 *  - 内部直接持有一个 MySQL 连接做查询
 *  - 写入仍可复用 DB_manager 里的 insert_page 逻辑
 *  - 读路径自带 LRU 缓存，默认容量 2 k 条
 */
class DocumentStore {
public:
    DocumentStore(const std::string& host,
                  const std::string& user,
                  const std::string& passwd,
                  const std::string& db_name,
                  unsigned int       port       = 3306,
                  size_t             cache_cap  = 2048);
    ~DocumentStore();

    // —— 写入（爬虫/增量索引阶段用） ——
    bool addDocument(const std::string& url,
                     const std::string& status,
                     const std::string& title,
                     const std::string& content);

    // —— 只读接口（检索链路用） ——
    DocHeader   getHeader(int docID);          // URL + Title + 等元数据
    std::string getContent(int docID);         // 整篇正文
    size_t      getDocLength(int docID);       // = content_length
    size_t      numDocs();                     // 表中文档总量

    // 禁拷贝
    DocumentStore(const DocumentStore&)            = delete;
    DocumentStore& operator=(const DocumentStore&) = delete;

private:
    // —— LRU 缓存结构 ——
    struct CacheEntry {
        DocHeader   header;
        std::string content;  // 懒加载：第一次 getContent 时填充
    };
    using ListIter = std::list<int>::iterator;

    void touchLRU(int docID);  // 更新最近使用顺序
    void evictIfNeeded();      // 超容量时驱逐

    // —— MySQL 相关 ——
    MYSQL*  conn_;             // 与 DB_manager 中用法一致
    size_t  cap_;
    std::mutex mtx_;

    // docID → { 缓存项, LRU 链表迭代器 }
    std::unordered_map<int, std::pair<CacheEntry, ListIter>> cache_;
    std::list<int> lru_;       // 最近使用的放前面

    // util
    std::string escape(const std::string& s);
};
