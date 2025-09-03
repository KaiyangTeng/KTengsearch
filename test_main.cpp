#include <iostream>
#include <vector>
#include <string>
#include "IndexReader.h"
#include "Retriever.h"
#include "QueryParser.h"
#include "Ranker.h"
#include "DocumentStore.h"
#include "SnippetGenerator.h"
using namespace std;

int main() 
{
   
    
    // 初始化 IndexReader, Retriever 和 Ranker

    IndexReader reader("3.138.141.182","crawler","12345678","crawler_db",3306);
    Retriever retriever("3.138.141.182","crawler","12345678","crawler_db",3306);
    Ranker ranker(reader);
    Tokenizer  tk;
    DocumentStore docStore("3.138.141.182","crawler","12345678","crawler_db",3306);
    SnippetGenerator snip(docStore, tk);

    // 测试查询
    std::string query = "Julia von";
    QueryParser parser;
    std::vector<std::string> terms = parser.parse(query);

    // 1) 执行检索（AND 模式）
    auto results = retriever.retrieve(terms);
    cout<<"results for <"<<query<<"> : "<<endl;

    for (const auto &kv : results) {
        std::cout << "Document ID: " << kv.first << " -> ";
        for (const auto &info : kv.second) {
            std::cout << "(freq=" << info.frequency << ") ";
        }
        std::cout << "\n";
    }

    // 2) 对检索结果进行排序
    auto scored = ranker.rank(terms, results);
    // std::cout << "\nRanked results for query: \"" << query << "\"\n";
    // for (const auto &p : scored) {
    //     std::cout << "DocID: " << p.first << ", Score: " << p.second << "\n";
    // }
    int rank = 0;
    for (const auto &kv : scored) 
    {
        const int    docID = kv.first;

        
        //const double score = kv.second;
        // auto header = docStore.getHeader(docID);     // URL / title / len…

        // std::cout << ++rank << ". "
        //           << "DocID "  << docID
        //           << "  (score=" << score << ")\n"
        //           << "    URL   : " << header.url   << "\n"
        //           << "    Title : " << header.title << "\n"
        //           << "    Bytes : " << header.content_length << "\n\n";
        std::string summary = snip.generate(docID, terms);
        std::cout << summary << std::endl;

        if (rank == 10) break;
    }




    return 0;
}













