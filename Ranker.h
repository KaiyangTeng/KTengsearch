// Ranker.h
// #pragma once

// #include <vector>
// #include <string>
// #include <unordered_map>
// #include <utility>
// #include <cmath>
// #include "IndexReader.h"

// // BM25 参数
// static constexpr double BM25_K1 = 1.5;
// static constexpr double BM25_B  = 0.75;

// class Ranker {
// public:
//     // 构造时注入 IndexReader，用于拿 DF、N、docLen、avgLen 等全局统计
//     explicit Ranker(IndexReader &reader);

//     /**
//      * 对命中文档进行打分并排序（BM25 算法示例）
//      * @param terms: 查询词列表（顺序要与 hits 中每个 vector<Docinfo> 插入顺序一致）
//      * @param hits: Retriever 输出的 map<docID, vector<Docinfo>>，
//      *              每个 vector<Docinfo>[i].frequency 对应 terms[i] 的 TF
//      * @return 按 BM25 得分降序排列的<docID, score>列表
//      */
//     std::vector<std::pair<int, double>>
//     rank(const std::vector<std::string> &terms,const std::unordered_map<int, std::vector<Docinfo>> &hits);

// private:
//     IndexReader &reader_;
// };

// Ranker.h
#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <utility>
#include <cmath>
#include "IndexReader.h"

// BM25 参数
static constexpr double BM25_K1 = 1.5;
static constexpr double BM25_B  = 0.75;

class Ranker {
public:
    // 构造时注入 IndexReader，用于拿 DF、N、docLen、avgLen 等全局统计
    explicit Ranker(IndexReader &reader);

    /**
     * 对命中文档进行打分并排序（BM25 算法，带缓存优化）
     * @param terms: 查询词列表
     * @param hits:  Retriever 输出的 map<docID, vector<Docinfo>>
     * @return 按 BM25 得分降序排列的 <docID, score> 列表
     */
    std::vector<std::pair<int, double>>
    rank(const std::vector<std::string> &terms,
         const std::unordered_map<int, std::vector<Docinfo>> &hits);

private:
    IndexReader &reader_;

    int     totalDocs_;               // N
    double  avgDocLength_;            // L_avg
    std::unordered_map<std::string,int> dfCache_;    // term -> DF
    std::unordered_map<int,int>         docLenCache_; // docID -> |d|
};
