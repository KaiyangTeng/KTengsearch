set -e                         # 任一子命令出错立即退出

g++ -std=c++11 \
    worker_v2.cpp \
    LinkFetcher.cpp HTML_getter.cpp URL_getter.cpp URL_filter.cpp URL_checker.cpp \
    RateLimiter.cpp DB_manager.cpp \
    -I./hiredis \
    -I./gumbo-parser/src \
    -L./hiredis \
    ./hiredis/libhiredis.a \
    ./gumbo-parser/.libs/libgumbo.a \
    -lcurl -lsqlite3 -lpthread \
    -o worker