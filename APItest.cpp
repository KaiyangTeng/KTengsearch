// #include <iostream>
// #include <iomanip>
// #include <string>
// #include <vector>
// #include "SearchEngine.h"
// using namespace std;

// int main() 
// {
// 	SearchEngine engine("IP","dbusrname","key","dbname",3306,10);
//     string query = "professor";
//     auto hits = engine.search(query);
//     if (hits.empty()) std::cout << "  no result\n\n";
            
//    	for (const auto& h : hits) 
//    	{
//             std::cout << h.title  << "\n"
//                       << "    " << h.url << "\n"
//                       << "    " << h.snippet << "\n"
//                       << "    score: " << h.score
//                       << "\n\n";
//     }
//     return 0;
// }


#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <regex>
#include "SearchEngine.h"
#include "nlohmann/json.hpp"
using nlohmann::json;

// UTF-8清理函数：移除无效的UTF-8字节序列
static std::string cleanUTF8(const std::string& input) {
    std::string result;
    result.reserve(input.size());
    
    for (size_t i = 0; i < input.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(input[i]);
        
        // ASCII字符 (0-127)
        if (c < 0x80) {
            result += c;
            continue;
        }
        
        // 检查UTF-8多字节序列
        if ((c & 0xE0) == 0xC0) {  // 2字节序列
            if (i + 1 < input.size() && 
                (static_cast<unsigned char>(input[i + 1]) & 0xC0) == 0x80) {
                result += c;
                result += input[i + 1];
                ++i;
            }
        } else if ((c & 0xF0) == 0xE0) {  // 3字节序列
            if (i + 2 < input.size() && 
                (static_cast<unsigned char>(input[i + 1]) & 0xC0) == 0x80 &&
                (static_cast<unsigned char>(input[i + 2]) & 0xC0) == 0x80) {
                result += c;
                result += input[i + 1];
                result += input[i + 2];
                i += 2;
            }
        } else if ((c & 0xF8) == 0xF0) {  // 4字节序列
            if (i + 3 < input.size() && 
                (static_cast<unsigned char>(input[i + 1]) & 0xC0) == 0x80 &&
                (static_cast<unsigned char>(input[i + 2]) & 0xC0) == 0x80 &&
                (static_cast<unsigned char>(input[i + 3]) & 0xC0) == 0x80) {
                result += c;
                result += input[i + 1];
                result += input[i + 2];
                result += input[i + 3];
                i += 3;
            }
        }
        // 无效的UTF-8字节被跳过
    }
    
    return result;
}

static std::string join_args(int argc, char* argv[], int start = 1) {
    std::ostringstream oss;
    for (int i = start; i < argc; ++i) {
        if (i > start) oss << ' ';
        oss << argv[i];
    }
    return oss.str();
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " \"search query\"\n";
        return 1;
    }
    
    std::string query = join_args(argc, argv, 1);

    
    SearchEngine engine("IP","dbusrname","key","dbname",3306,10);

    auto hits = engine.search(query);

    // hits -> JSON 数组
    json j = json::array();
    for (const auto& h : hits) {
        j.push_back({
            {"docID",   h.docID},
            {"score",   h.score},
            {"url",     cleanUTF8(h.url)},
            {"title",   cleanUTF8(h.title)},
            {"snippet", cleanUTF8(h.snippet)}
        });
    }

    
    std::cout << j.dump(2) << std::endl;
    return 0;
}




