#include <vector>
#include <string>
#include <iostream>
#include <gumbo.h>
#include "URL_getter.h"
using namespace std;

URL_getter::URL_getter(){}

URL_getter::~URL_getter()
{
	urls.clear();
}


//"<a\\s+(?:[^>]*?\\s+)?href=\\\"(/wiki/[^:\\\"#?\\s]+)\\\""



// void URL_getter::get(const string &response,const string& page_url)
// {
// 	int i=0;
//     for(;i<response.size();i++)
//     {
//         if(response[i]=='h')
//         {
//             string sub=response.substr(i,6);
//             string comp="href=";
//             comp+='"';
//             if(sub==comp)
//             {
//                 string temp="";
//                 i+=6;
//                 while(response[i]!='"')
//                 {
//                     temp+=response[i];
//                     i++;
//                 }
//                 urls.push_back(temp);
//             }

//         }
//     }
// }




string resolve_url(const string& raw, const string& base) 
{
    if (raw.rfind("http://", 0) == 0 || raw.rfind("https://",0) == 0) return raw;

    // 以 '//' 开头，继承协议
    if (raw.rfind("//", 0) == 0) 
    {
        auto pos = base.find("://");
        return base.substr(0, pos) + raw;
    }

    std::string scheme_host = base.substr(0, base.find('/', base.find("://")+3));
    if (raw[0] == '/') return scheme_host + raw;


    // 相对路径：截取 base 的目录
    string dir = base.substr(0, base.find_last_of('/'));
    vector<string> parts;
    string tmp;
    for (char c : (dir + "/" + raw)) 
    {
        if (c == '/') 
        {
            if (tmp == ".." && !parts.empty()) parts.pop_back();
            else if (tmp != "." && tmp.size()) parts.push_back(tmp);
            tmp.clear();
        } 
        else tmp.push_back(c);
    }
    if (!tmp.empty()) parts.push_back(tmp);
    // 合并
    string out = parts.front();
    for (size_t i = 1; i < parts.size(); i++) out += "/" + parts[i];
    return out;
}

// 递归遍历，收集 href + 处理 <base>
void collect_links(GumboNode* node, string& base_href,vector<string>& urls) 
{
    if (node->type != GUMBO_NODE_ELEMENT) return;
    GumboAttribute* attr;

    // 1) 先看 <base href="...">
    if (node->v.element.tag == GUMBO_TAG_BASE && (attr = gumbo_get_attribute(&node->v.element.attributes, "href"))) 
    {
        base_href = attr->value;
    }
    // 2) 处理 <a href="...">
    if (node->v.element.tag == GUMBO_TAG_A && (attr = gumbo_get_attribute(&node->v.element.attributes, "href"))) 
    {
        string raw = attr->value;
        urls.push_back(resolve_url(raw, base_href.empty() ? "" : base_href));
    }
    // 3) 深度遍历子节点

    GumboVector* children = &node->v.element.children;
    for (unsigned i = 0; i < children->length; ++i) collect_links(static_cast<GumboNode*>(children->data[i]),base_href,urls);
    
}




void URL_getter::get(const string &response,const string& page_url)
{
    GumboOutput* output = gumbo_parse(response.c_str());
    string base = page_url;
    collect_links(output->root, base, urls);
    gumbo_destroy_output(&kGumboDefaultOptions, output);
}






















