#include "SnippetGenerator.h"
#include <regex>
#include <algorithm>
#include <sstream>
#include <cctype>

using std::string;
using std::vector;
using std::unordered_set;

SnippetGenerator::SnippetGenerator(DocumentStore& ds,
                                   const Tokenizer&     tk,
                                   size_t               maxLen)
    : ds_(ds), tk_(tk), maxLen_(maxLen)
{}

// ———————————————————————————————————————————————
string SnippetGenerator::generate(int docID,const vector<string>& queryTerms) const
{
    // 1) 取正文并粗略去 HTML
    size_t RAW_LIMIT = 100'000;     // 100 KB 足够做摘要

    std::string raw = ds_.getContent(docID);
    if (raw.size() > RAW_LIMIT) raw.resize(RAW_LIMIT);
    if (raw.empty()) return "";

    string plain = stripHTML(raw);

    // 2) 查询词集合（小写去重）
    unordered_set<string> qset(queryTerms.begin(), queryTerms.end());

    // 3) 找到最早出现的 query term
    string lowerPlain = toLower(plain);
    size_t firstPos = string::npos;
    for (const auto& term : qset) {
        size_t pos = lowerPlain.find(term);
        if (pos != string::npos && pos < firstPos) firstPos = pos;
    }
    if (firstPos == string::npos) firstPos = 0;   // 没找到就从头截

    // 4) 截取窗口（左右各占一半）
    size_t start =
        firstPos > maxLen_ / 2 ? firstPos - maxLen_ / 2 : 0;
    if (start + maxLen_ > plain.size())
        start = plain.size() > maxLen_ ? plain.size() - maxLen_ : 0;

    string snippet = plain.substr(start,
                         std::min(maxLen_, plain.size() - start));

    // 补省略号
    if (start != 0) snippet = "…" + snippet;
    if (start + maxLen_ < plain.size()) snippet += "…";

    // 5) 转义 HTML & 高亮
    snippet = escapeHTML(snippet);
    highlight(snippet, qset);

    return snippet;
}

// ————————————————————— util ——————————————————————
std::string SnippetGenerator::stripHTML(const std::string& html)
{
    std::string out;
    out.reserve(html.size());
    bool inTag = false;
    for (char c : html) {
        if (c == '<') { inTag = true;  continue; }
        if (c == '>') { inTag = false; continue; }
        if (!inTag)   { out.push_back(c); }
    }
    return out;
}

string SnippetGenerator::toLower(const string& s)
{
    string out = s;
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return out;
}

string SnippetGenerator::escapeHTML(const string& s)
{
    string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '&':  out += "&amp;";  break;
            case '<':  out += "&lt;";   break;
            case '>':  out += "&gt;";   break;
            case '"':  out += "&quot;"; break;
            case '\'': out += "&#39;";  break;
            default:   out += c;
        }
    }
    return out;
}

void SnippetGenerator::highlight(
        string& snippet,
        const unordered_set<string>& qset)
{
    string lower = toLower(snippet);
    vector<std::pair<size_t,size_t>> hits;  // [start,end)

    // 找所有命中位置
    for (const auto& term : qset) {
        size_t pos = lower.find(term, 0);
        while (pos != string::npos) {
            hits.emplace_back(pos, pos + term.size());
            pos = lower.find(term, pos + term.size());
        }
    }
    if (hits.empty()) return;

    // 去重 / 合并区间
    std::sort(hits.begin(), hits.end());
    vector<std::pair<size_t,size_t>> merged;
    merged.push_back(hits[0]);
    for (size_t i = 1; i < hits.size(); ++i) {
        auto& last = merged.back();
        if (hits[i].first <= last.second) {
            last.second = std::max(last.second, hits[i].second);
        } else merged.push_back(hits[i]);
    }

    // 从后往前插入 <b></b> 保证偏移不乱
    for (auto it = merged.rbegin(); it != merged.rend(); ++it) {
        snippet.insert(it->second, "</b>");
        snippet.insert(it->first,  "<b>");
    }
}

