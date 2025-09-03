#include "HTML_getter.h"
#include <iostream>
using namespace std;




size_t HTML_getter::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    std::string* output = static_cast<std::string*>(userp);
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

HTML_getter::HTML_getter()
{
	curl=curl_easy_init();
	if(!curl) cerr << "CURL 初始化失败" << endl;
}


HTML_getter::~HTML_getter()
{
	if(curl) curl_easy_cleanup(curl);
}

string HTML_getter::get(const string &url)
{
	string response;
    last_http_code=0;
    if (curl) 
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3L);  
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);         
        CURLcode res = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &last_http_code);
        if (res != CURLE_OK)
        {
            cout<<"invalid URL";
            return "";
        }
        if (last_http_code != 200) {
            cerr << "HTTP 状态非 200: " << last_http_code << endl;
            return "";
        }
    }
    //std::cout << "Response of: " << url << std::endl;
    return response;
}






