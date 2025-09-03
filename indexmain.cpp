#include <iostream>
#include <string>
#include "Indexer.h"
int main() 
{
    Indexer indexer("18.219.206.82", "crawler","12345678","crawler_db",3306);
    std::cout << "开始全量索引..." << std::endl;
    indexer.buildAll();
    std::cout << "倒排索引构建完成！" << std::endl;
    return 0;
}
