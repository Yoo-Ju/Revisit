#include "debug.h"
#include <vector>
#include <set>
#include "utilities.h"
#include "KDTree.h"  // Asmodehn's KDTree implementation

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

// KDTREE Object
class H_KDobj
{
public:
	int id;
	static const H_KDobj ERROR;
	H_KDobj(const H_KDobj& in) {id=in.id;};
	H_KDobj(int new_id=-1) { id=new_id;};
	
};

bool operator< (const H_center& h1, const H_center& h2); 

enum indexing_strategy {HOMEMADE, KDTREE};

class H_store
{
public:
	vector<H*> rectangles;
	indexing_strategy strategy;
	/// Homemafe index
	vector< set<H_center> > index;
	/// KDTREE index
	KDTree<H_KDobj> *kd_index;
	int dim;
	/// Used to count number of erasures (for HOMEMADE index) or insertions (for KDTree)
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
	set<int> *kdtree_intersect_list(H& h, float tau);
	void change_index_strategy(indexing_strategy new_strategy);
	/// Constructor requires to know the space dimensionality
	H_store(int n, indexing_strategy strat) 
	{ 
		strategy=strat;

		set<H_center> empty; 
		switch(strategy) { 
			case HOMEMADE:
				kd_index=0;
				for(int i=0; i<n; i++)
					index.push_back(empty);
				break;
			case KDTREE:
				dimensions=n; // Global variable used by KDtree
				kd_index=checked_new(KDTree<H_KDobj>);
				break;
			default:
				launch_error("Which indexing structure should I use for H_store??");
		};
		dim=n; 
		stats_count=0; 
	};
	~H_store() { checked_delete(kd_index); };
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





