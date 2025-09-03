#include <vector>
#include <string>
#include <iostream>
#include <unordered_set>
#include "URL_filter.h"
using namespace std;



URL_filter::URL_filter()
{
	useless_surfix={".png", ".jpg", ".jpeg", ".gif", ".bmp", ".svg", ".webp",".mp4", ".avi", ".mov", ".mkv", ".flv",".mp3", ".wav", ".aac", ".flac",".pdf", ".doc", ".docx", ".xls", ".xlsx", ".ppt", ".pptx",".zip", ".rar", ".7z", ".tar", ".gz",".js", ".css", ".ico", ".json", ".xml"};
}

URL_filter::~URL_filter()
{
	useless_surfix.clear();
	filtered_url.clear();
}

// void URL_filter::filtrate(const vector<string>& urls,const string &weblink)
// {
// 	int indx=8;
//     while(weblink[indx]!='/'&&indx<weblink.size()) indx++;

//     string base=weblink.substr(0,indx);


//     for(int i=0;i<urls.size();i++)
//     {
//         string surfix="";
//         int j=urls[i].size()-1;
//         while(j>=0&&urls[i][j]!='.')
//         {
//             surfix+=urls[i][j];
//             j--;
//         }
//         surfix+='.';
//         reverse(surfix.begin(),surfix.end());
//         if(!useless_surfix.count(surfix))
//         {
//             if(urls[i][0]!='/'&&urls[i].substr(0,6)=="https:")
//             {
//                 filtered_url.insert(urls[i]);
//             }
//             else if(urls[i][0]=='/')filtered_url.insert(base+urls[i]);
//         }
//     }
// }







void URL_filter::filtrate(const vector<string>& urls, const string& weblink)
{

    auto get_suffix = [](const string& url) -> string {
        size_t pos = url.rfind('.');
        if (pos == string::npos) return "";
        return url.substr(pos);
    };

    auto contains_useless_keyword = [](const string& url) -> bool {
        static unordered_set<string> useless_keywords = {
            "login", "signup", "register", "logout", 
            "share?", "intent?", "mailto:", "tel:",
            "javascript:", "#", "twitter.com/share"
        };
        for (const string& kw : useless_keywords)
            if (url.find(kw) != string::npos)
                return true;
        return false;
    };

    filtered_url.clear();

    for (const string& it : urls)
    {
        if (!(it.rfind("http://", 0) == 0 || it.rfind("https://", 0) == 0)) continue;
            
        if (it.substr(0, 11) == "javascript:") continue;
        
        if (contains_useless_keyword(it)) continue;

        string suffix = get_suffix(it);
        if (useless_surfix.count(suffix)) continue;

        filtered_url.insert(it);
    }
}




// void URL_filter::filtrate(const vector<string>& urls, const string& weblink)
// {
//     auto get_base_url = [](const string& full_url) -> string {
//         size_t pos = full_url.find("://");
//         if (pos == string::npos) return "";
//         pos += 3;
//         size_t slash = full_url.find('/', pos);
//         if (slash == string::npos) return full_url;
//         return full_url.substr(0, slash);
//     };

//     auto get_suffix = [](const string& url) -> string {
//         size_t pos = url.rfind('.');
//         if (pos == string::npos) return "";
//         return url.substr(pos);
//     };

//     auto contains_useless_keyword = [](const string& url) -> bool {
//         static unordered_set<string> useless_keywords = {
//             "login", "signup", "register", "logout", 
//             "share?", "intent?", "mailto:", "tel:",
//             "javascript:", "#", "twitter.com/share"
//         };
//         for (const string& kw : useless_keywords)
//             if (url.find(kw) != string::npos)
//                 return true;
//         return false;
//     };

//     string base = get_base_url(weblink);

//     for (const string& url : urls)
//     {
//         if (url.empty()) continue;
//         if (url[0] == '#') continue;
//         if (url.substr(0, 11) == "javascript:") continue;
//         if (contains_useless_keyword(url)) continue;

//         string suffix = get_suffix(url);
//         if (useless_surfix.count(suffix)) continue;

//         // 绝对链接 https://xxx.com/...
//         if (url.substr(0, 6) == "https:")
//             filtered_url.insert(url);
//         // 相对路径 /xxx/xxx
//         else if (url[0] == '/')
//             filtered_url.insert(base + url);
//     }
// }

