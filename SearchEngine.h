// SearchService.h
#pragma once
#include <string>
#include <vector>

#include "Tokenizer.h"
#include "QueryParser.h"
#include "Retriever.h"
#include "Ranker.h"
#include "SnippetGenerator.h"
#include "DocumentStore.h"

struct SearchHit {
    int         docID;
    double      score;
    std::string url;
    std::string title;
    std::string snippet;
};

class SearchEngine {
public:
    SearchEngine(const std::string& host,
                  const std::string& user,
                  const std::string& pwd,
                  const std::string& db,
                  unsigned           port = 3306,
                  size_t             topK = 10);

    std::vector<SearchHit> search(const std::string& query);

private:
    // 按依赖关系持有模块
    Tokenizer            tokenizer_;
    QueryParser          parser_;
    DocumentStore        docStore_;
    IndexReader          indexReader_;
    Retriever            retriever_;
    Ranker               ranker_;
    SnippetGenerator     snipper_;

    size_t               topK_;
};

