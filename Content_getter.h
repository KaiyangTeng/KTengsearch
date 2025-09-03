#pragma once
#include <string>
#include <gumbo.h>
using namespace std;


class Content_getter {
public:
    // 返回 <title>, 可见正文
    pair<string,string> extract(const string& html);
};
