#include <string>
#include <curl/curl.h>
using namespace std;

class HTML_getter 
{
public:
	HTML_getter();
	string get(const string &url);
	~HTML_getter();
	long last_http_code=0;
private:
    CURL *curl;  
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
};