#include "HTML_getter.h"
#include "URL_getter.h"
#include "URL_filter.h"
#include "URL_checker.h"
#include "Content_getter.h"
#include <iostream>
#include <set>
#include <unordered_set>
#include <map>
using namespace std;


struct PageData {
    std::set<std::string> links;
    std::string title;
    std::string content;
    bool ok;              // 是否成功解析到链接 (沿用你的计数口径)
};



class LinkFetcher
{
public:
	LinkFetcher();
	~LinkFetcher();
	set<string> get_all_urls(const string& url);
	PageData get_all_info(const string& url);

private:
	HTML_getter *hgetter;
	URL_getter *ugetter;
	URL_filter *ufilter;
	URL_checker *uchecker;
	Content_getter *congetter;
	map<string, vector<string>> robotsRules;
	void parseRobots(const string& domain);
    bool isAllowed(const string& url);
    string extractDomain(const string& url);
};
















// void dfs(vector<string> urls,unordered_set<string>& visited_url,int level)










// int main()
// {
// 	string url="https://www.youtube.com/stanford/";//https://www.youtube.com/stanford
// 	hgetter=new HTML_getter();
// 	ugetter=new URL_getter();
// 	ufilter=new URL_filter();

// 	string response=hgetter->get(url);
// 	ugetter->get(response);
// 	ufilter->filtrate(ugetter->urls,url);


// 	cout<<endl<<"------------------------------"<<endl;
// 	for(auto it:ugetter->urls) cout<<it<<endl;
// 	cout<<endl<<"------------------------------"<<endl;
// 	for(auto it:ufilter->filtered_url) cout<<it<<endl;


// 	return 0;
// }










