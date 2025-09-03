// #include "DB_manager.h"
// #include <iostream>
// #include <ctime>
// #include <cstring>
// using namespace std;

// DB_manager::DB_manager(const string& db_name) {
//     if (sqlite3_open(db_name.c_str(), &db) != SQLITE_OK) {
//         cerr << "无法打开数据库: " << sqlite3_errmsg(db) << endl;
//         db = nullptr;
//     } else {
//         const char* sql_create = R"(
//             CREATE TABLE IF NOT EXISTS url_log (
//                 id        INTEGER PRIMARY KEY AUTOINCREMENT,
//                 url       TEXT    NOT NULL UNIQUE,
//                 status    TEXT,
//                 title     TEXT,
//                 content   TEXT,
//                 timestamp TEXT
//             );
//         )";
//         execute(sql_create);
//     }
// }

// DB_manager::~DB_manager() {
//     if (db) sqlite3_close(db);
// }

// bool DB_manager::insert_page(const std::string& url, const std::string& status, const std::string& title, const std::string& content) 
// {
//     if (!db) return false;

//     const char* sql =
//       "INSERT OR IGNORE INTO url_log(url,status,title,content,timestamp) "
//       "VALUES(?1,?2,?3,?4,?5);";
//     sqlite3_stmt* stmt = nullptr;

//     std::lock_guard<std::mutex> lock(db_mutex);
//     if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
//         std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << "\n";
//         return false;
//     }

//     // 绑定参数
//     sqlite3_bind_text(stmt, 1, url.c_str(),     -1, SQLITE_TRANSIENT);
//     sqlite3_bind_text(stmt, 2, status.c_str(),  -1, SQLITE_TRANSIENT);
//     sqlite3_bind_text(stmt, 3, title.c_str(),   -1, SQLITE_TRANSIENT);
//     sqlite3_bind_text(stmt, 4, content.c_str(), -1, SQLITE_TRANSIENT);

//     // 时间戳
//     char buf[20];
//     time_t now = time(nullptr);
//     strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
//     sqlite3_bind_text(stmt, 5, buf, -1, SQLITE_TRANSIENT);

//     // 执行并收尾
//     bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
//     sqlite3_finalize(stmt);
//     if (!ok) {
//         std::cerr << "Insert failed: " << sqlite3_errmsg(db) << "\n";
//     }
//     return ok;
// }

// bool DB_manager::execute(const string& sql) {
//     char* errmsg = nullptr;
//     int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errmsg);
//     if (rc != SQLITE_OK) {
//         cerr << "SQL错误: " << errmsg << endl;
//         sqlite3_free(errmsg);
//         return false;
//     }
//     return true;
// }




// DB_manager.cpp
#include "DB_manager.h"
#include <iostream>
#include <sstream>

// 建表语句：InnoDB 引擎，URL 最大长度设为 2083
static const char* CREATE_TABLE_SQL = R"(
CREATE TABLE IF NOT EXISTS url_log (
    id        INT AUTO_INCREMENT PRIMARY KEY,
    url       VARCHAR(2083) NOT NULL,
    status    VARCHAR(255),
    title     TEXT,
    content   TEXT,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    UNIQUE KEY uq_url (url(191))
) ENGINE=InnoDB
  DEFAULT CHARSET = utf8mb4
  COLLATE = utf8mb4_unicode_ci;
)";

DB_manager::DB_manager(const std::string &host,
                       const std::string &user,
                       const std::string &passwd,
                       const std::string &db_name,
                       unsigned int port) {
    conn = mysql_init(nullptr);
    if (!conn) {
        std::cerr << "MySQL init 失败\n";
        return;
    }
    if (!mysql_real_connect(conn,
                            host.c_str(),
                            user.c_str(),
                            passwd.c_str(),
                            db_name.c_str(),
                            port,
                            nullptr,
                            0)) {
        std::cerr << "MySQL 连接失败: " << mysql_error(conn) << "\n";
        mysql_close(conn);
        conn = nullptr;
    } else {
        // 保证使用 UTF8
        execute("SET NAMES utf8mb4;");
        // 创建表
        execute(CREATE_TABLE_SQL);
    }
}

DB_manager::~DB_manager() {
    if (conn) mysql_close(conn);
}

bool DB_manager::execute(const std::string &sql) {
    if (!conn) return false;
    if (mysql_query(conn, sql.c_str())) {
        std::cerr << "MySQL 执行错误: " << mysql_error(conn) << "\n";
        return false;
    }
    return true;
}

std::string DB_manager::escapeString(const std::string &input) {
    if (!conn) return "";
    // 分配两倍长度再加一，MySQL 文档推荐
    char* buf = new char[input.size() * 2 + 1];
    unsigned long len = mysql_real_escape_string(
        conn, buf, input.c_str(), input.size());
    std::string ret(buf, len);
    delete[] buf;
    return ret;
}

bool DB_manager::insert_page(const std::string &url,
                             const std::string &status,
                             const std::string &title,
                             const std::string &content) {
    if (!conn) return false;
    // 构造 INSERT IGNORE 语句
    std::ostringstream oss;
    oss << "INSERT IGNORE INTO url_log "
        "(url, status, title, content, timestamp) VALUES ("
        "'" << escapeString(url) << "',"
        "'" << escapeString(status) << "',"
        "'" << escapeString(title) << "',"
        "'" << escapeString(content) << "',"
        "NOW()"
        ");";
    return execute(oss.str());
}


std::vector<std::pair<int,std::string>> DB_manager::fetchAllPages() {
    std::vector<std::pair<int,std::string>> result;
    if (!conn) return result;
    // 查询 id 与 content
    if (mysql_query(conn, "SELECT id, content FROM url_log;")) {
        std::cerr << "MySQL 查询失败: " << mysql_error(conn) << std::endl;
        return result;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) return result;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        int id = row[0] ? std::stoi(row[0]) : 0;
        std::string content = row[1] ? row[1] : "";
        result.emplace_back(id, content);
    }
    mysql_free_result(res);
    return result;
}


