# g++ -std=c++17 \
#     worker_v3.cpp \
#     LinkFetcher.cpp Content_getterv2.cpp HTML_getter.cpp URL_getter.cpp URL_filter.cpp URL_checker.cpp \
#     RateLimiter.cpp DB_manager.cpp \
#     -I./hiredis -I./gumbo-parser/src \
#     -L./hiredis ./hiredis/libhiredis.a \
#     ./gumbo-parser/.libs/libgumbo.a \
#     -lcurl -lsqlite3 -lpthread \
#     -o worker
export PATH="/usr/local/mysql/bin:$PATH"

export DYLD_LIBRARY_PATH="/usr/local/mysql/lib:/usr/local/mysql-8.0.38-macos14-arm64/lib:$DYLD_LIBRARY_PATH"

MYSQL_CFLAGS=$(mysql_config --cflags)
MYSQL_LIBS=$(mysql_config --libs)

# g++ -std=c++17 \
#     worker_v3.cpp \
#     LinkFetcher.cpp Content_getterv2.cpp HTML_getter.cpp URL_getter.cpp URL_filter.cpp URL_checker.cpp \
#     RateLimiter.cpp DB_manager.cpp \
#     -I./hiredis -I./gumbo-parser/src \
#     -L./hiredis ./hiredis/libhiredis.a \
#     ./gumbo-parser/.libs/libgumbo.a \
#     ${MYSQL_CFLAGS} ${MYSQL_LIBS} \
#     -lcurl -lpthread \
#     -o worker
g++ -std=c++17 \
    worker_v3.cpp \
    LinkFetcher.cpp Content_getterv2.cpp HTML_getter.cpp URL_getter.cpp URL_filter.cpp URL_checker.cpp \
    RateLimiter.cpp DB_manager.cpp \
    -I./hiredis -I./gumbo-parser/src \
    -I/usr/local/mysql/include \
    -L./hiredis ./hiredis/libhiredis.a \
    ./gumbo-parser/.libs/libgumbo.a \
    -L/usr/local/mysql/lib -L/usr/local/mysql-8.0.38-macos14-arm64/lib \
    -Wl,-rpath,/usr/local/mysql/lib \
    -Wl,-rpath,/usr/local/mysql-8.0.38-macos14-arm64/lib \
    -lmysqlclient -lcurl -lpthread \
    -o workernew



