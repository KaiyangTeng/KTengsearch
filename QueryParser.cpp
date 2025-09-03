#include "QueryParser.h"

std::vector<std::string> QueryParser::parse(const std::string &query)
{
    return tokenizer_.getstrs(query);
}
