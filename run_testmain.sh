# # 1) 保证 mysql client 在 PATH 里
# export PATH="/usr/local/mysql/bin:$PATH"
# # 2) 保证动态库能被加载
# export DYLD_LIBRARY_PATH="/usr/local/mysql/lib:/usr/local/mysql-8.0.38-macos14-arm64/lib:$DYLD_LIBRARY_PATH"

# echo "编译 Ranker 测试程序..."

# g++ -std=c++17 \
#     rankermain.cpp Retriever.cpp Ranker.cpp IndexReader.cpp QueryParser.cpp Tokenizer.cpp \
#     -I/usr/local/mysql/include \
#     -L/usr/local/mysql/lib -L/usr/local/mysql-8.0.38-macos14-arm64/lib \
#     -Wl,-rpath,/usr/local/mysql/lib \
#     -Wl,-rpath,/usr/local/mysql-8.0.38-macos14-arm64/lib \
#     -lmysqlclient \
#     -o test_ranker

# if [ $? -ne 0 ]; then
#     echo "编译失败，请检查 MySQL 安装路径及依赖。"
#     exit 1
# fi

# echo "编译成功。运行: ./test_ranker"


#!/usr/bin/env bash
# 1) 保证 mysql client 在 PATH 里
export PATH="/usr/local/mysql/bin:$PATH"
# 2) 保证动态库能被加载
export DYLD_LIBRARY_PATH="/usr/local/mysql/lib:/usr/local/mysql-8.0.38-macos14-arm64/lib:$DYLD_LIBRARY_PATH"

echo "编译 Ranker + DocumentStore 测试程序..."

g++ -std=c++17 \
    test_main.cpp \
    Retriever.cpp Ranker.cpp IndexReader.cpp QueryParser.cpp Tokenizer.cpp \
    SnippetGenerator.cpp DocumentStore.cpp DB_manager.cpp \
    -I/usr/local/mysql/include \
    -L/usr/local/mysql/lib -L/usr/local/mysql-8.0.38-macos14-arm64/lib \
    -Wl,-rpath,/usr/local/mysql/lib \
    -Wl,-rpath,/usr/local/mysql-8.0.38-macos14-arm64/lib \
    -lmysqlclient -lpthread \
    -o testmain

if [ $? -ne 0 ]; then
    echo "编译失败"
    exit 1
fi

echo "编译成功。运行: ./testmain"

