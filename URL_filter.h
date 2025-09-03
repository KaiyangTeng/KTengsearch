#include <vector>
#include <string>
#include <iostream>
#include <unordered_set>
#include <set>
using namespace std;

class URL_filter
{
public:
	set<string> filtered_url;
	unordered_set<string> useless_surfix;
	URL_filter();
	~URL_filter();
	void filtrate(const vector<string>& urls, const string& weblink);

};










