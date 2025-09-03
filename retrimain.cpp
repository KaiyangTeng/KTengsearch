#include <iostream>
#include <vector>
#include <string>
#include "Retriever.h"
#include "QueryParser.h"
using namespace std;
int main() 
{
    
    Retriever retriever("3.138.169.217","crawler","12345678","crawler_db",3306);

   
    std::string query = "Professor Michelle Bezanson";
    QueryParser parser;
    std::vector<std::string> terms = parser.parse(query);

    // 执行检索（AND 模式）
    auto results = retriever.retrieve(terms);
    cout<<"results for <"<<query<<"> : "<<endl;

    for (const auto &kv : results) {
        std::cout << "Document ID: " << kv.first << " -> ";
        for (const auto &info : kv.second) {
            std::cout << "(freq=" << info.frequency << ") ";
        }
        std::cout << "\n";
    }
    return 0;
}