/*
 *  Copyright (C) 2007  Mirco Nanni <mirco.nanni@isti.cnr.it> and
 *                      Fabio Pinelli <fabio.pinelli@isti.cnr.it>
 *                           KDDLab - ISTI-CNR - Pisa, Italy     
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

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


