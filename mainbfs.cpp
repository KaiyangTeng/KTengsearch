#include <iostream>
#include <set>
#include <unordered_set>
#include <mutex>
#include <thread>
#include <chrono>
#include <vector>
#include <queue>
#include <atomic>
#include <string>
#include "LinkFetcher.h"
#include "RateLimiter.h"
#include "DB_manager.h"
using namespace std;

set<string> Url_bank;
unordered_set<string> visited_url;
queue<pair<string,int>> q;
atomic<bool> stop_crawl(false);
const size_t MAX_URLS = 3500;



mutex queue_mutex;
mutex visited_mutex;
mutex bank_mutex;
mutex count_mutex;
int active_threads=0;
int MAX_THREADS = thread::hardware_concurrency()>0 ? thread::hardware_concurrency():4;



static RateLimiter globalLimiter(20, std::chrono::seconds(1));
DB_manager db("crawler.db");


void releaseSlot() 
{
    lock_guard<mutex> lock(count_mutex);
    --active_threads;
}

void waitForSlot()
{

	while(true)
	{
		if (stop_crawl) return;
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

void thread_worker(string url,int level)
{
	if(stop_crawl) 
	{ 
		releaseSlot(); 
		return; 
	}

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
		globalLimiter.acquire();
		LinkFetcher local_fetcher;
		set<string> res=local_fetcher.get_all_urls(url);
		{
			lock_guard<mutex> lock(bank_mutex);
			Url_bank.insert(res.begin(),res.end());
			if(Url_bank.size()>=MAX_URLS) stop_crawl=true;
		}

		if(!stop_crawl&&level>1)
		{
			lock_guard<mutex> lock(queue_mutex);
			for(const auto& it:res) 
			{
				q.push({it,level-1});
			}
		}
		db.insert_url(url, "Success"); //enum url_status {0: Success 1: Failed 2: Retried}
		//db.insert_url(url, "Success") => db.insert_url(url, status)
	}
	releaseSlot();
}




void bfs(const set<string>& urls,int level)
{
	{
		lock_guard<mutex> lock(queue_mutex);
		for(const auto& it:urls) 
		{
			q.push({it,level});
		}
	}

	vector<thread> threads;
	while(true)
	{
		if(stop_crawl) break;
		pair<string,int> curr;
		{
			lock_guard<mutex> lock(queue_mutex);
			if(q.empty()&&active_threads==0) break;
            if(q.empty()) continue;
			curr=q.front();
			q.pop();
		}
		waitForSlot();
		if(stop_crawl) break;
		threads.emplace_back(thread_worker,curr.first,curr.second);
	}

	for(auto& it:threads) if (it.joinable()) it.join();
}






int main()
{
	string url1="https://www.youtube.com";//https://www.youtube.com/stanford
	string url2="https://www.youtube.com/";
	


	set<string> urls={url2};
	
	

	bfs(urls,3);


	cout<<endl<<endl<<"---------------"<<endl<<endl;

	for(auto it:Url_bank) cout<<it<<endl;







	return 0;
}











