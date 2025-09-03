// #include <string>
// #include <vector>
// #include <unordered_map>
// #include <mutex>
// #include "Tokenizer.h"
// #include "DB_manager.h"
// using namespace std;


// struct Docinfo {
//     int docID;               
//     int frequency; 
// };





// class Indexer 
// {
// 	public:
// 	unordered_map<string,vector<Docinfo>> statstable;
// 	Indexer();
// 	Tokenizer tknizer;
// 	void indexDocument(int docID,const string& content);
// };


#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include "Tokenizer.h"
#include "DB_manager.h"

// 每个 Docinfo 存储一次 term 在文档中出现的频次
struct Docinfo {
    int docID;
    int frequency;
};

// Indexer 负责：
// 1) 从数据库读取所有页面
// 2) 调用 Tokenizer 切词，统计各 term 在各 doc 中的频次
// 3) 将统计结果写入 MySQL 的 inverted_index 表
class Indexer {
public:
    
    Indexer(const std::string &host,const std::string &user,const std::string &passwd,const std::string &db_name,unsigned int port = 3306);
    ~Indexer();
    void buildAll();

private:
    DB_manager db_;                              
    Tokenizer tokenizer_;                        
    unordered_map<string,vector<Docinfo>> statstable_;      
    mutex mtx_;

    // 创建 inverted_index 表（若不存在）
    void initIndexTable();
    // 统计单篇文档的 term 频次
    void indexDocument(int docID, const std::string &content);
    // 将内存中的 statstable_ 写入数据库
    void saveIndexToDB();
};







