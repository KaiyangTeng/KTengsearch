#pragma once
#include <string>
#include <vector>
#include <mysql.h>

// 与 Indexer 中 Docinfo 保持一致
struct Docinfo {
    int docID;
    int frequency;
};

class IndexReader {
public:
    // 构造时传入与 DB_manager 相同的连接参数
    IndexReader(const std::string &host,
                const std::string &user,
                const std::string &passwd,
                const std::string &db_name,
                unsigned int port = 3306);
    ~IndexReader();

    // 返回指定 term 的 posting list（按 doc_id 升序）
    std::vector<Docinfo> getPostingList(const std::string &term);

    // 返回倒排索引中不同文档的总数 N
    int getTotalDocCount();

    // 返回某个 term 出现过的文档数，即文档频次 DF(term)
    int getDocFreq(const std::string &term);

    // 返回指定 docID 的文档长度（即该 docID 所有 term 频次之和）
    int getDocLength(int docID);

    // 返回所有文档的平均长度 L_avg
    double getAvgDocLength();

private:
    // 对用户输入的 term 做 MySQL 转义
    std::string escapeString(const std::string &input);

    MYSQL *conn_;  // MySQL 连接句柄
};
