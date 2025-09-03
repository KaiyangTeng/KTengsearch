#include <deque>
#include <mutex>
#include <chrono>

class RateLimiter {
public:
    // 在给定的 time_window 内，最多允许 max_req 次 acquire()
    RateLimiter(int max_req, std::chrono::milliseconds time_window);
    void acquire();

private:
    int _max_req;
    std::chrono::milliseconds _window;
    std::deque<std::chrono::steady_clock::time_point> _timestamps;
    std::mutex _mtx;
};

