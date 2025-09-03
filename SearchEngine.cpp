// SearchService.cpp
#include "SearchEngine.h"
#include <algorithm>

SearchEngine::SearchEngine(const std::string& host,const std::string& user,const std::string& pwd,const std::string& db,unsigned port,size_t topK)
    : tokenizer_(),
      parser_(),
      docStore_(host, user, pwd, db, port),
      indexReader_(host, user, pwd, db, port),
      retriever_(host, user, pwd, db, port),
      ranker_(indexReader_),
      snipper_(docStore_, tokenizer_),
      topK_(topK)
{}

std::vector<SearchHit> SearchEngine::search(const std::string& query)
{
    // 1) 解析查询
    auto terms = parser_.parse(query);

    // 2) 候选检索
    auto posting = retriever_.retrieve(terms);
    if (posting.empty()) return {};

    // 3) 排名
    auto scored = ranker_.rank(terms, posting);
    if (scored.empty()) return {};

    // 4) 裁剪 top‑K
    size_t k = std::min(topK_, scored.size());
    std::vector<SearchHit> hits;
    hits.reserve(k);

    // 5) 生成摘要 & 读取元数据
    for (size_t i = 0; i < k; ++i) {
        int    docID = scored[i].first;
        double score = scored[i].second;

        auto   header  = docStore_.getHeader(docID);
        auto   snippet = snipper_.generate(docID, terms);

        hits.push_back({docID, score,header.url, header.title, std::move(snippet)});
    }
    return hits;
}
