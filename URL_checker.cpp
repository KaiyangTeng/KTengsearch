// #include "URL_checker.h"
// #include <curl/curl.h>

// bool URL_checker::is_valid(const string& url)
// {
//     CURL* curl = curl_easy_init();
//     if (!curl) return false;

//     curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
//     curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); // 不下载正文，只获取响应头
//     curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // 跟随跳转
//     curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L); // 超时保护

//     CURLcode res = curl_easy_perform(curl);

//     long response_code = 0;
//     curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

//     curl_easy_cleanup(curl);

//     return (res == CURLE_OK) && (response_code == 200);
// }
#include "URL_checker.h"
#include <curl/curl.h>

// 一个 dummy 写入函数，不保存内容，只是防止崩溃
size_t DummyWriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    return size * nmemb;
}

bool URL_checker::is_valid(const std::string& url)
{
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);         // 自动跟随跳转
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2L);                // 最长总时长：2秒
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 1L);         // TCP连接超时：1秒
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, DummyWriteCallback); // 忽略正文
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, nullptr);         // 不接收数据

    CURLcode res = curl_easy_perform(curl);

    long response_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_easy_cleanup(curl);

    return (res == CURLE_OK) && (response_code == 200);
}







