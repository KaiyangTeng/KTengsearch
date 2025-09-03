#include "RateLimiter.h"
#include <thread>

RateLimiter::RateLimiter(int max_req, std::chrono::milliseconds time_window) : _max_req(max_req), _window(time_window){}

void RateLimiter::acquire() 
{
    auto now = std::chrono::steady_clock::now();
    std::unique_lock<std::mutex> lk(_mtx);

    // 移除过期记录
    while (!_timestamps.empty() &&
           now - _timestamps.front() >= _window) {
        _timestamps.pop_front();
    }

    // 如果已用完令牌，则等待至第一个时间点过期
    if ((int)_timestamps.size() >= _max_req) {
        auto wait_for = _window - (now - _timestamps.front());
        lk.unlock();
        std::this_thread::sleep_for(wait_for);
        acquire();  // 递归重试
        return;
    }

    // 记录本次调用时间戳
    _timestamps.push_back(now);
}

