#include <vector>
#include <string>
#include <iostream>
using namespace std;



class URL_getter
{
public:
	vector<string> urls;
	URL_getter();
	~URL_getter();
	void get(const string &response,const string& page_url);
};