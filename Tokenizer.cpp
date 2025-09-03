#include "Tokenizer.h"
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <unordered_set>
using namespace std;



vector<string> Tokenizer::getstrs(const string& text)
{
	unordered_set<string> stopwords={
        "a", "an", "and", "the", "of", "is", "to", "in", "that", "it",
        "on", "for", "with", "as", "by", "at", "from", "this", "be", "or"};

	vector<string> tokens;
	string temp="";
	for(auto it:text)
	{
		if (std::isalnum(static_cast<unsigned char>(it))) 
		{
            temp += std::tolower(static_cast<unsigned char>(it));
        } 
		else
		{
			if(!stopwords.count(temp)&&!temp.empty())tokens.push_back(temp);
			temp.clear();
		}
	}
	if(!stopwords.count(temp)&&!temp.empty())tokens.push_back(temp);
	return tokens;
}

