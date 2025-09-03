

# 1) 保证 mysql client 在 PATH 里
export PATH="/usr/local/mysql/bin:$PATH"
# 2) 保证动态库能被加载
export DYLD_LIBRARY_PATH="/usr/local/mysql/lib:$DYLD_LIBRARY_PATH"

echo "编译 IndexReader 测试程序..."

g++ -std=c++17 \
    indxreadermain.cpp IndexReader.cpp \
    -I/usr/local/mysql/include \
    -L/usr/local/mysql/lib \
    -Wl,-rpath,/usr/local/mysql/lib \
    -lmysqlclient \
    -o test_indexreader

if [ $? -ne 0 ]; then
    echo "编译失败，请检查 MySQL 安装路径及依赖。"
    exit 1
fi

echo "编译成功"

