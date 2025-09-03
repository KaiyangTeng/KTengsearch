#include <string>
#include <vector>
#include "Tokenizer.h"

class QueryParser {
public:
    QueryParser() = default;
    ~QueryParser() = default;
    std::vector<std::string> parse(const std::string &query);

private:
    Tokenizer tokenizer_;
};
