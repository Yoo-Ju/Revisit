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

#include "debug.h"
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include <fstream>
#include "name_map.h"
#include "utilities.h"
#include "Hcubes.h"
//#include "trajectory.h"

#pragma once

using namespace std;

enum projection_type {ENLARGE, EXTEND};

#define MAX_SEQUENCE_LEN 1000
extern int min_sup;
extern name_map name_mapping;
extern int ANNOTATION_PARTITIONS;
extern fstream* TProjection_dump_file;
extern int TProjection_dump_size;
extern bool just_init;
extern Statistics mystat;
//extern int DUMP_BUFFER_SIZE;
//extern int elem_count,entry_count,elem_count_t,entry_count_t;

#define TProjection_dump_file_seekp(pos, code) TProjection_dump_file->seekp(pos)
#define TProjection_dump_file_seekg(pos, code) TProjection_dump_file->seekg(pos)
//// Slower version, that checks file access
//#define TProjection_dump_file_seekp(pos, code) TProjection_dump_file_checked_seekp(pos,code)
//#define TProjection_dump_file_seekg(pos, code) TProjection_dump_file_checked_seekg(pos,code)

void TProjection_dump_file_checked_seekp(int pos, char code);
void TProjection_dump_file_checked_seekg(int pos, char code);

/// Useful template for writing binary data


//template<class T> int mywrite(T x);


/// Useful template for reading binary data

//template<class T> void myread(T& x);

class dump_switcher
{
	vector<fstream*> file_table;
	vector<int> dump_size_table;
	int actual_prefix_size;
	int allocated_streams;
public:
	dump_switcher(): actual_prefix_size(-1), allocated_streams(0) {};
	~dump_switcher()
	{
		for(int i=0; i<allocated_streams; i++)
		{
			file_table[i]->close();
			checked_delete(file_table[i]);
		};
	};
	/// Switch context for dump files to specified prefix_size (= number of items in the prefix)
	void operator () (int prefix_size)
	{
		if(prefix_size<0 || prefix_size>500) // Allowed number of dump files (and also max pattern length)
		{
			cerr << "Called dump_switcher(" << prefix_size << ")";
			launch_error("Switch to invalid prefix_size requested!!");
		};
		if(prefix_size!=actual_prefix_size)
		{
			// Create new stream(s) if necessary
			while(allocated_streams <= prefix_size)
			{
				
				string file_name("MiSTA.dump.");
				file_name += (char)((int)'0'+((allocated_streams/100) % 10)); 
				file_name += (char)((int)'0'+((allocated_streams/10) % 10));
				file_name += (char)((int)'0'+(allocated_streams % 10));
				fstream *new_stream = new fstream;
				new_stream->open(file_name.c_str(), ios::in | ios::out | ios::binary | ios::trunc);
				file_table.push_back(new_stream);
				dump_size_table.push_back(0);
				allocated_streams++;
			};
			// Dump previous file size
			if(actual_prefix_size>=0)
				dump_size_table[actual_prefix_size]=TProjection_dump_size;
			// Retrieve actual file and size
			TProjection_dump_file=file_table[prefix_size];
			TProjection_dump_size=dump_size_table[prefix_size];
			actual_prefix_size=prefix_size;
		};
	};
};

// Declare a global one
extern dump_switcher dump_context;

/// Set of items
class Element: public vector<int>
{
public:
	int contains(Element& elem);
};

/// Simple sequence = sequence of itemsets
class Sequence: public vector<Element> 
{};
	
/// Element plus time
class TElement: public Element
{
public:
	/// Time-stamp of each element
	float time;

	/// Create a copy of whole element (time included)
	TElement *duplicate();

	/// Create a partial copy of element (time included)
	TElement *duplicate_from(unsigned int first_item);

	//TElement() { elem_count++; elem_count_t++;};
	//~TElement() { elem_count--; };
};

class TItemPointer;
class EntryPoint;

