// #include "Indexer.h"
// #include <string>
// #include <vector>
// #include <unordered_map>
// #include <mutex>
// using namespace std;




// void Indexer::indexDocument(int currdocID,const string& content)
// {
// 	vector<string> strs=tknizer.getstrs(content);
// 	for(auto it:strs)
// 	{
// 		vector<Docinfo>& temp=statstable[it];
// 		if(temp.empty()||temp.back().docID!=currdocID)
// 		{
// 			temp.push_back({currdocID,1});
// 		}
// 		else
// 		{
// 			temp.back().frequency++;
// 		}

// 	}
// }

#include "Indexer.h"
#include <sstream>
#include <iostream>

Indexer::Indexer(const std::string &host,
                 const std::string &user,
                 const std::string &passwd,
                 const std::string &db_name,
                 unsigned int port)
    : db_(host, user, passwd, db_name, port) {
    // 初始化倒排索引表
    initIndexTable();
}

Indexer::~Indexer() {
    // 析构时可不做额外操作
}

void Indexer::initIndexTable() {
    const std::string sql = R"(
        CREATE TABLE IF NOT EXISTS inverted_index (
            term VARCHAR(255) NOT NULL,
            doc_id INT NOT NULL,
            frequency INT NOT NULL,
            PRIMARY KEY (term, doc_id)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
    )";
    if (!db_.execute(sql)) {
        std::cerr << "初始化 inverted_index 表失败" << std::endl;
    }
}

void Indexer::buildAll() {
    
    auto pages = db_.fetchAllPages();
    for (const auto &p : pages) {
        indexDocument(p.first, p.second);
    }
    saveIndexToDB();
    std::cout << "全量索引完成！共索引 " << pages.size() << " 篇文档。" << std::endl;
}

void Indexer::indexDocument(int docID, const std::string &content) {
    auto tokens = tokenizer_.getstrs(content);
    std::lock_guard<std::mutex> lock(mtx_);
    for (const auto &term : tokens) 
    {
        auto &vec = statstable_[term];
        if (vec.empty() || vec.back().docID != docID) vec.push_back({docID, 1});
        else vec.back().frequency++;
    }
}

void Indexer::saveIndexToDB() {
    for (const auto &kv : statstable_) {
        const std::string &term = kv.first;
        for (const auto &info : kv.second) {
            std::ostringstream oss;
            oss << "INSERT INTO inverted_index(term, doc_id, frequency) VALUES('"
                << term << "', " << info.docID << ", " << info.frequency << ") "
                "ON DUPLICATE KEY UPDATE frequency=VALUES(frequency);";
            if (!db_.execute(oss.str())) {
                std::cerr << "插入索引失败: " << oss.str() << std::endl;
            }
        }
    }
    std::cout << "索引已写入数据库。" << std::endl;
}



