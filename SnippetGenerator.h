#pragma once
#include <string>
#include <vector>
#include <unordered_set>

#include "DocumentStore.h"
#include "Tokenizer.h"

/**
 * 生成结果摘要并高亮查询词
 * 用法：
 *   SnippetGenerator snip(docStore, tokenizer);
 *   auto s = snip.generate(docID, queryTerms);
 */
class SnippetGenerator {
public:
    SnippetGenerator(DocumentStore& ds,
                     const Tokenizer&     tk,
                     size_t               maxLen = 160);

    /**
     * @param docID      文档 ID
     * @param queryTerms 小写化后的查询 token 列表
     * @return           已高亮的摘要（UTF‑8；<b> 包裹）
     */
    std::string generate(int docID,
                         const std::vector<std::string>& queryTerms) const;

private:
    DocumentStore& ds_;
    const Tokenizer&     tk_;
    size_t               maxLen_;

    // —— 工具函数 ——
    static std::string stripHTML(const std::string& html);
    static std::string toLower(const std::string& s);
    static std::string escapeHTML(const std::string& s);
    static void        highlight(std::string& snippet,
                                 const std::unordered_set<std::string>& qset);
};

