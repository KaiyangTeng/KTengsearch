// #include "Ranker.h"
// #include <algorithm>

// Ranker::Ranker(IndexReader &reader): reader_(reader) {}

// std::vector<std::pair<int, double>>
// Ranker::rank(const std::vector<std::string> &terms,
//              const std::unordered_map<int, std::vector<Docinfo>> &hits) {
//     // 全局统计
//     int N      = reader_.getTotalDocCount();    // 文档总数
//     double avg = reader_.getAvgDocLength();     // 平均文档长度

//     std::vector<std::pair<int, double>> scored;
//     scored.reserve(hits.size());

//     // 对每个命中文档计算 BM25
//     for (auto &kv : hits) {
//         int docID = kv.first;
//         double dl = reader_.getDocLength(docID);
//         double score = 0.0;

//         const auto &vec = kv.second;
//         for (size_t i = 0; i < vec.size(); ++i) {
//             int tf       = vec[i].frequency;      // term frequency
//             const auto &term = terms[i];          // 对应的查询词
//             int df       = reader_.getDocFreq(term);

//             // idf 计算（带 0.5 平滑）
//             double idf = std::log((N - df + 0.5) / (df + 0.5));

//             // BM25 归一化 TF
//             double tf_norm = (tf * (BM25_K1 + 1)) /
//                 (tf + BM25_K1 * (1 - BM25_B + BM25_B * dl / avg));

//             score += idf * tf_norm;
//         }
//         scored.emplace_back(docID, score);
//     }

//     // 按分数降序排序
//     std::sort(scored.begin(), scored.end(),[](auto &a, auto &b) { return a.second > b.second;});
//     return scored;
// }

// Ranker.cpp
#include "Ranker.h"
#include <algorithm>

Ranker::Ranker(IndexReader &reader)
    : reader_(reader)
{
    // 全局统计只做一次
    totalDocs_     = reader_.getTotalDocCount();
    avgDocLength_  = reader_.getAvgDocLength();
}

std::vector<std::pair<int, double>>
Ranker::rank(const std::vector<std::string> &terms,
             const std::unordered_map<int, std::vector<Docinfo>> &hits) {
    // 1) 为每个 term 缓存 DF（如果还没缓存）
    for (const auto &term : terms) {
        if (dfCache_.find(term) == dfCache_.end()) {
            dfCache_[term] = reader_.getDocFreq(term);
        }
    }

    // 2) 为每个命中文档缓存文档长度（如果还没缓存）
    for (const auto &kv : hits) {
        int docID = kv.first;
        if (docLenCache_.find(docID) == docLenCache_.end()) {
            docLenCache_[docID] = reader_.getDocLength(docID);
        }
    }

    // 3) 计算 BM25 并排序
    std::vector<std::pair<int, double>> scored;
    scored.reserve(hits.size());

    for (const auto &kv : hits) {
        int docID = kv.first;
        double dl = docLenCache_[docID];
        double score = 0.0;

        const auto &vec = kv.second;
        // 注意：vec[i] 对应 terms[i]
        for (size_t i = 0; i < vec.size(); ++i) {
            int tf       = vec[i].frequency;
            const auto &term = terms[i];
            int df       = dfCache_[term];

            // idf 计算（带 0.5 平滑）
            double idf = std::log((totalDocs_ - df + 0.5) / (df + 0.5));

            // BM25 归一化 TF
            double tf_norm = (tf * (BM25_K1 + 1)) /
                (tf + BM25_K1 * (1 - BM25_B + BM25_B * dl / avgDocLength_));

            score += idf * tf_norm;
        }

        scored.emplace_back(docID, score);
    }

    std::sort(scored.begin(), scored.end(),
              [](auto &a, auto &b) { return a.second > b.second; });
    return scored;
}
