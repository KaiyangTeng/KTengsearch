#include <iostream>
#include <set>
#include <unordered_set>
#include <mutex>
#include <thread>
#include <chrono>
#include <vector>
#include "LinkFetcher.h"
using namespace std;


// LinkFetcher *fetcher;


// //scu.edu, stanford.edu



// set<string> Url_bank;
// unordered_set<string> Inalid_url_bank;


// void dfs(set<string> urls,unordered_set<string>& visited_url,int level)
// {
// 	if(level==0) return;
// 	for(auto it:urls)
// 	{
// 		if(!visited_url.count(it))
// 		{
// 			visited_url.insert(it);
// 			string temp=it;
// 			set<string> res=fetcher->get_all_urls(temp,Inalid_url_bank);
// 			Url_bank.insert(res.begin(),res.end());
// 			dfs(res,visited_url,level-1);
// 		}
// 	}
// }
//scu.edu, stanford.edu









set<string> Url_bank;
unordered_set<string> visited_url;
// queue<pair<string,int>> q;

mutex queue_mutex;
mutex visited_mutex;
mutex bank_mutex;
mutex count_mutex;
int active_threads=0;
int level=2;
int MAX_THREADS = thread::hardware_concurrency()>0 ? thread::hardware_concurrency():4;





void dfs(const set<string>& urls, int level);

void releaseSlot() 
{
    lock_guard<mutex> lock(count_mutex);
    --active_threads;
}

void waitForSlot()
{
	while(true)
	{
		{
			lock_guard<mutex> lock(count_mutex);
			if(active_threads<MAX_THREADS)
			{
				++active_threads;
				return;
			}
		}
		this_thread::sleep_for(chrono::milliseconds(50));
	}
}

void worker(string url,int level)
{
	bool should_continue=false;
	{
		lock_guard<mutex> lock(visited_mutex);
		if(!visited_url.count(url))
		{
			visited_url.insert(url);
			should_continue=true;
		}
	}

	if(should_continue)
	{
		LinkFetcher local_fetcher;
		set<string> res=local_fetcher.get_all_urls(url);
		{
			lock_guard<mutex> lock(bank_mutex);
			Url_bank.insert(res.begin(),res.end());
		}
		dfs(res,level-1);
	}
	releaseSlot();
}




void dfs(const set<string>& urls,int level)
{
	if(level==0) return;
	vector<thread> threads;
	for(const auto& it:urls)
	{
		waitForSlot();
		threads.emplace_back(worker,it,level);
	}
	for(auto& it:threads) if (it.joinable()) it.join();
}



// void bfs(const set<string>& urls)
// {

// 	while(!q.empty())
// 	{
// 		pair<string,int> curr=q.top();
// 		q.pop();
// 		if(curr.second>level) continue;






// 	}

// }




int main()
{
	string url1="https://www.youtube.com";//https://www.youtube.com/stanford
	string url2="https://www.google.com/";
	//fetcher=new LinkFetcher();

	


	set<string> urls={url2};
	// unordered_set<string> visited_url;
	

	dfs(urls,2);




	cout<<endl<<endl<<"---------------"<<endl<<endl;

	for(auto it:Url_bank) cout<<it<<endl;






	// set<string> res=fetcher->get_all_urls(url);
	// for(auto it:res) cout<<it<<" "<<endl;




	return 0;
}









