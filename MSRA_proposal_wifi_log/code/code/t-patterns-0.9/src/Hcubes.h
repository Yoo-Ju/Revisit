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

#include <vector>
#include <set>
#include "utilities.h"
#include "debug.h"

using namespace std;

extern int ERASURE_COUNT_MAX;
extern int INSERTION_COUNT_MAX;

class H
{
public:
	/// Lower bound coordinates of Hyper-rectangle
	vector<float> low;
	/// Upper bound coordinates of Hyper-rectangle
	vector<float> up;
	/// Density of the region covered by Hyper-rectangle
	int density;

	H() { density=0; };
	H(vector<float>& L, vector<float>& U, int den)
	{
		if(L.size()==0 || L.size()!=U.size()) launch_error("Wrong input to \"H\" constructor!!");
		low=L; up=U; density=den;
	};
	H(const H& h_in) { low=h_in.low; up=h_in.up; density=h_in.density;};
};

class H_center
{ 
public:
	int id;
	float center;

	H_center(int new_id, float new_center) { id=new_id; center=new_center; };
	H_center() { id=-1; center=0; };
};

bool operator< (const H_center& h1, const H_center& h2); 

class H_store
{
public:
	vector<H*> rectangles;
	/// Index
	vector< set<H_center> > index;
	int dim;
	/// Used to count number of erasures
	int stats_count;

	/// Access rectangles
	H* at(int idx) { return rectangles.at(idx); };
	/// Add Hyper-rectangle to the repository, and return its ID
	int insert(H& h_add);
	int insert_pt(H* h_pt);
	/// Delete Hyper-rectangle with the given ID from repository
	void erase(int h_erase);
	/// Return a list of hyper-rectangle that intersect the input one for a given tau value
	set<int> *intersect_list(H& h, float tau);
	set<int> *homemade_intersect_list(H& h, float tau);
	/// Constructor requires to know the space dimensionality
	H_store(int n) 
	{ 
		set<H_center> empty; 
		for(int i=0; i<n; i++)
			index.push_back(empty);
		dim=n; 
		stats_count=0; 
	};
	~H_store() { };
	/// Clear & deallocate "rectangles" and indexes on each dimension
	void clear();
	/// Clear (but do not deallocate) rectangles and indexes
	void fast_clear();
};

/// Partition the two intersecting Hs -- other fields of output H have default values
void HPartitioning(H& h, H& h_prime, vector<H>& Out, vector<H>& Out_prime, H& h_intersect);

/// Check for intersection between two Hs
bool H_intersection(H& h1, H& h2);

/// Add all Hs in I to the H_store O, splitting Hs where necessary, and (if add_freq==true) computes frequencies
void combine_rectangle_set(vector<H>& I, H_store *O, bool add_freq, float tau);

ostream& operator << (ostream& s, H h);
ostream& operator << (ostream& s, H_center h);
ostream& operator << (ostream& s, vector<H> v);




