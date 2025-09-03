// #include <string>
// #include <mutex>
// #include <sqlite3.h>

// class DB_manager {
// public:
//     DB_manager(const std::string& db_name = "crawler.db");
//     ~DB_manager();

//     bool insert_page(const std::string& url, const std::string& status, const std::string& title, const std::string& content);
//     sqlite3* get_raw_handle() { return db; } 
//     bool execute(const std::string& sql);
// private:
//     sqlite3* db;
//     std::mutex db_mutex;
    
// };






// DB_manager.h

#include <string>
#include <vector>
#include <utility>
#include <mysql.h>

class DB_manager {
public:
    // host: MySQL 主机；user: 用户名；passwd: 密码；db_name: 数据库名；port: 端口
    DB_manager(const std::string &host,
               const std::string &user,
               const std::string &passwd,
               const std::string &db_name,
               unsigned int port = 3306);
    ~DB_manager();

    // 插入一条页面记录（URL 唯一，忽略重复）
    bool insert_page(const std::string &url,
                     const std::string &status,
                     const std::string &title,
                     const std::string &content);

    // 执行任意 SQL（建表、SET NAMES 等）
    bool execute(const std::string &sql);
    std::vector<std::pair<int,std::string>> fetchAllPages();
private:
    MYSQL* conn;
    // 对所有输入串进行转义，防止 SQL 注入
    std::string escapeString(const std::string &input);
};









