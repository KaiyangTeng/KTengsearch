#include <iostream>
#include <curl/curl.h>
#include <vector>
#include <algorithm>
#include <unordered_set>
using namespace std;




size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

int main() 
{
    CURL* curl = curl_easy_init();
    string response;
    string weblink="https://www.bilibili.com/";
    





    if (curl) 
    {
        curl_easy_setopt(curl, CURLOPT_URL, weblink.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
    
        curl_easy_cleanup(curl);
    }
    std::cout << "Response:\n" << response << std::endl;







    vector<string> urls;
    int i=0;
    for(;i<response.size();i++)
    {
        if(response[i]=='h')
        {
            string sub=response.substr(i,6);
            string comp="href=";
            comp+='"';
            if(sub==comp)
            {
                string temp="";
                i+=6;
                while(response[i]!='"')
                {
                    temp+=response[i];
                    i++;
                }
                urls.push_back(temp);
            }

        }
    }








    cout<<endl<<"------------------------------"<<endl;
    
    for(int i=0;i<urls.size();i++) cout<<urls[i]<<endl;

    unordered_set<string> useless_surfix={".png", ".jpg", ".jpeg", ".gif", ".bmp", ".svg", ".webp",".mp4", ".avi", ".mov", ".mkv", ".flv",".mp3", ".wav", ".aac", ".flac",".pdf", ".doc", ".docx", ".xls", ".xlsx", ".ppt", ".pptx",".zip", ".rar", ".7z", ".tar", ".gz",".js", ".css", ".ico", ".json", ".xml"};
    vector<string> filtered_url;

    int indx=8;
    while(weblink[indx]!='/'&&indx<weblink.size()) indx++;

    string base=weblink.substr(0,indx);


    for(int i=0;i<urls.size();i++)
    {
        string surfix="";
        int j=urls[i].size()-1;
        while(j>=0&&urls[i][j]!='.')
        {
            surfix+=urls[i][j];
            j--;
        }
        surfix+='.';
        reverse(surfix.begin(),surfix.end());
        if(!useless_surfix.count(surfix))
        {
            if(urls[i][0]!='/'&&urls[i].substr(0,6)=="https:")
            {
                filtered_url.push_back(urls[i]);
            }
            else if(urls[i][0]=='/')filtered_url.push_back(base+urls[i]);
        }
    }

    

    cout<<endl<<"------------------------------"<<endl;



    for(int i=0;i<filtered_url.size();i++) cout<<filtered_url[i]<<endl;
    return 0;
}





