/// Temporal projected sequence (w.r.t. a given prefix) with its entrypoints
class TSequence: public vector<TElement*>
{
public:
	/// Times and entry/continuation points for the prefix
	vector<EntryPoint> *annotations;
		
	int id;
	
	/// Default: the vector for annotations is not allocated
	TSequence() { annotations=0; };

	/// Destructor
	~TSequence() { clear_all(); };

	/// Clear TSequence, deallocating everything
	void clear_all()
	{ 
		checked_delete(annotations); 
		annotations=0;
		for(vector<TElement*>::iterator it=begin(); it!=end(); it++) checked_delete(*it); 
		this->clear();
	};

	/// Create a copy of whole TSequence, WITHOUT setting annotations
	TSequence *duplicate();

	/// Create a partial copy of TSequence, WITHOUT setting annotations (if elem. index<0 => copy all)
	TSequence *duplicate_from(TItemPointer from_item);

};

/// Projection of the database w.r.t. a given prefix
class TProjection: public vector<TSequence*> 
{
public:
	/// Prefix of the projected database
	Sequence *prefix;
	/// Frequence of items in the projected database
	map<int,int> item_freq_enlarge, item_freq_extend;
	/// Assign each item with its projection's rough size estimation.
	map<int,int> ext_proj_size, enl_proj_size;
	/// This is the TRUE support of the projection
	int prefix_support;
	int non_empty_sequences;
	/// Position in the dump file (start==-1 => no dump created, yet)
	int dump_file_start;
	int dump_file_size;
	int dump_file_read_pos;
	int dump_file_id;
	
	/// Empty projection
	TProjection() { 
		prefix=checked_new(Sequence); 
		prefix_support=0;
		non_empty_sequences=0;
		dump_file_start=-1; dump_file_size=0;
		dump_file_id=-1;
	};

	/// Build projection from file (=empty prefix)
	TProjection(const char* filename) { 
		prefix=checked_new(Sequence); 
		dump_file_start=-1; dump_file_size=0;
		load_file_tas(filename); 
	}; 

	/// Destructor
	~TProjection() { 
		checked_delete(prefix);
		for(vector<TSequence*>::iterator it=begin(); it!=end(); it++) 
			checked_delete(*it); 
	};

	/// Load Tsequences from old TAS file (only singleton elements)
	void load_file_simple_tas(const char* filename);

	/// Load Tsequences from a TAS file (new version, following SLP syntax: -1=EOElem. -2=EOSeq.)
	void load_file_tas(const char* filename);

	/// Scan actual projection and compute items frequencies
	void set_item_freq();

	/// Delete occurrences of unfrequent items
	void filter_items();

	/// Delete occurrences of unfrequent items from a single Tsequence
	void filter_items_from_sequence(TSequence * tseq, int item);

	/// Extract all projections with (n+1)-length prefix
	vector<TProjection*> *generate_projections();

	/// Project a single TSequence and adjust entrypoints
	TSequence *generate_projected_tsequence(TProjection* proj,
		TProjection::iterator tseq,
		TItemPointer item_pointer,
		projection_type type);

	/// Project a single TSequence WITHOUT adjusting entrypoints
	TSequence *generate_projected_tsequence_simplified(TProjection::iterator tseq,
										TItemPointer item_pointer,
										projection_type type);

	/// Extract a set of rectangles representing frequent annotations
	H_store *extract_dense_rectangles(float tau);
	H_store *extract_dense_rectangles_brute_force(float tau);
	H_store *extract_dense_rectangles_brute_force2(float tau);

	/// Delete useless annotations
	void filter_annotations(H_store *O, float tau);

	/// Evaluate size of the projection (for profiling/stats purposes)
	void evaluate_size(int& nseqs, int& nelems, int& nitems, int& nannots);

	/// Returns a string containing the projection prefix, using only chars in "_0123456789"
	string myname();


