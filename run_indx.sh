export PATH="/usr/local/mysql/bin:$PATH"
export DYLD_LIBRARY_PATH="/usr/local/mysql/lib:/usr/local/mysql-8.0.38-macos14-arm64/lib:$DYLD_LIBRARY_PATH"

# 编译
echo "编译 indexmain.cpp..."

g++ -std=c++17 \
    indexmain.cpp DB_manager.cpp Tokenizer.cpp Indexer.cpp \
    -I/usr/local/mysql/include \
    -L/usr/local/mysql/lib -L/usr/local/mysql-8.0.38-macos14-arm64/lib \
    -Wl,-rpath,/usr/local/mysql/lib \
    -Wl,-rpath,/usr/local/mysql-8.0.38-macos14-arm64/lib \
    -lmysqlclient -pthread \
    -o indexmain

if [ $? -ne 0 ]; then
    echo "编译失败，请检查 MySQL 安装路径及依赖。"
    exit 1
fi

echo "编译成功."