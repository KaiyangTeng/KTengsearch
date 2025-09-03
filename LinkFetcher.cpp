#include "LinkFetcher.h"
#include <iostream>
#include <set>
#include <unordered_set>
#include <sstream>
using namespace std;




LinkFetcher::LinkFetcher()
{
	hgetter=new HTML_getter();
	ugetter=new URL_getter();
	ufilter=new URL_filter();
	uchecker=new URL_checker();
	congetter=new Content_getter();
}

LinkFetcher::~LinkFetcher()
{
	delete hgetter;
    delete ugetter;
    delete ufilter;
    delete uchecker;
    delete congetter;
}

string LinkFetcher::extractDomain(const string& url) //regex
{
    size_t p = url.find("://");
    if (p == string::npos) return "";
    size_t start = p + 3;
    size_t slash = url.find('/', start);
    return url.substr(0, slash);
}

void LinkFetcher::parseRobots(const string& domain)
{
	robotsRules[domain].clear();
	string bottxt=domain+"/robots.txt";
	string txtfile=hgetter->get(bottxt);
	// cout<<txtfile<<endl;
	istringstream ss(txtfile);
	//cout<<endl<<endl<<"++++++++++++++++++++++++++++"<<endl<<endl;
    string line;
    bool startcollect=false;
    while(getline(ss,line))
    {
    	// cout<<line<<endl;
    	if(line.empty()||line[0]=='#') continue;
    	if(line.substr(0,12)=="User-agent: ") 
    	{
    		// cout<<line.substr(12)<<endl;
    		if(line.substr(12,1)=="*")
    		{
    			startcollect=true;
    		}
    		else startcollect=false;
    	}
    	if(startcollect&&line.substr(0,10)=="Disallow: ")
    	{
    		//cout<<line.substr(10)<<endl;
    		robotsRules[domain].push_back(line.substr(10));
    	}
    }
}



bool LinkFetcher::isAllowed(const string& url)
{
	if(url.size()==0) return false;
	string domain=extractDomain(url);
	if(!robotsRules.count(domain)) parseRobots(domain);

	string path=url.substr(domain.size());
	if(path.empty()) path+='/';

	for(auto it:robotsRules[domain])
	{
		if(it=="/"||path.find(it)==0)
		{
			//cout<<url<<" "<<it<<endl;
			return false;
		}
	}
	return true;
}



set<string> LinkFetcher::get_all_urls(const string& url)
{
	if(!isAllowed(url))
	{
		cout<<"Skipped by robots.txt: "<<url<<endl;
		return {};
	}
	ugetter->urls.clear();
    ufilter->filtered_url.clear();


	string response=hgetter->get(url);
	//cout<<"response: "<<response<<endl;


	ugetter->get(response,url);
	ufilter->filtrate(ugetter->urls,url);
	return ufilter->filtered_url;
}


PageData LinkFetcher::get_all_info(const std::string& url) 
{
    PageData out;
    out.ok = false;
    if (!isAllowed(url)) return out;

    std::string html = hgetter->get(url);      // ① 下载
    long status_code = hgetter->last_http_code;
    if (status_code < 200 || status_code >= 400 || html.empty()) return out;
	
	out.ok = true;

    // ② 抽链接
    ugetter->urls.clear();
    ufilter->filtered_url.clear();

    ugetter->get(html, url);
    ufilter->filtrate(ugetter->urls, url);
    out.links=ufilter->filtered_url;
    
    // ③ 抽内容
    auto [title, text]=congetter->extract(html);
    out.title=std::move(title);
    out.content=std::move(text);
    return out;
}


// set<string> LinkFetcher::get_all_urls(const string& url,unordered_set<string>& invalidurl)
// {
// 	string response=hgetter->get(url);

	
// 	//cout<<"response: "<<response<<endl;
// 	ugetter->get(response);
// 	ufilter->filtrate(ugetter->urls,url);
// 	set<string> result;

//     for (const auto& it:ufilter->filtered_url)
//     {
//     	if(invalidurl.count(it)) continue;
//         if (uchecker->is_valid(it))
//         {
//             result.insert(it);
//         }
//         else
//         {
//         	invalidurl.insert(it);
//             cout<<"无效链接已跳过: "<<it<< endl;
//         }
//     }

// 	return result;
// }







