#ifndef __debug_h
#define __debug_h

#include <vector>
#include <iostream>

using namespace std;
extern int new_counter;
extern int delete_counter;

//// NEW & DELETE that count calls
//#define checked_new(X) (new_counter++,new X) 
//#define checked_delete(X) ((X!=0)?(delete_counter++):0,delete X)

// Plain NEW and DELETE
#define checked_new(X) (new X) 
#define checked_delete(X) (delete X)

class Statistics
{
public:
	vector<int> hits;
	vector<float> times_density;
	vector<float> times_coalesce;
	vector<int> blocks_before;
	vector<int> blocks_after;
	int Nproj, Nseq;
	float sigma, tau, avg_seq_len, time;

	Statistics()
	{
		hits.reserve(100);
		times_density.reserve(100);
		times_coalesce.reserve(100);
		blocks_before.reserve(100);
		blocks_after.reserve(100);
		for(int i=0; i<100; i++)
		{
			hits.push_back(0);
			times_density.push_back(0);
			times_coalesce.push_back(0);
			blocks_before.push_back(0);
			blocks_after.push_back(0);
		};
		Nproj=Nseq=0;
		avg_seq_len=sigma=tau=time=0;
	};

	void print()
	{
		int last_d;
		for(last_d=0; hits[last_d+1]>0; last_d++);
		cout << Nseq << "\t" 
			 << sigma << "\t"
			 << tau << "\t"
			 << Nproj << "\t"
			 << avg_seq_len << "\t"
			 << time << "\t"
			 << last_d ;
		for(int i=1; i<=last_d; i++)
		{
			cout << "\t" << hits[i]
				 << "\t" << times_density[i]
				 << "\t" << times_coalesce[i]
				 << "\t" << blocks_before[i]
				 << "\t" << blocks_after[i];
		}
		cout << endl;
	};
};

#endif

