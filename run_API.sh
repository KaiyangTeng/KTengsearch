# export PATH="/usr/local/mysql/bin:$PATH"
# # 2) 保证动态库能被加载
# export DYLD_LIBRARY_PATH="/usr/local/mysql/lib:/usr/local/mysql-8.0.38-macos14-arm64/lib:$DYLD_LIBRARY_PATH"

# echo "编译 Ranker + DocumentStore 测试程序..."

# g++ -std=c++17 \
#     APItest.cpp \
#     Retriever.cpp Ranker.cpp IndexReader.cpp QueryParser.cpp Tokenizer.cpp \
#     SearchEngine.cpp SnippetGenerator.cpp DocumentStore.cpp DB_manager.cpp \
#     -I/usr/local/mysql/include \
#     -L/usr/local/mysql/lib -L/usr/local/mysql-8.0.38-macos14-arm64/lib \
#     -Wl,-rpath,/usr/local/mysql/lib \
#     -Wl,-rpath,/usr/local/mysql-8.0.38-macos14-arm64/lib \
#     -lmysqlclient -lpthread \
#     -o apitest

# if [ $? -ne 0 ]; then
#     echo "编译失败"
#     exit 1
# fi

# echo "编译成功。运行: ./apitest"

#!/usr/bin/env bash
set -e

g++ -std=c++17 \
    APItest.cpp \
    Retriever.cpp Ranker.cpp IndexReader.cpp QueryParser.cpp Tokenizer.cpp \
    SearchEngine.cpp SnippetGenerator.cpp DocumentStore.cpp DB_manager.cpp \
    -I/usr/local/mysql/include \
    -Ithird_party \
    -L/usr/local/mysql/lib -L/usr/local/mysql-8.0.38-macos14-arm64/lib \
    -Wl,-rpath,/usr/local/mysql/lib \
    -Wl,-rpath,/usr/local/mysql-8.0.38-macos14-arm64/lib \
    -lmysqlclient -lpthread \
    -o out

echo '编译成功。运行示例：'
echo './out "professor"'
