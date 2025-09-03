
/********************************************************************
 *  worker_limit_success_ac.cpp                                     *
 *  - 计数口径：只有抓取成功(links != empty)才写库 + 计数器 + 扩散   *
 *  - A：扩散用 Lua 脚本，原子检查 success_counter < MAX_URLS        *
 *  - 已将 SQLite 存储迁移为 MySQL                                 *
 *******************************************************************/
#include <iostream>
#include <string>
#include <set>
#include <chrono>
#include <thread>
#include "hiredis.h"
#include "LinkFetcher.h"
#include "DB_manager.h"     // MySQL 版本
#include "RateLimiter.h"

constexpr long long MAX_URLS = 1000;                        // 成功 URL 目标
static RateLimiter limiter(1, std::chrono::seconds(1));
// 将下面的参数替换为你的 MySQL 地址、用户名、密码、数据库名和端口
static DB_manager db( "18.219.206.82", "crawler","12345678","crawler_db",3306);

/* ---------- Lua 脚本：原子 LPUSH ---------- */
const char* LUA_PUSH = R"(
local cnt = tonumber(redis.call('GET', KEYS[1]) or '0')
if cnt < tonumber(ARGV[1]) then
  redis.call('LPUSH', KEYS[2], ARGV[2])
  return 1            -- push 成功
end
return 0              -- 已达上限，忽略
)";

/* 封装一层 C++ 调用 try_push */
bool try_push(redisContext* r, const std::string& task) {
    redisReply* rep = (redisReply*)redisCommand(
        r,
        "EVAL %s 2 success_counter task_queue %lld %s",
        LUA_PUSH,
        (long long)MAX_URLS,
        task.c_str()
    );
    bool ok = rep && rep->type == REDIS_REPLY_INTEGER && rep->integer == 1;
    if (rep) freeReplyObject(rep);
    return ok;
}

int main() {
    /* 1. 连接 Redis */
    redisContext* redis = redisConnect("18.219.206.82", 6379);
    if (!redis || redis->err) {
        std::cerr << "Redis 连接失败: "
                  << (redis ? redis->errstr : "Unknown error")
                  << std::endl;
        return 1;
    }
    // 初始化计数器
    redisCommand(redis, "SETNX success_counter 0");
    std::cout << "[Worker] 启动，目标 Success ≥ "
              << MAX_URLS << std::endl;

    /* 2. 主循环 */
    while (true) {
        // 2‑1. 阻塞获取任务
        redisReply* rep = (redisReply*)redisCommand( redis, "BRPOP task_queue 0");
        if (!rep || rep->type != REDIS_REPLY_ARRAY || rep->elements != 2) {
            if (rep) freeReplyObject(rep);
            continue;
        }
        std::string task(rep->element[1]->str, rep->element[1]->len);
        freeReplyObject(rep);

        // 2‑2. 解析 URL
        size_t bar = task.find('|');
        std::string url = (bar == std::string::npos)
                          ? task
                          : task.substr(0, bar);

        // 2‑3. 去重：Redis SET 存储已访问 URL
        redisReply* dup = (redisReply*)redisCommand(
            redis, "SADD visited_set %s", url.c_str()
        );
        bool first_time = dup && dup->type == REDIS_REPLY_INTEGER
                          && dup->integer == 1;
        if (dup) freeReplyObject(dup);
        if (!first_time) continue;

        // 2‑4. 执行抓取（带限流）
        limiter.acquire();
        LinkFetcher fetcher;
        PageData page = fetcher.get_all_info(url);
        bool ok = page.ok;

        // 2‑5. 更新计数器 & 写库
        long long success_total = 0;
        if (ok) {
            // 插入到 MySQL
            db.insert_page(url, "Success", page.title, page.content);

            // Redis 计数器自增
            redisReply* cnt = (redisReply*)redisCommand(
                redis, "INCR success_counter"
            );
            if (cnt && cnt->type == REDIS_REPLY_INTEGER)
                success_total = cnt->integer;
            if (cnt) freeReplyObject(cnt);
        } else {
            // 失败时仅读取计数器，不写库
            redisReply* cnt = (redisReply*)redisCommand(
                redis, "GET success_counter"
            );
            if (cnt && cnt->type == REDIS_REPLY_STRING)
                success_total = std::stoll(cnt->str);
            if (cnt) freeReplyObject(cnt);
        }

        std::cout << "[抓取] " << url
                  << "   succ=" << success_total
                  << std::endl;

        // 目标达成则退出
        if (success_total >= MAX_URLS) {
            std::cout << "[Worker] 已达到最大 URL 数量 "
                      << MAX_URLS << "，退出。" << std::endl;
            break;
        }

        // 2‑6. 扩散：将新链接原子推入队列
        if (ok && success_total < MAX_URLS) {
            for (const auto& link : page.links) {
                try_push(redis, link + "|1");
            }
        }
    }

    // 清理并退出
    redisFree(redis);
    return 0;
}


