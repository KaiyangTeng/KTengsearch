// // worker_limit.cpp — 轻量级 Redis 分布式爬虫 Worker
// // 停止条件：当首次抓取的 URL 总数 >= MAX_URLS 时，不再向队列添加新任务
// #include <iostream>
// #include <string>
// #include <set>
// #include <chrono>
// #include <thread>
// #include "hiredis.h"

// #include "LinkFetcher.h"
// #include "DB_manager.h"
// #include "RateLimiter.h"

// /* ---------- 全局常量 ---------- */
// static RateLimiter limiter(20, std::chrono::seconds(1));   // 每秒 20 次请求
// static DB_manager  db("crawler.db");

// constexpr long long MAX_URLS = 100;                      // ☆ 全局抓取上限

// /* ---------- 把新链接丢回 Redis 队列 ---------- */
// void push_tasks(redisContext* redis, const std::set<std::string>& links) {
//     for (const auto& link : links) {
//         std::string task = link + "|1";    // depth 字段留作占位
//         redisReply* r = (redisReply*)redisCommand(
//             redis, "LPUSH task_queue %s", task.c_str());
//         if (r) freeReplyObject(r);
//     }
// }

// int main() {
//     /* 1. 连接 Redis */
//     redisContext* redis = redisConnect("127.0.0.1", 6379);
//     if (!redis || redis->err) {
//         std::cerr << "Redis 连接失败: "
//                   << (redis ? redis->errstr : "null") << '\n';
//         return 1;
//     }

//     /* 2. 初始化计数器（若不存在） */
//     redisCommand(redis, "SETNX crawled_counter 0");

//     std::cout << "[Worker] 启动成功，目标总量 = "
//               << MAX_URLS << " 个 URL\n";

//     /* 3. 主循环：领取并处理任务 */
//     while (true) {
//         /* -- 3‑1 阻塞式领取任务 -- */
//         redisReply* rep = (redisReply*)redisCommand(
//             redis, "BRPOP task_queue 0");
//         if (!rep || rep->type != REDIS_REPLY_ARRAY || rep->elements != 2) {
//             if (rep) freeReplyObject(rep);
//             continue;                       // 理论上很少发生
//         }
//         std::string task(rep->element[1]->str, rep->element[1]->len);
//         freeReplyObject(rep);

//         /* -- 3‑2 解析 URL -- */
//         size_t bar = task.find('|');
//         std::string url = (bar == std::string::npos) ? task
//                                                      : task.substr(0, bar);

//         /* -- 3‑3 去重：只处理第一次出现的 URL -- */
//         redisReply* dup = (redisReply*)redisCommand(
//             redis, "SADD visited_set %s", url.c_str());
//         bool first_time = dup && dup->type == REDIS_REPLY_INTEGER &&
//                           dup->integer == 1;
//         if (dup) freeReplyObject(dup);
//         if (!first_time) continue;




//         /* -- 3‑4 全局计数器 +1，并读取当前总量 -- */
//         redisReply* cnt = (redisReply*)redisCommand(
//             redis, "INCR crawled_counter");
//         long long total = cnt && cnt->type == REDIS_REPLY_INTEGER
//                           ? cnt->integer : 0;
//         if (cnt) freeReplyObject(cnt);

//         std::cout << "[抓取] " << url
//                   << "   total=" << total << '\n';





//         /* -- 3‑5 限速 + 抓取网页 -- */
//         limiter.acquire();

//         LinkFetcher fetcher;
//         std::set<std::string> links = fetcher.get_all_urls(url);




//         /* -- 3‑6 结果写数据库 -- */
//         if (links.empty())
//             db.insert_url(url, "Failed");
//         else
//             db.insert_url(url, "Success");

//         /* -- 3‑7 只有“总量 < 上限”时，才扩散子任务 -- */
//         if (total < MAX_URLS && !links.empty()) {
//             push_tasks(redis, links);
//         }
//         /* 若 total ≥ MAX_URLS，不再 LPUSH；队列处理完就会变空 */
//     }

//     redisFree(redis);
//     return 0;
// }


// worker_limit_success.cpp — 至少收集 MAX_URLS 条 Success
#include <iostream>
#include <string>
#include <set>
#include <chrono>
#include <thread>
#include "hiredis.h"

#include "LinkFetcher.h"
#include "DB_manager.h"
#include "RateLimiter.h"

/* ---------- 配置 ---------- */
static RateLimiter limiter(1, std::chrono::seconds(10));
static DB_manager  db("crawler.db");
constexpr long long MAX_URLS = 100;        // 至少写入这么多 Success

/* ---------- LPUSH 新任务 ---------- */
void push_tasks(redisContext* redis, const std::set<std::string>& links) {
    for (const auto& link : links) {
        std::string task = link + "|1";
        redisReply* r = (redisReply*)redisCommand(
            redis, "LPUSH task_queue %s", task.c_str());
        if (r) freeReplyObject(r);
    }
}

int main() {
    redisContext* redis = redisConnect("127.0.0.1", 6379);
    if (!redis || redis->err) {
        std::cerr << "Redis 连接失败: "
                  << (redis ? redis->errstr : "null") << '\n';
        return 1;
    }
    /* 成功计数器初始化 */
    redisCommand(redis, "SETNX success_counter 0");
    std::cout << "[Worker] 启动，目标 Success = "
              << MAX_URLS << '\n';

    while (true) {
        /* --- 领取任务 --- */
        redisReply* rep = (redisReply*)redisCommand(
            redis, "BRPOP task_queue 0");
        if (!rep || rep->type != REDIS_REPLY_ARRAY || rep->elements != 2) {
            if (rep) freeReplyObject(rep);
            continue;
        }
        std::string task(rep->element[1]->str, rep->element[1]->len);
        freeReplyObject(rep);

        size_t bar = task.find('|');
        std::string url = (bar == std::string::npos) ?
                          task : task.substr(0, bar);

        /* --- 去重 --- */
        redisReply* dup = (redisReply*)redisCommand(
            redis, "SADD visited_set %s", url.c_str());
        bool first = dup && dup->type == REDIS_REPLY_INTEGER && dup->integer == 1;
        if (dup) freeReplyObject(dup);
        if (!first) continue;

        /* --- 抓取 --- */
        limiter.acquire();
        LinkFetcher fetcher;
        auto links = fetcher.get_all_urls(url);

        bool ok = !links.empty();
    
        /* --- Success 计数 --- */
        long long success_total = 0;
        if (ok) {
            db.insert_url(url, "Success");
            redisReply* cnt = (redisReply*)redisCommand(
                redis, "INCR success_counter");
            success_total = cnt && cnt->type == REDIS_REPLY_INTEGER
                            ? cnt->integer : 0;
            if (cnt) freeReplyObject(cnt);
        } else {
            redisReply* cnt = (redisReply*)redisCommand(
                redis, "GET success_counter");
            if (cnt && cnt->type == REDIS_REPLY_STRING)
                success_total = std::stoll(cnt->str);
            if (cnt) freeReplyObject(cnt);
        }

        std::cout << "[抓取] " << url << "   succ=" << success_total << '\n';

        if (success_total >= MAX_URLS) {
            std::cout << "[Worker] 已达到最大 URL 数量 "
                      << MAX_URLS << "，退出。\n";
            break;
        }
                  

        /* --- 扩散条件：Success 总数未达标 且 当前抓取成功 --- */
        if (success_total < MAX_URLS && ok)
            push_tasks(redis, links);
    }

    redisFree(redis);
    return 0;
}


