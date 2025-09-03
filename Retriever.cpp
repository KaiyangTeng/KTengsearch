// Retriever.cpp
#include "Retriever.h"
using namespace std;
Retriever::Retriever(const std::string& host,
                     const std::string& usrname,
                     const std::string& paswrd,
                     const std::string& dbname,
                     unsigned int port)
  : reader_(host, usrname, paswrd, dbname, port)
{ }

std::unordered_map<int, std::vector<Docinfo>>
Retriever::retrieve(const std::vector<std::string> &terms) 
{
    std::unordered_map<int, std::vector<Docinfo>> results,copy;

    // 1. 遍历每个 term，获取 posting list 并聚合到 results
    for (const auto &term : terms) {
        auto plist = reader_.getPostingList(term);
        for (const auto &info : plist) {
            results[info.docID].push_back(info);
        }
    }

    copy=results;
    // 2. 如果是 AND 模式，剔除那些 term 出现次数 < terms.size() 的文档
    
    for (auto it = results.begin(); it != results.end(); ) 
    {
        if (it->second.size() < terms.size()) it = results.erase(it);
        else ++it;
    }
    
    if(results.size() == 0) return copy;
    return results;
}
