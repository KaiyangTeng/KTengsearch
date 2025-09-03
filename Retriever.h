#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "IndexReader.h"
using namespace std;
class Retriever {
public:
     Retriever(const std::string& host,
              const std::string& usrname,
              const std::string& paswrd,
              const std::string& dbname,
              unsigned int port = 3306);
    std::unordered_map<int, std::vector<Docinfo>> retrieve(const std::vector<std::string> &terms);
private:
    IndexReader reader_;
};