	void set_dump_id()
	{
		dump_file_id=0;
		for(Sequence::iterator el=prefix->begin(); el!=prefix->end(); el++)
			dump_file_id+=(int)el->size();
	};

	/// Dump sequence to the given (assumed to be binary) stream
	void dump_tseq_to_disk(TSequence* tseq);

	/// Retrieve sequence from the given (assumed to be binary) stream
	void retrieve_tseq_from_disk(TSequence* tseq, int prefix_len);

	/// Position and write init data (N_sequences and prefix_length)
	void dump_write_init(int N, int prefix_len)
	{
		//cerr << "[WriteInit " << this->myname() << " @ " << TProjection_dump_size;
		// Get starting position
		dump_context(dump_file_id);
		dump_file_start=TProjection_dump_size;
		dump_file_size=0;
		TProjection_dump_file_seekp(dump_file_start,'G');
		//cerr << " (real: " << (int)TProjection_dump_file.tellp() << ")]\n";
		// Write heading
		
		//mywrite(N);
		
		//mywrite(prefix_len);
		
		// Update position
		dump_file_size+=2*sizeof(int);
		TProjection_dump_size+=2*sizeof(int);
	};

	/// Position and read init data (N_sequences and prefix_length)
	void dump_read_init(int& N, int& prefix_len)
	{
		dump_context(dump_file_id);
		if(this->dump_file_start==-1)
		{
			string s="Trying to ReadInit " + this->myname() + " before WriteInit!";
			launch_error(s.c_str());
		};
		//cerr << "[ReadInit " << this->myname() << " @ " << this->dump_file_start; 
		// Position to the starting position in dump file
		TProjection_dump_file_seekg(this->dump_file_start,'H');
		//cerr << " (real: " << (int)TProjection_dump_file.tellg() << ")]\n";
		// Retrieve headings
		//myread(N);
		//myread(prefix_len);
		if(TProjection_dump_file->tellg()<0)
		{
			cout << "Dump context: " << dump_file_id 
				<< "\nMoving dump_file_read_pos from " << dump_file_read_pos << " to " 
				<< TProjection_dump_file->tellg() 
				<< "\nProjection range: " << dump_file_start << "-" << dump_file_start+dump_file_size << endl;
		};

		this->dump_file_read_pos=TProjection_dump_file->tellg();
		//if(TProjection_dump_file.tellg()<0)
		//{
		//	TProjection_dump_file_seekg(0,ios::end);
		//	cout << "\nDump file physical size: " << TProjection_dump_file.tellg() << endl;
		//};

		just_init=1;
		//cerr << " -> N=" << N << " prefix_len=" << prefix_len << "\n"; 
	};

	/// Write the TSequences of TProj on disk
	void dump_on_disk() 
	{
		// Notice: actually, we do not dump "prefix", "item_freq"'s and "prefix_support"
		// Format: N_sequences Prefix_len <Sequences>
		int N=(int)this->size();
		int prefix_len=&(*this->prefix)==0 ? 0 : (int)this->prefix->size();
		dump_write_init(N,prefix_len); // Write N_sequences and Prefix_len
		for(TProjection::iterator it=this->begin(); it!=this->end(); it++)
			dump_tseq_to_disk(*it);
	};

	/// Read the TSequences of TProj from disk
	void retrieve_from_disk()
	{
		//cout << "Retrieve from Disk\n";
		// Notice: actually, we do not dump "prefix", "item_freq"'s and "prefix_support"
		//         Moreover: we assume N_sequences (below) is always correct
		// Format: N_sequences Prefix_len <Sequences>
		int N,prefix_len;
		dump_read_init(N,prefix_len);
		this->clear();
		if(N>0)  // Length of projection present in the file
		{
			this->resize(N);  // OPTIMIZE: is a single "new TSequence[N]" faster ??
			// int i=0;
			// cout << "SIZE: " << this->size() << endl;
			for(TProjection::iterator it=this->begin(); it!=this->end(); it++)
			{
				// cout << "Reading " << i++ << "/" << N << " at " << TProjection_dump_file.tellg() << "\r"; cout.flush();
				*it=checked_new(TSequence);
				retrieve_tseq_from_disk(*it,prefix_len);
			};
		};
	};

};

