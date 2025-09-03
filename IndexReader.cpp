#include "IndexReader.h"
#include <iostream>
#include <sstream>
#include <cstdlib>

IndexReader::IndexReader(const std::string &host,
                         const std::string &user,
                         const std::string &passwd,
                         const std::string &db_name,
                         unsigned int port) {
    conn_ = mysql_init(nullptr);
    if (!conn_) {
        std::cerr << "[IndexReader] mysql_init 失败\n";
        return;
    }
    if (!mysql_real_connect(conn_,
                            host.c_str(),
                            user.c_str(),
                            passwd.c_str(),
                            db_name.c_str(),
                            port,
                            nullptr,
                            0)) {
        std::cerr << "[IndexReader] 连接失败: "
                  << mysql_error(conn_) << "\n";
        mysql_close(conn_);
        conn_ = nullptr;
        return;
    }
    // 保证使用 UTF8 编码
    if (mysql_query(conn_, "SET NAMES utf8mb4;")) {
        std::cerr << "[IndexReader] SET NAMES 失败: "
                  << mysql_error(conn_) << "\n";
    }
}

IndexReader::~IndexReader() {
    if (conn_) {
        mysql_close(conn_);
    }
}

std::string IndexReader::escapeString(const std::string &input) {
    if (!conn_) return "";
    // 按 MySQL 文档推荐，上溢分配
    char *buf = new char[input.size() * 2 + 1];
    unsigned long len = mysql_real_escape_string(
        conn_, buf, input.c_str(), input.size());
    std::string ret(buf, len);
    delete[] buf;
    return ret;
}

std::vector<Docinfo> IndexReader::getPostingList(const std::string &term) {
    std::vector<Docinfo> result;
    if (!conn_) return result;

    // 构造安全的查询
    std::string esc = escapeString(term);
    std::ostringstream oss;
    oss << "SELECT doc_id, frequency "
        << "FROM inverted_index "
        << "WHERE term = '" << esc << "' "
        << "ORDER BY doc_id;";
    std::string sql = oss.str();

    if (mysql_query(conn_, sql.c_str())) {
        std::cerr << "[IndexReader] 查询失败: "
                  << mysql_error(conn_) << "\n";
        return result;
    }
    MYSQL_RES *res = mysql_store_result(conn_);
    if (!res) return result;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        int docID    = row[0] ? std::atoi(row[0])       : 0;
        int frequency= row[1] ? std::atoi(row[1])       : 0;
        result.push_back({docID, frequency});
    }
    mysql_free_result(res);
    return result;
}

int IndexReader::getTotalDocCount() {
    if (!conn_) return 0;
    const char *sql = 
      "SELECT COUNT(DISTINCT doc_id) "
      "FROM inverted_index;";
    if (mysql_query(conn_, sql)) {
        std::cerr << "[IndexReader] getTotalDocCount 查询失败: "
                  << mysql_error(conn_) << "\n";
        return 0;
    }
    MYSQL_RES *res = mysql_store_result(conn_);
    if (!res) return 0;
    MYSQL_ROW row = mysql_fetch_row(res);
    int count = row && row[0] ? std::atoi(row[0]) : 0;
    mysql_free_result(res);
    return count;
}

int IndexReader::getDocFreq(const std::string &term) {
    if (!conn_) return 0;
    std::string esc = escapeString(term);
    std::ostringstream oss;
    oss << "SELECT COUNT(*) "
        << "FROM inverted_index "
        << "WHERE term = '" << esc << "';";
    if (mysql_query(conn_, oss.str().c_str())) {
        std::cerr << "[IndexReader] getDocFreq 查询失败: "
                  << mysql_error(conn_) << "\n";
        return 0;
    }
    MYSQL_RES *res = mysql_store_result(conn_);
    if (!res) return 0;
    MYSQL_ROW row = mysql_fetch_row(res);
    int df = row && row[0] ? std::atoi(row[0]) : 0;
    mysql_free_result(res);
    return df;
}

int IndexReader::getDocLength(int docID) {
    if (!conn_) return 0;
    std::ostringstream oss;
    oss << "SELECT SUM(frequency) "
        << "FROM inverted_index "
        << "WHERE doc_id = " << docID << ";";
    if (mysql_query(conn_, oss.str().c_str())) {
        std::cerr << "[IndexReader] getDocLength 查询失败: "
                  << mysql_error(conn_) << "\n";
        return 0;
    }
    MYSQL_RES *res = mysql_store_result(conn_);
    if (!res) return 0;
    MYSQL_ROW row = mysql_fetch_row(res);
    int length = row && row[0] ? std::atoi(row[0]) : 0;
    mysql_free_result(res);
    return length;
}

double IndexReader::getAvgDocLength() {
    if (!conn_) return 0.0;
    const char *sql =
      "SELECT SUM(frequency) AS total_terms, "
      "       COUNT(DISTINCT doc_id) AS doc_count "
      "FROM inverted_index;";
    if (mysql_query(conn_, sql)) {
        std::cerr << "[IndexReader] getAvgDocLength 查询失败: "
                  << mysql_error(conn_) << "\n";
        return 0.0;
    }
    MYSQL_RES *res = mysql_store_result(conn_);
    if (!res) return 0.0;
    MYSQL_ROW row = mysql_fetch_row(res);
    long long total_terms = row && row[0] ? std::atoll(row[0]) : 0;
    int doc_count        = row && row[1] ? std::atoi(row[1])  : 0;
    mysql_free_result(res);

    return doc_count > 0
         ? static_cast<double>(total_terms) / doc_count
         : 0.0;
}
