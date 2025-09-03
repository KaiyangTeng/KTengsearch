#include <iostream>
#include "IndexReader.h"
using namespace std;
int main() 
{
	IndexReader reader("13.59.130.88","crawler","12345678","crawler_db",3306);
	auto postings = reader.getPostingList("resilience");
	cout<<"total doc count: "<<reader.getTotalDocCount();
	cout<<"term freq: "<<reader.getDocFreq("resilience")<<endl;
	cout<<"Doc 1 len: "<<reader.getDocLength(1)<<endl;
	cout<<"avg doc len: "<<reader.getAvgDocLength()<<endl;
	for(auto it:postings)
	{
		cout<<"Doc ID: "<<it.docID<<" frequency: "<<it.frequency<<endl;
	}



}