/// Pointer to a single item in a (t)element 
class TItemPointer 
{
public:
	/// Index of a TElement in a TSequence -- values in [0,tseq.size()-1],  values<0 => NULL
	int idx;
	/// Position of item in the TElement
	unsigned int position;

	/// Default TItemPointer is (0,0): it is the smallest possible, also used with special meaning
	TItemPointer() { idx=-1; position=0; };
	/// Constructor with values for element index and item offset
	TItemPointer(int telem_idx, int pos) { idx=telem_idx; position=pos; };
	// Constructor which derives idx and position from iterators w.r.t. a given TSequence
	TItemPointer(TProjection::iterator seq_pt, TSequence::iterator elem, TElement::iterator item)
		{	idx=static_cast<int>(elem-(*seq_pt)->begin()); position=static_cast<int>(item-(*elem)->begin()); };
	/// Comparison operator
	bool operator>(TItemPointer second) 
		{ return (idx>second.idx) ? 1 : ((idx==second.idx && position>second.position)?1:0); };
	/// Comparison operator
	bool operator<(TItemPointer second) 
		{ return (second>*this); };
	/// Comparison operator
	bool operator<=(TItemPointer second) 
		{ return *this>second ? 0 : 1; };
	/// Comparison operator
	bool operator==(TItemPointer second) 
		{ return (idx==second.idx && position==second.position);};
	/// Comparison operator
	bool operator!=(TItemPointer second) 
		{ return !(*this==second);};
	/// Increment item offset and, if needed, element index
	TItemPointer increment(TProjection::iterator seq_pt)
	{ 
		TSequence::iterator elem=(*seq_pt)->begin()+idx; 
		position++; 
		if(position>=(*elem)->size()) { position=0; idx++; /* No check ! */}; 
		return *this; 
	};
	/// Returns item pointed by TItemPointer. NOTICE: No consistency check!
	int get_item(TProjection::iterator seq_pt) { return (*seq_pt)->at(idx)->at(position); };
};

/// Times for a prefix plus a pointer to a "continuation (or access) point"
class EntryPoint 
{
public:
	/// List of times
	vector<float> times;
	/// Pointer to continuation points
	mutable TItemPointer entrypoint;   // Set to mutable to shut up "entry_point_less_than"...
	//EntryPoint() { entry_count++; entry_count_t++; };
	//~EntryPoint() { entry_count--; };
};

ostream& operator << (ostream& s, TProjection& tp);
ostream& operator << (ostream& s, Sequence& tp);
ostream& operator << (ostream& s, Element& tp);
ostream& operator << (ostream& s, TSequence& tp);
ostream& operator << (ostream& s, TElement& tp);
ostringstream& operator << (ostringstream& s, Sequence& tp);
int get_telem(ifstream& s, TElement* &tp);

class entrypoint_less_than 
{ 
public:
	bool operator ()(const EntryPoint& a, const EntryPoint& b) 
		{return (b.entrypoint > a.entrypoint); };
};

class pair_second_less_than
{ 
public:
	bool operator ()(const pair<int,float>& a, const pair<int,float>& b) 
		{return (b.second > a.second); };
};

// Visible outside only for debug/performance analysis purposes
void brute_force_density_computation2(vector<H>& annot_vector, vector<int>& data_points, 
									 vector<vector<float> >& all_bounds,
									 vector<int>& idx, int i,
									 set<vector<int> >& working);
void coalesce_density_blocks(set<vector<int> >& dense, 
							 vector<vector<float> >& all_bounds, 
							 int n, H_store *working);



