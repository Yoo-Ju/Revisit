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
#include "Tsequences.h"
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

using namespace std;

bool just_init=0;
Statistics mystat;
name_map name_mapping;
int min_sup=0;
int ANNOTATION_PARTITIONS=5;
my_clock local_clock;

//int DUMP_BUFFER_SIZE = 10;
//TSequence *NULL_tseq_ptr=0;

//OLD VERSION: fstream TProjection_dump_file("MiSTA.dump",ios::in | ios::out | ios::binary | ios::trunc);

dump_switcher dump_context;
fstream *TProjection_dump_file;
int TProjection_dump_size=0;

//int elem_count=0;
//int entry_count=0;
//int elem_count_t=0;
//int entry_count_t=0;

void TProjection_dump_file_checked_seekp(int pos, char code)
{
	if(pos<0 || pos>TProjection_dump_size)
		cerr << "WARNING: seekp out of range (" << pos << " / " << TProjection_dump_size << ", code " << code << ")\n";
	TProjection_dump_file->seekp(pos);
};
void TProjection_dump_file_checked_seekg(int pos, char code)
{
	if(pos<0 || pos>=TProjection_dump_size)
		cerr << "WARNING: seekg out of range (" << pos << " / " << TProjection_dump_size << ", code " << code << ")\n";
	TProjection_dump_file->seekg(pos);
};

template<class T> void my_swap(T& a, T& b) { T tmp=a; a=b; b=tmp; }; 

/// Returns -1 if elem is empty, [0 .. this->size()-1] if found, this->size() otherwise
int Element::contains(Element& elem)
{
	if(elem.size()==0)
		return -1;

	Element::iterator pos = this->begin();
	for(Element::iterator sub=elem.begin(); (sub!=elem.end()) && (pos!=this->end()); sub++)
		pos=find(pos,this->end(),*sub);

	return static_cast<int>(pos-this->begin());
};


TElement *TElement::duplicate_from(unsigned int first_item)
{
	if(first_item>=this->size())
		launch_error("Upper boundary overflow in TElement::duplicate_from !!!");
	TElement *tmp = checked_new(TElement);
	tmp->reserve(this->size()-first_item);
	for(TElement::iterator it=this->begin()+first_item; it!=this->end(); it++)
		tmp->push_back(*it);
	tmp->time = time;
	return tmp;
}

TElement *TElement::duplicate()
{
	return duplicate_from(0);
}

TSequence *TSequence::duplicate_from(TItemPointer from_item)
{
	TSequence *new_tseq = checked_new(TSequence);
	if(from_item.idx<0)
		from_item=TItemPointer(0,0);
	TSequence::iterator from_elem=this->begin()+from_item.idx;
	if(from_item.idx < static_cast<int>(this->size()))
	{
		new_tseq->reserve(this->size()-from_item.idx);
		new_tseq->push_back((*from_elem)->duplicate_from(from_item.position));
		for(TSequence::iterator elem=from_elem+1; elem!=this->end(); elem++)
			new_tseq->push_back((*elem)->duplicate());
	}
	return new_tseq;
}

TSequence *TSequence::duplicate()
{
	return duplicate_from(TItemPointer(0,0));
}

/// This method requires whole projections. Optimizations should do the work while building them
void TProjection::set_item_freq()
{
	// DEBUG
	// cout << "[TProj in set_item_freq(): " << *this << "]\n";

	set<int> local_support; // # items occurring in tseq 
	item_freq_extend.clear();
	item_freq_enlarge.clear();
	for(TProjection::iterator tseq=this->begin(); tseq!=this->end(); tseq++)
	{
		TSequence::iterator extelem=(*tseq)->begin();
		// Enlarge-count
		local_support.clear();
		if((*tseq)->annotations!=0 && 
			!(*tseq)->annotations->empty()) 
		{
			// Scan elems eligible for enlargement (quickly found thanks to annotations!) -- Careful: scan elem only once!
			int last_idx=-1;
			for(vector<EntryPoint>::iterator ann=(*tseq)->annotations->begin(); ann!=(*tseq)->annotations->end();ann++)
				if(ann->entrypoint.idx>last_idx) // Implicitly avoid (-1,0) annotations, that cannot serve for enlargement
				{
					int pos=ann->entrypoint.position;
					if(ann->entrypoint.idx!=0) // First elem doesn't explicitly contain last item of prefix
						pos++;
					TSequence::iterator enlelem=(*tseq)->begin()+ann->entrypoint.idx;
					for(TElement::iterator item=(*enlelem)->begin()+pos; item!=(*enlelem)->end(); item++)
						local_support.insert(*item);
					last_idx=ann->entrypoint.idx;
				};
			for(set<int>::iterator item=local_support.begin(); item!=local_support.end(); item++)
				item_freq_enlarge[*item]++;
			if((*tseq)->annotations->front().entrypoint.idx!=-1)
			// If first elem was used for enlargement, then do not use it for extension
				extelem++;
		};
		// Extend-count
		local_support.clear();
		for(; extelem!=(*tseq)->end(); extelem++)
			for(TElement::iterator item=(*extelem)->begin(); item!=(*extelem)->end(); item++)
				local_support.insert(*item);
		for(set<int>::iterator item=local_support.begin(); item!=local_support.end(); item++)
			item_freq_extend[*item]++;
	}
};

/// Delete unfrequent items from sequence "tseq". "item" is the last inserted item in actual projection
void TProjection::filter_items_from_sequence(TSequence *tseq, int item)
{
	// Filter items and, in parallel, adjust entrypoints (both idx and position might be shifted)
	int elem_shift=0;	// Offset to be added to idx to obtain new value
	int elem_idx=0;			// Index corresponding to telem = actual TElement
	TSequence::iterator  telem =tseq->begin(); // Actual TElement
	vector<EntryPoint>::iterator  entry =tseq->annotations->begin(); // First entrypoint
	vector<EntryPoint>::iterator  entry_end =(tseq)->annotations->end(); // Last entrypoint
	bool first_elem_extendible=(entry!=entry_end && entry->entrypoint.idx<0);
	for( ; entry!=entry_end && entry->entrypoint.idx<0; entry++); // Skip (-1,0) entrypoints (not affected by item shifts)
	// Boolean used later...
	bool first_elem_enlargeable=(entry!=entry_end && entry->entrypoint.idx==0 && entry->entrypoint.position==0);
	for( ; entry!=entry_end && entry->entrypoint.idx==0 && entry->entrypoint.position==0; entry++); // The same for (0,0) entrypoints
	bool del_elem;
	for( ; telem!=(tseq)->end(); elem_idx++, del_elem?telem=(tseq)->erase(telem):telem++)
	{
		del_elem=0;
		int item_shift=0;	// Offset to be added to (possible) entrypoint item positions
		int item_pos=0;	// Item position correponding to item_pt
		bool del_item;
		bool extendible_from_now= (first_elem_extendible || telem!=(tseq)->begin());
		bool enlargeable_from_now= (first_elem_enlargeable && telem==(tseq)->begin());
		TElement::iterator item_pt=(*telem)->begin(); // Actual item in TElement
		// Perform deletions
		// Notice: items pointed by entrypoints cannot be removed now (to avoid messing up pointers...)
		//         -- they will in following extensions/enlargements for different items
		for( ; item_pt!=(*telem)->end(); item_pos++, del_item?item_pt=(*telem)->erase(item_pt):item_pt++)
		{
			del_item=0;
			if(*item_pt==item)  // There could be entrypoints to update
			{
				// Check: if an entrypoint points to an element without "item" there's some bug...
				if(entry!=entry_end && entry->entrypoint.idx < elem_idx) 
					launch_error("Dangling entrypoint reference!!");
				if(entry!=entry_end && entry->entrypoint.idx == elem_idx)
				{
					enlargeable_from_now=true;
					for( ; entry!=entry_end && entry->entrypoint.idx==elem_idx; entry++) // Scan all entrypoints for this occurrence of item
					{
						entry->entrypoint.idx+=elem_shift;
						entry->entrypoint.position+=item_shift;
					};
				};
			} else 
				// NOTICE: "item_freq_extend[x]" always decreases, but "item_freq_enlarge[x]" might also grow!!
				//         At this point we know only OLD frequencies (those owned by the calling projection) 
				//      => only "item_freq_extend" can be used to filter items -- we must keep all enlargeable ones...
				//   TODO: We could compute also simple "item_freq[x]" which is a monotonic upper bound of "item_freq_enl".
				if((!enlargeable_from_now)  &&  (!extendible_from_now || item_freq_extend[*item_pt]<min_sup))						   
			{
				del_item=1;
				item_shift--;
			};
		};
		// If element now is empty => remove it
		if((*telem)->empty()) // All items were removed
		{
			del_elem=1;
			elem_shift--;
		};
	};
};


void TProjection::filter_items()
{
	// Scanning the prefix, retrieve the item entrypoints are referred to (if any)
	int item=-1;
	if(prefix!=0 && prefix->size()>0 && prefix->back().size()>0)
		item=prefix->back().back();

	for(TProjection::iterator tseq=this->begin(); tseq!=this->end(); tseq++)
		this->filter_items_from_sequence(*tseq,item); 
	// NOTICE: in this case, the sequence to filter is owned by the calling projection,
	//         and not by a sub-projection => also "item_freq_enlarge" is reliable, and
	//         could be used... TODO: pass a flag to "filter_items_from_sequence"
};

/// This method expects 1 item per element
void TProjection::load_file_simple_tas(const char* filename)
{
	cout << "******************+++simple tas**********************+\n" ;
	// = initial context is that of 0-size prefix
	this->dump_file_id=0;
	dump_context(0); 

	// TODO: modify to allow loading of segments of data files (just avoid init of counters)
	name_mapping.disable(); // IDs are already int

	ifstream is(filename);
	if(!is)
		launch_error("Cannot load input file!");
	int ni;
	is >> prefix_support;

	mystat.Nseq=prefix_support;

	TSequence *tseq;
	TElement *telem;
	map<int,int> local_support; // # item occurrences in tseq
	map<int,int> first_occ; // Pos. of first occ. of item
	enl_proj_size.clear();
	ext_proj_size.clear();

	// Original Tprojection is always written to disk
	this->dump_write_init(prefix_support,0);
	int dumped_tseq=0;
	for(int i=0; i<prefix_support; i++)
	{
		is >> ni;
		
		cout << "ni: " << ni;

		mystat.avg_seq_len+=(float)ni;
		
		int item;
		float tt=0;

		is >> item;
		// Init tseq
		tseq=checked_new(TSequence);
		tseq->annotations=checked_new(vector<EntryPoint>);
		(*tseq->annotations).push_back(EntryPoint()); // Default entrypoint=(-1,0) with an empty vector of times 

		telem=checked_new(TElement);
		local_support.clear();
		first_occ.clear();
		telem->push_back(item);
		local_support[item]++;
		first_occ[item]=0;
		telem->time=0;
		tseq->push_back(telem);
		for(int j=1; j<ni; j++)
		{
			telem=checked_new(TElement);
			is >> telem->time;
			cout << "telem->time" << telem->time;
			telem->time=tt=(telem->time+tt);
			is >> item;
			cout << "is >> item" << item;
			telem->push_back(item);
			// NOTICE: with non-singleton TElement's, here a "sort(telem->begin(),telem->end());" would be required
			tseq->push_back(telem);
			local_support[item]++;
			if(first_occ.find(item)==first_occ.end())
				first_occ[item]=j;
		};
		// Discard 1-element transactions: they cannot generate annotated sequences
//		if(tseq->size()>1) 
//			this->push_back(tseq);
//		else
//			delete tseq;
		if(tseq->size()<=MAX_SEQUENCE_LEN  &&  tseq->size()>1) 
			// If significant, DUMP sequence to disk --> keep those that can generate 2-element sequences
			// The others are disregarded (can generate 1-elem frequent seqs, but we are not interested in them)
		{
			this->dump_tseq_to_disk(tseq);
			dumped_tseq++;
			// Update item frequencies and their projections'estimated size
			for(map<int,int>::iterator pt=local_support.begin(); pt!=local_support.end(); pt++)
			{
				this->item_freq_extend[pt->first]++;
				int n_annotations=pt->second;
				int n_elem=ni-first_occ[pt->first];
				// Update size of item projection on disk
				ext_proj_size[pt->first]+=
					sizeof(int)*(1+2*n_elem)+sizeof(float)*n_elem +
					sizeof(int)*(1+2*n_annotations)+sizeof(float)*n_annotations;
			};
		};
		checked_delete(tseq);
	};

	mystat.avg_seq_len = mystat.avg_seq_len / mystat.Nseq;

//	this->dump_tseq_to_disk(NULL_tseq_ptr);
	// Add size of init data
	for(map<int,int>::iterator it=ext_proj_size.begin(); it!=ext_proj_size.end(); it++)
		it->second+=2*sizeof(int);

	if(dumped_tseq!=prefix_support)
	{
		TProjection_dump_file_seekp(this->dump_file_start,'A');
		//mywrite(dumped_tseq);
	};
	this->non_empty_sequences=dumped_tseq;
};

/// Fills in a TElement expecting format "time item1 ... itemN -1/-2", and returns last number (-3 if EOF reached, -4 for errors)
int get_telem(ifstream& s, TElement* &tp)
{
	int item=-4;
	string token;
	float tt;
	if(s >> tt)
	{
		tp=checked_new(TElement);
		tp->time=tt;
		while((s >> token) && ( (item=name_mapping.assign_id(token)) >= 0)){
			tp->push_back(item);
		}
	};
	if(item<0)
		return item;
	else 
		return -3;
};

/// General TAS loader
void TProjection::load_file_tas(const char* filename)
{
	// = initial context is that of 0-size prefix
	this->dump_file_id=0;
	dump_context(0); 

	name_mapping.enable();

	ifstream is(filename);
	if(!is)
					
		launch_error("Cannot load input file!");

	//	prefix_support=0;

	int state=-2;  // We assume at least a TSequence exists
	TSequence *tseq;
	TElement *telem;

	int elem_count=0;
	int item_count=0;
	int written_tseq=0;

	map<int,int> local_support;
	map<int,int> first_occ_el, first_occ_item; // Pos. of first occurrence
	// Original Tprojection is always written to disk
	// Notice: now we do not know the number of TSequences... will rewrite it later
	this->dump_write_init(0,0);
	while(state>-3) // Stop if EOF reached or error
	{
		// Allocate default TSequence

		tseq=checked_new(TSequence);
		tseq->annotations=checked_new(vector<EntryPoint>);
		(*tseq->annotations).push_back(EntryPoint()); // Default entrypoint=(0,0) with an empty vector of times 

		local_support.clear();
		first_occ_el.clear();
		first_occ_item.clear();

		// Push elements (if any)
		state=-1;
		while(state==-1)
		{
			state=get_telem(is,telem);
			if(state!=-4)//Normal Case!
			{
				sort(telem->begin(),telem->end());//sorting vector of integer
				tseq->push_back(telem);
				for(TElement::iterator it=telem->begin(); it!=telem->end(); it++, item_count++)
				{
					local_support[*it]++;
					if(first_occ_el.find(*it)==first_occ_el.end())
					{
						first_occ_el[*it]=elem_count;
						first_occ_item[*it]=item_count;
					};
				};
			};
			elem_count++;
		};

		//cout << *tseq;
		// Discard 1-element and empty transactions: they cannot generate annotated sequences
		// The same do for too long sequences: they slow down the computation without use
		int seq_len=(int)tseq->size();
		mystat.avg_seq_len+=(float)seq_len;
		if(seq_len<=MAX_SEQUENCE_LEN  && seq_len>1) // Update countings
		{
			this->dump_tseq_to_disk(tseq);
			written_tseq++;
			for(map<int,int>::iterator pt=local_support.begin(); pt!=local_support.end(); pt++)
			{
				this->item_freq_extend[pt->first]++;
				int n_annotations=pt->second;
				int n_elem=elem_count-first_occ_el[pt->first];
				int n_item=item_count-first_occ_item[pt->first];
				// Update size of item projection on disk
				ext_proj_size[pt->first]+=
					sizeof(int)*(1+n_elem+n_item)+sizeof(float)*n_elem +
					sizeof(int)*(1+2*n_annotations)+sizeof(float)*n_annotations;

				//cerr << "[ESTIMATIONS for " << pt->first << ": ANN->" << n_annotations 
				//	<< " ELEM->" << n_elem << " ITEM->" << n_item << " TOT->"
				//	<< sizeof(int)*(1+n_elem+n_item)+sizeof(float)*n_elem +
				//	sizeof(int)*(1+2*n_annotations)+sizeof(float)*n_annotations << "]\n";
			};
		};
		if(seq_len>0)
			prefix_support++;
		checked_delete(tseq);
	}
	mystat.Nseq=prefix_support;
	mystat.avg_seq_len = mystat.avg_seq_len / mystat.Nseq;

//	this->dump_tseq_to_disk(NULL_tseq_ptr);
	// Add init data to estimated size
	for(map<int,int>::iterator it=ext_proj_size.begin(); it!=ext_proj_size.end(); it++)
		it->second+=2*sizeof(int);
	// Rewrite number of TSequences (now we know it...)
	TProjection_dump_file_seekp(this->dump_file_start,'B');
	//mywrite(written_tseq);
	non_empty_sequences=written_tseq;
};

/// Simplified version of "generate_projected_tsequence" that creates just dummy annotations !!OUTDATED!!
TSequence *TProjection::generate_projected_tsequence_simplified(TProjection::iterator tseq,
										TItemPointer item_pointer,
										projection_type type)
{

	// Projection type is not needed here. "Touch" it to avoid warnings in compilation
	type=type;
	// Step 1: create a copy of tseq's postfix, containing only TElements
	TItemPointer orig_item_pointer = item_pointer;
	TSequence::iterator first_elem=(*tseq)->begin()+item_pointer.idx;
	// Postfix must start right after new_item (which is moved to the prefix)
	item_pointer.increment(tseq);
	// NOTICE: if ( item_pointer.telement == (*tseq)->end() ) => *new_tseq will be an empty vector
	TSequence* new_seq = (*tseq)->duplicate_from(item_pointer);

	// Step 2: SIMPLIFIED
	new_seq->annotations=checked_new(vector<EntryPoint>);
	EntryPoint tmp;	// Default = (-1,0) = last element has been exhausted
	if(item_pointer.position>0)
		tmp.entrypoint=TItemPointer(0,0); // = last element can be enlarged
	new_seq->annotations->push_back(tmp);

	return new_seq;
};

/// Generate a projected TSequence for tseq by adding a new item (located at (elem,offset)) to the prefix
TSequence *TProjection::generate_projected_tsequence(TProjection* new_proj,
										TProjection::iterator tseq,
										TItemPointer item_pointer,
										projection_type type)
{

#ifdef _NO_ANNOTATIONS_
	// Skip annotation computations
	return generate_projected_tsequence_simplified(tseq,item_pointer,type);
#endif

	//cerr << "\nCALLED with " << item_pointer.idx << "," << item_pointer.position << "\n";

	// Step 1: create a copy of tseq's postfix, containing only TElements
	int new_item = item_pointer.get_item(tseq);
	TItemPointer orig_item_pointer = item_pointer;
	TSequence::iterator first_elem=(*tseq)->begin()+item_pointer.idx;
	// Postfix must start right after new_item (which is moved to the prefix)
	item_pointer.increment(tseq);
	// NOTICE: if ( item_pointer.telement == (*tseq)->end() ) => *new_tseq will be an empty vector
	TSequence* new_seq = (*tseq)->duplicate_from(item_pointer);
	new_seq->id = (*tseq)->id;
	// Step 2: (critical point) create new annotations/entrypoints *w.r.t. old TSequence* (it's easier)
	vector<TItemPointer> occ_list;
	vector<TItemPointer>::iterator occ, first_occ;
	vector<EntryPoint>::iterator entry;
	TElement::iterator item;
	/**** changed, we have only extend, not enlargement! ****/
	// -Method for type==EXTEND: to each annotation append each consecutive occurrence of new_item
			// First, collect all occurrences of new_item in tseq, starting from first_elem
	for(TSequence::iterator scan_elem=first_elem; scan_elem!=(*tseq)->end(); scan_elem++)
		if((item=find((*scan_elem)->begin(), (*scan_elem)->end(), new_item)) != (*scan_elem)->end() )
			occ_list.push_back(TItemPointer(tseq,scan_elem,item));
	// Second, extend each entrypoint E with all occ. O, such that "O > E"
	// NOTICE: It works also for E==(-1,0)  -> all occurrence are O>E and so they are kept
	new_seq->annotations=checked_new(vector<EntryPoint>);
	for(first_occ=occ_list.begin(), entry=(*tseq)->annotations->begin();
		first_occ != occ_list.end()  &&  entry != (*tseq)->annotations->end();
		entry++)
	{
		// Skip all unsuitable occurrences
		for(; (first_occ!=occ_list.end()) && (first_occ->idx<=entry->entrypoint.idx); first_occ++); 
		for(occ=first_occ; occ!=occ_list.end(); occ++) // Now, these occurrences are all suitable
		{
			new_seq->annotations->push_back(*entry);	// Copy old annotation
			new_seq->annotations->back().times.push_back((*((*tseq)->begin()+occ->idx))->time); // Append new time
			// Update entrypoint (a TItemPointer value)
			if(*occ==orig_item_pointer && item_pointer.idx>orig_item_pointer.idx) 
				// This occurrence of new_item will become "implicit" => entrypoint:=(-1,0)
				new_seq->annotations->back().entrypoint=TItemPointer();
			else// Otherwise: copy a pointer to the item occurrence
				new_seq->annotations->back().entrypoint=*occ; 
		}
	};
	// Entrypoints are not sorted any more => sort needed
	// OPTIMIZE: rewrite EXTENSION for implicitly ensuring ordering
	sort(new_seq->annotations->begin(), new_seq->annotations->end(), entrypoint_less_than());

	// Step 3: remap annotations *over new TSequence*  -- new_idx = old_idx - item_pointer.idx
	// Notice: item positions are unchanged, except (possibly) for references to the first element of new_seq
	for(entry=new_seq->annotations->begin(); entry!=new_seq->annotations->end(); entry++)
		if(entry->entrypoint.idx >= 0) // case 1: Exhausted first_element [ == (-1,0)] => no change
		{
			if(entry->entrypoint.idx==orig_item_pointer.idx) // case 2: non-exhausted first_elem => set offset to zero
				entry->entrypoint.position=0;
			// case 3: common elements have to be shifted, with the same item offset
			entry->entrypoint.idx=entry->entrypoint.idx-item_pointer.idx;
		};

	// Step 4: Filter out unfrequent items and adjust annotations. 
	// NOTICE: it uses old frequencies -- Indeed, it is called by "this", not "new_proj"
//cerr << "\n---------\nBEFORE\n" << *new_seq << "\n----------\n";
	this->filter_items_from_sequence(new_seq,new_item);
//cerr << "\n---------\nAFTER\n" << *new_seq << "\n----------\n";

	// Step 5: update item frequencies and item estimated_proj_size
	TSequence::iterator extelem=new_seq->begin();
	// *********** Enlarge-count *********** 
	map<int,int> local_support, local_annotations, first_occ_el, first_occ_item;
	// Counters of items cointained in new_seq (strictly) before count_pointer
	int item_count=0;
	TSequence::iterator count_pointer=new_seq->begin();
	if(!new_seq->annotations->empty()) 
	{
		// Scan elems eligible for enlargement (quickly found thanks to annotations!)
		// Careful: each elem could be scanned more than once! (OK for annotations, wrong for local_support)
		int last_idx=-1;
		int first_idx=new_seq->annotations->front().entrypoint.idx;
		for(vector<EntryPoint>::iterator ann=new_seq->annotations->begin(); ann!=new_seq->annotations->end();ann++)
			if(ann->entrypoint.idx>=0) // Avoid (-1,0) annotations, that cannot serve for enlargement
			{
				int pos=ann->entrypoint.position;
				if(first_idx==-1 || ann->entrypoint.idx!=0) // First elem doesn't explicitly contain last item of prefix
					pos++;
				TSequence::iterator enlelem=new_seq->begin()+ann->entrypoint.idx;
				for(TElement::iterator item=(*enlelem)->begin()+pos; item!=(*enlelem)->end(); item++)
				{
					if(ann->entrypoint.idx!=last_idx)  // Avoid multiple scan for the same element - scan only first time
					{
						local_support[*item]++;
						if(first_occ_el.find(*item)==first_occ_el.end()) // First occurrence of *item
						{
							// Update item/elem counting and their associated pointer
							for( ; count_pointer!=new_seq->end() && count_pointer!=enlelem; count_pointer++)
								item_count+=(int)(*count_pointer)->size();
							first_occ_item[*item]=item_count+(int)distance((*enlelem)->begin(),item);
							first_occ_el[*item]=ann->entrypoint.idx;
						};
					};
					local_annotations[*item]++;
				};
				last_idx=ann->entrypoint.idx;
			};
		// Complete item counting
		for( ; count_pointer!=new_seq->end(); count_pointer++)
			item_count+=(int)(*count_pointer)->size();
		for(map<int,int>::iterator item=local_support.begin(); item!=local_support.end(); item++)
		{
			new_proj->item_freq_enlarge[item->first]++;
			int n_annotations=local_annotations[item->first];
			int n_elem=(int)new_seq->size()-first_occ_el[item->first];
			int n_item=item_count-first_occ_item[item->first];
			if(new_proj->enl_proj_size[item->first]==0)
				new_proj->enl_proj_size[item->first]=1000+2*sizeof(int); // The first proj. sequence counts also headings
			new_proj->enl_proj_size[item->first] += 
				(1+n_elem+n_item)*sizeof(int)+n_elem*sizeof(float) +
				(1+2*n_annotations)*sizeof(int)+((int)new_proj->prefix->size()+1)*n_annotations*sizeof(float);
		};
		if(first_idx!=-1)
		// If first elem was used for enlargement, then do not use it for extension
			extelem++;
	};
	// *********** Extend-count *********** 
	local_support.clear();
	local_annotations.clear();
	first_occ_el.clear();
	first_occ_item.clear();
	int elem_pos;
	// If first element is skipped, then init stats in a different way. NOTICE: OK also for new_seq empty
	if(extelem==new_seq->begin()) 
		elem_pos=item_count=0;
	else
	{
		elem_pos=1;
		item_count=(int)new_seq->front()->size();
	};
	int item_pos;
	int annotation_count=0;
	vector<EntryPoint>::iterator next_annot=new_seq->annotations->begin();
	for( ; extelem!=new_seq->end(); extelem++, elem_pos++)
	{
		//cerr << "<"; cerr.flush();
		// First update annotation counter & pointer
		for( ; next_annot!=new_seq->annotations->end() && next_annot->entrypoint.idx<elem_pos; next_annot++,annotation_count++);
		//cerr << "> "; cerr.flush();
		// Then, scan all items in the element and update stats
		item_pos=0;
		for(TElement::iterator item=(*extelem)->begin(); item!=(*extelem)->end(); item++, item_pos++, item_count++)
		{
			local_support[*item]++;
			if(first_occ_el.find(*item)==first_occ_el.end()) // First occurrence of *item
			{
				first_occ_item[*item]=item_count+item_pos;
				first_occ_el[*item]=elem_pos;
			};
			local_annotations[*item] += annotation_count;
		};
	};
	for(map<int,int>::iterator item=local_support.begin(); item!=local_support.end(); item++)
	{
		new_proj->item_freq_extend[item->first]++;
		int n_annotations=local_annotations[item->first];
		int n_elem=(int)new_seq->size()-first_occ_el[item->first];
		int n_item=item_count-first_occ_item[item->first];
		//cout << *new_proj << endl << "Item to add: " << item->first << endl 
		//	 << *new_seq << endl << ": ann=" << n_annotations << ", elem=" << n_elem << ", items=" << n_item << endl;
		if(new_proj->ext_proj_size[item->first]==0)
			new_proj->ext_proj_size[item->first]=10000+2*sizeof(int);
		new_proj->ext_proj_size[item->first] += 
			(1+n_elem+n_item)*sizeof(int)+n_elem*sizeof(float) +
			(1+2*n_annotations)*sizeof(int)+((int)new_proj->prefix->size()+1)*n_annotations*sizeof(float);
	};
	
	return new_seq;
};

/// Generate a projection for each new prefix, deleting non-frequent items (To be done, yet)
vector<TProjection*> *TProjection::generate_projections()
{
	// At least min_sup non-empty transactions are needed, for generating meaningful projections
	if(non_empty_sequences<min_sup)
		return checked_new(vector<TProjection*>);

	map<int,TProjection*> extend; // Projections obtained by EXTENDING prefix
	map<int,TProjection*> enlarge;// Projections obtained by ENLARGING prefix
	// General purpose pointers and iterators
	map<int,TProjection*>::iterator ext_enl_iter;
	TProjection* proj_ptr;
	TSequence* seq_ptr; //???
	set<int> local_extend; // Items already used for EXTENDING prefix in actual tseq
	set<int> local_enlarge;// Items already used for ENLARGING prefix in actual tseq

	// Shortcuts: last_prefix_el   (== 0 for no prefix at all)
	Sequence::iterator last_prefix_el = prefix->end(); /*=((prefix->size()!=0)?(Sequence::iterator)0):(prefix->end()-1));*/
	//****************************************************************************************************//
	//Sequence::iterator last_prefix_el;
	int test_size=((prefix->size()==0)?(0):(1));
	if (test_size){
		last_prefix_el = prefix->end()-1;
	}
	else 
		test_size= 0;
	//****************************************************************************************************//
	
	TProjection dummy_tproj;
	TProjection::iterator tseq;
	int N_to_retrieve;
	/*
	if(disk_in)
	{
		this->clear(); // Do not check if TProj is in memory (it's non-sense, but allowed)
		this->dump_read_init(N_to_retrieve, prefix_len);
		if(prefix_len!=this->prefix->size())
		{
			cerr << "\nREAD prefix_len=" << prefix_len << ", real=" << (int)this->prefix->size() << endl;
			launch_error("Inconsistent prefix_len in \"generate_projections\"!");
		};
		dummy_tproj.push_back(checked_new(TSequence));
		tseq=dummy_tproj.begin();
	} 
	else
	{
	*/
	//};
	N_to_retrieve=(int)this->size();
	tseq=this->begin();
	

	for( ; N_to_retrieve>0; N_to_retrieve-- ) // Foreach seq.
	{
		//cerr << "CYCLE: " << N_to_retrieve << " left.\n";
		/*
		if(disk_in)
		{
			dummy_tproj.front()->clear_all();
			this->retrieve_tseq_from_disk(dummy_tproj.front(),prefix_len);
		}
		*/
		if(!(*tseq)->empty())  // Empty sequences do not contribute with new postfixes => ignore them
		{
			// Various pointers to elements
			int extend_elem_idx=0;
			local_extend.clear();
			local_enlarge.clear();
			// First element ENLARGEMENT
			/*
			if((*tseq)->annotations->begin()->entrypoint.idx != -1)	// First element is for ENLARGE -- without containment check of last_prefix_el
			{
			
				if((*tseq)->annotations->begin()->entrypoint.idx != 0)
				{
					cerr << "IDX == " << (*tseq)->annotations->begin()->entrypoint.idx << " " << (*tseq)->id << endl;
					launch_error("Index of ENLARGMENT element should be 0 !!");
				}
				TSequence::iterator enlarge_first=(*tseq)->begin();
				// Shift start position for "normal" EXTENSION/ENLARGEMENT
				enlarge_elem_idx++;
				extend_elem_idx++;
				// Scan all items in first element for ENLARGEMENT
				for(int offset=static_cast<int>((*enlarge_first)->size()-1); offset>=0; offset--)
				{	
			
					const int new_item = (*enlarge_first)->at(offset);
					if( item_freq_enlarge[new_item]>=min_sup )  // NOTICE: no check for local_enlarge[new_item] needed, here
					{
						local_enlarge.insert(new_item);
						ext_enl_iter=enlarge.find(new_item);
						if(ext_enl_iter == enlarge.end()) // No projection created for new_item, yet => create one
						{
							enlarge[new_item] = proj_ptr =checked_new(TProjection);
							// Compute new prefix by ENLARGING old one
							*proj_ptr->prefix=*this->prefix;
							proj_ptr->dump_file_id=this->dump_file_id+1;
							proj_ptr->prefix->back().push_back(new_item);
							if(disk_out) // Init projection dump
							{
								// Init dump file with an approximated size (adjusted later, if needed)
								proj_ptr->dump_write_init(item_freq_enlarge[new_item],(int)proj_ptr->prefix->size());
								// Manually "allocate" space from dump_file, used later for this projection
								TProjection_dump_size= max(TProjection_dump_size,
									proj_ptr->dump_file_start+this->enl_proj_size[new_item]);
							}
							else
								proj_ptr->reserve(item_freq_enlarge[new_item]);
						} 
						else
							proj_ptr=ext_enl_iter->second;
						// Notice: even if the projection is empty, we need to store it for its annotations...
						// Generate sub-sequence w/ annotations, and updates stats of its projection
						seq_ptr=generate_projected_tsequence(proj_ptr,tseq,TItemPointer(0,offset),ENLARGE);
						proj_ptr->prefix_support++;
						if(seq_ptr->size()>0)
							proj_ptr->non_empty_sequences++;
						if(seq_ptr->annotations->size()==0)
						{
							cerr << **tseq << endl;
							launch_error("Empty annotation extracted (ENL-first)!!");
						};
						// Either put sub-sequence on disk or add it to its projection
						if(disk_out)
						{
							proj_ptr->dump_tseq_to_disk(seq_ptr);
							checked_delete(seq_ptr);
						}
						else
							proj_ptr->push_back(seq_ptr);
					};
				};
			};
			*/
			/*
			// General ENLARGEMENT
			if(test_size!=0  &&  (*tseq)->annotations!=0) // if no prefix at all => no ENLARGE
			{
			
				int prev_idx=-1;
				for(vector<EntryPoint>::iterator annots=(*tseq)->annotations->begin(); annots!=(*tseq)->annotations->end(); annots++)
					if(annots->entrypoint.idx!=prev_idx)
					{
						//cerr << "Using annotation: " << annots->entrypoint.idx << endl;
						prev_idx=enlarge_elem_idx=annots->entrypoint.idx;
						TSequence::iterator enlarge_elem = (*tseq)->begin()+enlarge_elem_idx; 
						if((enlarge_item=(*enlarge_elem)->contains(*last_prefix_el)+1) < (*enlarge_elem)->size()) // Select OK elements
							for(unsigned int offset=enlarge_item; offset<(*enlarge_elem)->size(); offset++) // Scan candidate enl. items
							{
								int new_item = (*enlarge_elem)->at(offset);
								if(local_enlarge.find(new_item)==local_enlarge.end() && 
								item_freq_enlarge[new_item]>=min_sup )  // Select new, frequent items
								{
									// Do as for "first enlargement" case -- but using "enlarge_elem_idx" and "offset" 
									local_enlarge.insert(new_item);
									ext_enl_iter=enlarge.find(new_item);
									if(ext_enl_iter == enlarge.end()) // No projection created for new_item, yet => create one
									{
										enlarge[new_item] = proj_ptr =checked_new(TProjection);
										proj_ptr->reserve(item_freq_enlarge[new_item]);
										// Compute new prefix by ENLARGING old one
										*proj_ptr->prefix=*this->prefix;
										proj_ptr->dump_file_id=this->dump_file_id+1;
										proj_ptr->prefix->back().push_back(new_item);
										if(disk_out) // Init projection dump
										{
											// Init dump file with an approximated size (adjusted later, if needed)
											proj_ptr->dump_write_init(item_freq_enlarge[new_item],(int)proj_ptr->prefix->size());
											// Manually "allocate" space from dump_file, used later for this projection
											TProjection_dump_size= max(TProjection_dump_size,
												proj_ptr->dump_file_start+this->enl_proj_size[new_item]);
										};
									} 
									else
										proj_ptr=ext_enl_iter->second;
									// Notice: even if the projection is empty, we need to store it for its annotations...
									// Generate sub-sequence w/ annotations, and updates stats of its projection
									seq_ptr=generate_projected_tsequence(proj_ptr,tseq,TItemPointer(enlarge_elem_idx,offset),ENLARGE);
									proj_ptr->prefix_support++;
									if(seq_ptr->size()>0)
										proj_ptr->non_empty_sequences++;
									if(seq_ptr->annotations->size()==0)
									{
										// DEBUG MESSAGE
										for(TProjection::iterator itp=this->begin(); itp!=this->end(); itp++)
										{
											bool found=0;
											for(TSequence::iterator itseq=(*itp)->begin(); !found && itseq!=(*itp)->end(); itseq++)
												for(TElement::iterator itel=(*itseq)->begin(); !found && itel!=(*itseq)->end()-1; itel++)
													if(*itel==name_mapping.get_id("1520"))
														found=1;
											if(found)
												cerr << "-------------------------" << **itp << endl;

										};
										cerr << "[" << enlarge_elem_idx << "," << offset << "] = " << name_mapping.get_string(new_item) << endl;
										cerr << *this->prefix << endl;
										cerr << **tseq << endl;
										cerr << *seq_ptr << endl;
										for(TSequence::iterator itseq=(*tseq)->begin(); itseq!=(*tseq)->end(); itseq++)
											for(TElement::iterator itel=(*itseq)->begin(); itel!=(*itseq)->end(); itel++)
												cerr << name_mapping.get_string(*itel) << "->" << item_freq_enlarge[*itel] << "/" << item_freq_extend[*itel] << endl;
										launch_error("Empty annotation extracted (ENL-general)!!");
									};
									// Either put sub-sequence on disk or add it to its projection
									if(disk_out)
									{
										proj_ptr->dump_tseq_to_disk(seq_ptr);
										checked_delete(seq_ptr);
									}
									else
										proj_ptr->push_back(seq_ptr);
								};
							};
					};
			};
			*/
			// EXTENSION
			//cout << **tseq;
			for(TSequence::iterator extend_elem = (*tseq)->begin()+extend_elem_idx; extend_elem!=(*tseq)->end(); extend_elem++, extend_elem_idx++)
				for(unsigned int offset=0; offset<(*extend_elem)->size(); offset++)
				{ 
					int new_item = (*extend_elem)->at(offset);
					if( local_extend.find(new_item)==local_extend.end() && item_freq_extend[new_item]>=min_sup )
					{
						local_extend.insert(new_item);
						ext_enl_iter=extend.find(new_item);
						if(ext_enl_iter == extend.end()) // No projection created for new_item, yet => create one
						{
							extend[new_item] = proj_ptr =checked_new(TProjection);
							proj_ptr->reserve(item_freq_extend[new_item]);
							// Compute new prefix by EXTENDING old one
							*proj_ptr->prefix=*this->prefix;
							proj_ptr->dump_file_id=this->dump_file_id+1;
							Element tmp_el;
							tmp_el.push_back(new_item);
							proj_ptr->prefix->push_back(tmp_el);
							
							/*
							if(disk_out) // Init projection dump
							{
								// Init dump file with an approximated size (adjusted later, if needed)
								proj_ptr->dump_write_init(item_freq_extend[new_item],(int)proj_ptr->prefix->size());
								// Manually "allocate" space from dump_file, used later for this projection
								TProjection_dump_size= max(TProjection_dump_size,
									proj_ptr->dump_file_start+this->ext_proj_size[new_item]);
								//cerr << "[ALLOCATED space for " << proj_ptr->myname() << ": " 
								//	  << proj_ptr->dump_file_start << "-" 
								//	  << proj_ptr->dump_file_start+this->ext_proj_size[new_item] << "]\n"; 
							};
							*/
						}
						else
							proj_ptr=ext_enl_iter->second;
						// Notice: even if the projection is empty, we need to store it for its annotations...
						// Generate sub-sequence w/ annotations, and updates stats of its projection
						seq_ptr=generate_projected_tsequence(proj_ptr,tseq,TItemPointer(extend_elem_idx,offset),EXTEND);
						proj_ptr->prefix_support++;
						if(seq_ptr->size()>0)
							proj_ptr->non_empty_sequences++;
						if(seq_ptr->annotations->empty())
						{
							launch_error("Empty annotation extracted (EXT)!!");
						};
						// Either put sub-sequence on disk or add it to its projection
						/*
						if(disk_out)
						{
							proj_ptr->dump_tseq_to_disk(seq_ptr);
							checked_delete(seq_ptr);
						}
						
						else{*/
						//}
						proj_ptr->push_back(seq_ptr);
						
					};
				};
		};
		//if (!disk_in)
		tseq++;
	};
	//// Now we can delete allocated AUX data structures
	//// ???? Causes runtime errors...
	//for(TProjection::iterator it=dummy_tproj.begin(); it!=dummy_tproj.end(); it++)
	//	checked_delete(*it);

	// Put all projections in a vector, and return it
	vector<TProjection*> *proj_list = checked_new(vector<TProjection*>);
	proj_list->reserve(enlarge.size()+extend.size());
	for(map<int,TProjection*>::iterator ext=extend.begin(); ext!=extend.end(); ext++)
	{
		// Update support of the prefix exploiting previous item_frequency computations
		// NO! Annotation pruning can have decreased real support
		// ext->second->prefix_support=item_freq_extend[ext->first];
		proj_list->push_back(ext->second);
		/*
		if(disk_out)
		{
//			ext->second->dump_tseq_to_disk(NULL_tseq_ptr); // Dump sequences left in the buffer
			if(ext->second->prefix_support!=item_freq_extend[ext->first])
			{
				if(ext->second->dump_file_start==-1)
				{
					cerr << "REPORT: " << ext->second << " SIZE:" << (int)ext->second->size() << endl;
					cerr << "PREF: " << *ext->second->prefix << " from " << *this->prefix << endl;
				};
				dump_context(ext->second->dump_file_id);
				TProjection_dump_file_seekp(ext->second->dump_file_start,'C');
				//mywrite(ext->second->prefix_support);
			};
		};
		*/
	};
	for(map<int,TProjection*>::iterator enl=enlarge.begin(); enl!=enlarge.end(); enl++)
	{
		// Update support of the prefix exploiting previous item_frequency computations
		// NO! Annotation pruning can have decreased real support
		// enl->second->prefix_support=item_freq_enlarge[enl->first];
		proj_list->push_back(enl->second);
		/*
		if(disk_out)
		{
//			enl->second->dump_tseq_to_disk(NULL_tseq_ptr); // Dump sequences left in the buffer
			if(disk_out && enl->second->prefix_support!=item_freq_extend[enl->first])
			{
				if(enl->second->dump_file_start==-1)
				{
					cerr << "REPORT: " << enl->second << " SIZE:" << (int)enl->second->size() << endl;
					cerr << "PREF: " << *enl->second->prefix << " from " << *this->prefix << endl;
				};
				dump_context(enl->second->dump_file_id);
				TProjection_dump_file_seekp(enl->second->dump_file_start,'D');
				//mywrite(enl->second->prefix_support);
			};
		};
		*/
	};

	//// Computes item frequencies for all projections
	//for(vector<TProjection*>::iterator pr=proj_list->begin(); pr!=proj_list->end(); pr++)
	//	(*pr)->set_item_freq();

	//// Check consistency of space allocations on disk
	/*
	if(disk_out)
	{
		for(map<int,TProjection*>::iterator ext=extend.begin(); ext!=extend.end(); ext++)
			if(ext->second->dump_file_size > this->ext_proj_size[ext->first])
			{
				//ext->second->retrieve_from_disk();
				cerr << *this->prefix << " --> " << *ext->second->prefix << endl;
				cerr << ext->second->dump_file_size << " > " << this->ext_proj_size[ext->first] << endl;
				launch_error("Under-estimated disk space for extend-projected sequence!!");
			};
		for(map<int,TProjection*>::iterator enl=enlarge.begin(); enl!=enlarge.end(); enl++)
			if(enl->second->dump_file_size > this->enl_proj_size[enl->first])
			{
				cerr << endl << *enl->second << endl;
				launch_error("Under-estimated disk space for enlarge-projected sequence!!");
			};
	};
	*/
	return proj_list;
};

/// Delete useless annotations, i.e., those that do not contribute to dense regions
void TProjection::filter_annotations(H_store *O, float tau)
{
	H tmp_h;
	bool tmp_bool;
	TItemPointer tmp_TIP;
	TProjection::iterator tmp_tseq_pt;
	vector<EntryPoint>::iterator erase_from, erase_to;
	vector<EntryPoint> *my_annotations;

	// This function could be used for removing only IDX=-2 
	bool use_only_for_idx_minus_2 = (this->empty() || 
									(*this->begin())->annotations->empty() || 
									((*this->begin())->annotations->front().times.size()<2));

	// Repeat for each transaction in the projection
	for(TProjection::iterator tseq=this->begin(); tseq!=this->end();) // tseq is incremented manually below...
	{
		if ((*tseq)->annotations->empty())
		{
			cerr << "=================================\n" << *this << "\n=================================\n";
			launch_error("Trying filtering a sequence without annotations!!!");
		};
		// Scan all annotations, and, if necessary, delete them -- care needed for first annotation!!!
		my_annotations=(*tseq)->annotations;
 		tmp_TIP=my_annotations->front().entrypoint;
		erase_from=erase_to=my_annotations->begin(); // Used to delete annotations in batch -> [from,to) interval
		for(vector<EntryPoint>::iterator ann=my_annotations->begin(); ann!=my_annotations->end(); ann++)
		{
			tmp_bool=true; // == "a dense region touched by tmp_h has still to be found"
			if ((*ann).entrypoint.idx != -2)
			{
				if(use_only_for_idx_minus_2)
				{
					tmp_bool=false;				
				}
				else
				{
					// Build the H corresponding to actual annotation
					tmp_h.low.clear();
					tmp_h.up.clear();
					float base=ann->times.front();
					for(vector<float>::iterator tt=ann->times.begin()+1; tt!=ann->times.end(); tt++)
					{
						float time_gap=*tt-base;
						tmp_h.low.push_back(time_gap>tau?(time_gap-tau):0);
						tmp_h.up.push_back(*tt-base+tau);
						base=*tt;
					};
					// Check density of overlapping regions
					set<int> *intersect=O->intersect_list(tmp_h,tau);
					for(set<int>::iterator it=intersect->begin(); tmp_bool && (it!=intersect->end()); it++){
						tmp_bool= (O->rectangles[*it]->density < min_sup);
					}
				}
			}
			
			if(tmp_bool){ // No dense region found => add annotation to the list of will-be-removed's
				erase_to++;
			}
			else 
			{	// Dense region found: element survives, but I have to delete annotations [from,to), if any
				if(erase_from!=erase_to) // Erasure! "ann" is updated
					ann=(my_annotations->erase(erase_from,erase_to)); // = new position of actual annotation
				erase_from=erase_to=ann+1; // Reset erasure interval
			};
		};
		if(erase_from!=erase_to) {// Erasure left behind...
			(my_annotations->erase(erase_from,erase_to));
		}// First check whether no annotation are left => remove all *tseq
		if( my_annotations->empty() )
		{
			// "Clever" erasure of (*tseq): switch it with last element, then delete last element --> avoids vector shifts		
			//ATTENTION: "Clever" erasure dismissed for problems with pointers (tseq points an erased location)
			//tmp_tseq_pt=this->end()-1;
			//if(tseq!=tmp_tseq_pt)
			//	my_swap<TSequence*>(*tseq,*tmp_tseq_pt);
			//checked_delete(*tmp_tseq_pt);
			//this->pop_back();
			tseq = this->erase(tseq);
			// ... and update non-empty sequences count
			this->non_empty_sequences--;
		} 
		else // Otherwise, if first annotation was erased, cut *tseq from first survived annotation and adjust everything
		{
			if(my_annotations->begin()->entrypoint != tmp_TIP) 
				// The first annotations (that followed a special semantics) were erased 
				// => the next one becomes new "first"
				// => it has to be adjusted to follow the special semantics (= entrypoint points to *next* candidate item)
			{
				TItemPointer first_tip = my_annotations->front().entrypoint; // Copy entrypoint
				TItemPointer next_tip = first_tip;
				next_tip.increment(tseq);

				// Erase all elements before new "first" element

				(*tseq)->erase((*tseq)->begin(), (*tseq)->begin()+next_tip.idx);

				// Adjust first entrypoint (and all successive ones that pointed to the same point)
				TItemPointer new_first_tip = TItemPointer(0,0);
				if(next_tip.idx>first_tip.idx) // Element overflow => (-1,0)
					new_first_tip.idx=-1;
				else {// Otherwise: erase items in the element before next_tip.position
					(*tseq)->front()->erase( (*tseq)->front()->begin() , (*tseq)->front()->begin()+next_tip.position 
);
				}
				vector<EntryPoint>::iterator ann=my_annotations->begin();
				for( ; (ann!=my_annotations->end()) && (ann->entrypoint == first_tip);  ann++)
					ann->entrypoint=new_first_tip;

				// Adjust all remaining entrypoints
				for( ; ann!=my_annotations->end();  ann++)
					ann->entrypoint=new_first_tip;
			};
			// Finally, tseq is manually incremented...
			tseq++;
		};
	};
};

/// Split "annot_vector" into ANNOTATION_PARTITIONS segments along dimension "best_idx"
vector<vector<H>*> split_rectangle_set(vector<H>& annot_vector, int best_idx, float tau)
{
	// Equal-frequency partitioning
	// a. collect and sort low[] values
	vector<pair<int,float> > couples;
	couples.reserve(annot_vector.size());
	int tmp_i=0;
	for(vector<H>::iterator it=annot_vector.begin(); it!=annot_vector.end(); it++)
		couples.push_back(make_pair(tmp_i++,it->low[best_idx]));
	sort(couples.begin(), couples.end(), pair_second_less_than());
	// b. compute division points
	vector<vector<pair<int,float> >::iterator> starts;
	starts.push_back(couples.begin());
	vector<pair<int,float> >::iterator actual_pos=couples.begin();
	vector<pair<int,float> >::iterator tau_pos, step_pos, tmp_pos;
	int partitions_left=ANNOTATION_PARTITIONS;

	while(actual_pos!=couples.end())
	{
		int r_left=(int)distance(actual_pos,couples.end());
		int width=min( max(r_left/partitions_left , 10), r_left);  // Enforce at least 10 rects per segment
		step_pos=actual_pos+width;  // Notice: it never exceeds couples.end()
		tau_pos=lower_bound(actual_pos, couples.end(), 
						    make_pair(-1,actual_pos->second+(float)2.001*tau), // Dummy pair
							pair_second_less_than());
		actual_pos=max(tau_pos,step_pos);
		starts.push_back(actual_pos); // Store actual_pos. Notice: at last iteration actual_pos==couples.end()
		partitions_left--;
	};
	// At this point "starts" contains all starting points (at most ANNOTATION_PARTITIONS) + a "cap" (couples.end())
	// For the moment proceeds as for equal-size partitining, i.e., build "annot_div". We can do better...
	vector<vector<H>* > annot_div;
	int n_partitions=(int)starts.size()-1;
	for(int i=0; i<n_partitions; i++)
		annot_div.push_back(checked_new(vector<H>));

	H* h_ptr, h_ptr_next;
	for(int i=0; i<n_partitions; i++)
		for(vector<pair<int,float> >::iterator it=starts[i]; it!=starts[i+1]; it++)
		{
			h_ptr=&annot_vector[it->first];
			annot_div[i]->push_back(*h_ptr);
			if((i+1)<n_partitions && h_ptr->up[best_idx]>=starts[i+1]->second) // Box crosses two partitions
			{
				// Adjust occurrence in first partition
				annot_div[i]->back().up[best_idx]=starts[i+1]->second;
				// Add & adjust occurrence in adjacent partition
				annot_div[i+1]->push_back(*h_ptr);
				annot_div[i+1]->back().low[best_idx]=starts[i+1]->second;
			};
		};
	return annot_div;
};

/// Extract a set of rectangles representing frequent annotations
H_store *TProjection::extract_dense_rectangles(float tau)
{
	if(this->prefix==0 || this->prefix->size()<2) // Sequences smaller than 2 elems cannot have transition times
		launch_error("Cannot extract frequent annotations on sequences with less than 2 elements!!");

	int n=(int)(this->prefix->size()-1);
	if(n<1)
		launch_error("Function \"extract_dense_rectangles\" invoked with |prefix|<2 !!");
	// At the present it is the caller that chooses the indexing strategy
	H_store *local_store = checked_new(H_store(n));

	vector<H> tmp_vector; 
	vector<H> annot_vector;
	annot_vector.reserve(this->size());  // TODO: Find a better estimate for number of annotations
	H tmp_h;

	// Step 1: build the rectangles for each sequence (they all have frequency set to 1) and collect them
	for(TProjection::iterator seq=this->begin(); seq!=this->end(); seq++)
	{
		// First compute the vectors of transition times
		for(vector<EntryPoint>::iterator entry=(*seq)->annotations->begin(); entry!=(*seq)->annotations->end(); entry++)
		{
			tmp_h.low.clear();
			tmp_h.up.clear();
			float base=entry->times.front();
			for(vector<float>::iterator tt=entry->times.begin()+1; tt!=entry->times.end(); tt++)
			{
				float time_gap=*tt-base;
				tmp_h.low.push_back(APPROX_FLOAT(time_gap>tau?(time_gap-tau):0));
				tmp_h.up.push_back(APPROX_FLOAT(*tt-base+tau));
				base=*tt;
			};
			tmp_vector.push_back(tmp_h);
		};
		// Combine transition times for the same transaction
		combine_rectangle_set(tmp_vector,local_store,false,tau);
		// Copy results from previous step to a vector --> BAD: Optimize to avoid multiple copies!
		for(vector<H*>::iterator it=local_store->rectangles.begin(); it!=local_store->rectangles.end(); it++)
			if(*it!=0)
				annot_vector.push_back(**it);
		local_store->clear();
		tmp_vector.clear();
	};
	checked_delete(local_store);

	// Step 2: divide annotations in ANNOTATION_PARTITIONS^n groups -- min size = 2tau
	vector<vector<H>* > annot_div[2];
	vector<vector<H>* > tmp_div;
	int last=0;
	annot_div[last]=split_rectangle_set(annot_vector,0,tau);
	for(int i=1; i<n; i++)
	{
		// Split vectors in "last" and put results into "1-last"
		for(vector<vector<H>* >::iterator it=annot_div[last].begin(); it!=annot_div[last].end(); it++)
		{
			tmp_div=split_rectangle_set(**it,i,tau);
			for(vector<vector<H>* >::iterator it2=tmp_div.begin(); it2!=tmp_div.end(); it2++)
				annot_div[1-last].push_back(*it2);
			checked_delete(*it); // Once splitted, the rectangle set is not used anymore
		};
		annot_div[last].clear();
		last=1-last;
	};

	// Debug
	//cout << (int)annot_vector.size() << " -> " << (int)annot_div[last].at(0)->size();
	//int mycounter=(int)annot_div[last].at(0)->size();
	//int n_partitions=(int)annot_div[last].size();
	//for(int i=1; i<n_partitions; i++)
	//{
	//	cout << " + " << (int)annot_div[last].at(i)->size();
	//	mycounter+=(int)annot_div[last].at(i)->size();
	//};
	//cout << " = " << mycounter << endl;
	// END Debug

	// Step 3: add rectangles of each partition to a different storage, summing up frequencies
	H_store *working = checked_new(H_store(n));
	H_store *local_working = checked_new(H_store(n));
	for(vector<vector<H>* >::iterator segment=annot_div[last].begin(); segment!=annot_div[last].end(); segment++)
	{
		combine_rectangle_set(**segment,local_working,true,tau);
		checked_delete(*segment); // Once put into the (local) working, it's no use anymore
		for(vector<H*>::iterator it=local_working->rectangles.begin(); it!=local_working->rectangles.end(); it++)
			if(*it!=0) // move H from local_ to working
			{
				working->insert_pt(*it);
				*it=0;
			};
		local_working->fast_clear(); // Rectangles have already been migrated...
	};
	checked_delete(local_working);

	return working;
};

class compare_H_on_ith_dimension
{ 
public:
	int dim;
	vector<H> *H_list;
	bool operator ()(const int& a, const int& b) 
	{return (H_list->operator[](a).low[dim] < H_list->operator[](b).low[dim]); };
};

compare_H_on_ith_dimension H_compare;

void brute_force_density_computation(vector<H>& annot_vector, vector<int>& data_points, 
									 vector<float>& low, vector<float>& up, int i,
									 H_store *working)
{
	// Split along dimension i-1
	i--;
	H_compare.dim=i;
	H_compare.H_list=&annot_vector;
	// Sort Hs
	sort(data_points.begin(), data_points.end(), H_compare); 
	// Build bands
	vector<float> bounds;
	vector<vector<int> > bands;
	for(vector<H>::iterator it=annot_vector.begin(); it!=annot_vector.end(); it++)
	{
		bounds.push_back(it->low[i]);
		bounds.push_back(it->up[i]);
	};
	sort(bounds.begin(),bounds.end());
	bounds.erase(unique(bounds.begin(),bounds.end()), bounds.end());
	bands.resize((int)bounds.size()-1);
	int left_band=0;
	for(int h=0; h<(int)data_points.size(); h++) // For each H, see which bands it crosses
	{
		float ll=annot_vector[data_points[h]].low[i];
		float uu=annot_vector[data_points[h]].up[i];
		while(bounds[left_band]< ll)
			left_band++;
		for(int my_band=left_band; bounds[my_band]<uu; my_band++)
			bands[my_band].push_back(data_points[h]);
	};

	if(i>0)
	{
		// Recursively apply to all (dense) bands
		for(int bb=0; bb<(int)bands.size(); bb++)
		{
			if((int)bands[bb].size() >= min_sup)
			{
				low[i]=bounds[bb];
				up[i]=bounds[bb+1];
				brute_force_density_computation(annot_vector,bands[bb],low,up,i,working);
			};
		};
	}
	else
	{
		// Create an H from each dense bands, and add it to "working"
		for(int bb=0; bb<(int)bands.size(); bb++)
		{
			if((int)bands[bb].size() >= min_sup)
			{
				low[i]=bounds[bb];
				up[i]=bounds[bb+1];
				working->insert_pt(checked_new(H(low,up,(int)bands[bb].size())));
			};
		};
	};
};

/// Extract a set of rectangles representing frequent annotations
H_store *TProjection::extract_dense_rectangles_brute_force(float tau)
{
	if(this->prefix==0 || this->prefix->size()<2) // Sequences smaller than 2 elems cannot have transition times
		launch_error("Cannot extract frequent annotations on sequences with less than 2 elements!!");

	int n=(int)(this->prefix->size()-1);
	if(n<1)
		launch_error("Function \"extract_dense_rectangles_new\" invoked with |prefix|<2 !!");

	// At the present it is the caller that chooses the indexing strategy
	H_store *local_store = checked_new(H_store(n));

	vector<H> tmp_vector; 
	vector<H> annot_vector;
	annot_vector.reserve(this->size());  // TODO: Find a better estimate for number of annotations
	H tmp_h;

	// Step 1: build the rectangles for each sequence (they all have frequency set to 1) and collect them
	for(TProjection::iterator seq=this->begin(); seq!=this->end(); seq++)
	{
		// First compute the vectors of transition times
		for(vector<EntryPoint>::iterator entry=(*seq)->annotations->begin(); entry!=(*seq)->annotations->end(); entry++)
		{
			tmp_h.low.clear();
			tmp_h.up.clear();
			float base=entry->times.front();
			for(vector<float>::iterator tt=entry->times.begin()+1; tt!=entry->times.end(); tt++)
			{
				float time_gap=*tt-base;
				tmp_h.low.push_back(APPROX_FLOAT(time_gap>tau?(time_gap-tau):0));
				tmp_h.up.push_back(APPROX_FLOAT(*tt-base+tau));
				base=*tt;
			};
			tmp_vector.push_back(tmp_h);
		};
		// Combine transition times for the same transaction
		combine_rectangle_set(tmp_vector,local_store,false,tau);
		// Copy results from previous step to a vector --> BAD: Optimize to avoid multiple copies!
		for(vector<H*>::iterator it=local_store->rectangles.begin(); it!=local_store->rectangles.end(); it++)
			if(*it!=0)
				annot_vector.push_back(**it);
		local_store->clear();
		tmp_vector.clear();
	};
	checked_delete(local_store);

	// Step 2
	vector<int> data_points;
	vector<float> low(n), up(n);
	data_points.reserve((int)annot_vector.size());
	for(int i=0; i<(int)annot_vector.size(); i++)
		data_points.push_back(i);
	H_store *working = checked_new(H_store(n));
	brute_force_density_computation(annot_vector, data_points, low, up, n, working);

	return working;
};

void brute_force_density_computation2(vector<H>& annot_vector, vector<int>& data_points, 
									 vector<vector<float> >& all_bounds,
									 vector<int>& idx, int i,
									 set<vector<int> >& working)
{
	// Split along dimension i-1
	i--;
	H_compare.dim=i;
	H_compare.H_list=&annot_vector;
	// Sort Hs
	sort(data_points.begin(), data_points.end(), H_compare); 
	// Build bands
	vector<vector<int> > bands;
	bands.resize((int)all_bounds[i].size()-1);
	int left_band=0;
	for(int h=0; h<(int)data_points.size(); h++) // For each H, see which bands it crosses
	{
		float ll=annot_vector[data_points[h]].low[i];
		float uu=annot_vector[data_points[h]].up[i];
		while(all_bounds[i][left_band]< ll)
			left_band++;
		for(int my_band=left_band; all_bounds[i][my_band]<uu; my_band++)
			bands[my_band].push_back(data_points[h]);
	};

	if(i>0)
	{
		// Recursively apply to all (dense) bands
		for(int bb=0; bb<(int)bands.size(); bb++)
		{
			if((int)bands[bb].size() >= min_sup)
			{
				idx[i]=bb;
				brute_force_density_computation2(annot_vector,bands[bb],all_bounds,idx,i,working);
			};
		};
	}
	else
	{
		// Create an H from each dense bands, and add it to "working"
		for(int bb=0; bb<(int)bands.size(); bb++)
		{
			if((int)bands[bb].size() >= min_sup)
			{
				idx[i]=bb;
				working.insert(idx);
			};
		};
	};
};

bool recursive_check_coverage(set<vector<int> >& dense, vector<int>& myfirst, vector<int>& mylast,
							  vector<int>& actual, int idx)
{
	if(idx==0)
		return ( dense.find(actual)!=dense.end() );
	else
	{
		bool flag=true;
		idx--;
		for(int i=myfirst[idx]; flag && i<=mylast[idx]; i++)
		{
			actual[idx]=i;
			flag = recursive_check_coverage(dense,myfirst,mylast,actual,idx);
		};
		return flag;
	};
};

float check_coverage(set<vector<int> >& dense,vector<int>& first, vector<int>& last, 
					int i, int direction, vector<vector<float> >& all_bounds) 
{
	vector<int> myfirst(first);
	vector<int> mylast(last);
	vector<int> tmp((int)first.size());
	if(direction>0)
		myfirst[i]=mylast[i]=last[i]+1;
	else
		myfirst[i]=mylast[i]=first[i]-1;
	float vol=0;
	// Check out-of-range
	if(myfirst[i]<0 || myfirst[i]>=((int)all_bounds[0].size()-1))
		return 0;
	if(recursive_check_coverage(dense,myfirst,mylast,tmp,(int)myfirst.size()))
	{
		// Compute volume
		vol=1;
		for(int i=0; i<(int)first.size(); i++)
			vol=vol*(all_bounds[i][mylast[i]+1]-all_bounds[i][myfirst[i]]);
	};
	return vol;
};

void recursive_delete_coverage(set<vector<int> >& dense,vector<int>& first, vector<int>& last, vector<int>& actual, int idx)
{
	//cerr << "recursive_delete_coverage(...," << first.front() << "," << last.front() << "," 
	//	<< actual.front() << "," << idx << ")\n"; 
	if(idx==0)
		dense.erase(actual);
	else
	{
		idx--;
		for(int i=first[idx]; i<=last[idx]; i++)
		{
			actual[idx]=i;
			recursive_delete_coverage(dense,first,last,actual,idx);
		};	
	};
};

void delete_coverage(set<vector<int> >& dense,vector<int>& first, vector<int>& last) 
{
	//cerr << "delete_coverage(...," << first.front() << "," << last.front() << ")\n"; 
	vector<int> actual((int)first.size());
	recursive_delete_coverage(dense,first,last,actual,(int)first.size());
};

//void print_bounds()
//void print_dense(set<vector<int> >& dense, vector<vector<float> >& all_bounds)
//{
//	cerr << "--------\n";
//	int i=0;
//	cerr << "Bounds:\n";
//	for(vector<vector<float> >::iterator it=all_bounds.begin(); it!=all_bounds.end(); it++)
//	{ 
//		for(vector<float>::iterator it2=it->begin(); it2!=it->end(); it2++)
//			cerr << " " << *it2;
//		cerr << endl;
//	};
//	for(set<vector<int> >::iterator it=dense.begin(); it!=dense.end(); it++, i++)
//	{
//		cerr << i << ". ";
//		for(vector<int>::iterator it2=(*it).begin(); it2!=it->end(); it2++)
//			for(vector<vector<float> >::iterator bb=all_bounds.begin(); bb!=all_bounds.end(); bb++)
//			{
//				cerr << *it2; 
//				cerr << " (" << bb->operator [](*it2) << "-";
//				cerr << bb->operator [](*it2+1) << ") ";
//			};
//		cerr << endl;
//	};
//	cerr << "--------\n";
//};

void coalesce_density_blocks(set<vector<int> >& dense, 
							 vector<vector<float> >& all_bounds, 
							 int n, H_store *working)
{
	vector<int> start, first, last, tmp;
	set<vector<int> >::iterator it_first, it_last, it_tmp;
	while(!dense.empty())
	{
		//cerr << "MAIN CYCLE " << debug_c++ << ": " << (int)dense.size() << "\n";
		//print_dense(dense,all_bounds);

		// 1. Choose a random block
		start.clear();
		for(int i=0; i<n; i++)
		{
			// Given the fixed components in "start", find min and max of next component
			tmp=first=last=start;
			for(int j=i; j<n; j++)
			{
				first.push_back(0);
				last.push_back((int)all_bounds[j].size());
			};
			it_first=dense.lower_bound(first);
			it_last=dense.lower_bound(last);
			it_last--; // Trick...
			// Now, choose a value within the range found
			tmp.push_back(it_first->operator [](i)+ rand()%(1 + it_last->operator [](i) - it_first->operator [](i)));		
			// Find the first exact value for this component
			for(int j=i+1; j<n; j++)
				tmp.push_back(0);
			it_tmp=dense.lower_bound(tmp);
			if(it_tmp==dense.end())
				launch_error("Could not find random block while coalescing densities!");
			start.push_back(it_tmp->operator [](i));
		};

		//cerr << "Random block: " << start.front() << endl;

		// 2. Try to expand it
		first=last=start; // first=lower bounds, last=upper bounds
		float best_volume=1; // Trick to avoid performing coalescing
		float volume;
		int best_idx, best_direction;
		//int debug_c2=0;
		while(!dense.empty() && best_volume>0)
		{
			//cerr << "EXPANSION " << debug_c2++ << ": " << best_volume << "\n";
			best_volume=0;
			// Check 1-step expansion along all directions
			for(int i=0; i<n; i++)
			{
				for(int direction=-1; direction<2; direction+=2)
				{
					// i-th positive direction
					volume=check_coverage(dense,first,last,i,direction,all_bounds);
					//cerr << "DIR: " << direction << " -> VOL: " << volume;
					if(volume>best_volume)
					{
						best_volume=volume;
						best_idx=i;
						best_direction=direction;
					};
				};
			};
			if(best_volume>0)
			{
				// Expand bounds
				if(best_direction>0)
					last[best_idx]++;
				else
					first[best_idx]--;
			};
		};

		// 3. When expansion is maximized, push it in "working" and delete it from "dense"
		H* tmp_h_pt=new H;
		tmp_h_pt->density=min_sup;
		for(int i=0; i<n; i++)
		{
			tmp_h_pt->low.push_back(all_bounds[i][first[i]]);
			tmp_h_pt->up.push_back(all_bounds[i][last[i]+1]);
		};
		working->insert_pt(tmp_h_pt);
		delete_coverage(dense,first,last);
	};
};

/// Extract a set of rectangles representing frequent annotations
H_store *TProjection::extract_dense_rectangles_brute_force2(float tau)
{

	if(this->prefix==0 || this->prefix->size()<2) // Sequences smaller than 2 elems cannot have transition times
		launch_error("Cannot extract frequent annotations on sequences with less than 2 elements!!");

	int n=(int)(this->prefix->size()-1);
	if(n<1)
		launch_error("Function \"extract_dense_rectangles_new\" invoked with |prefix|<2 !!");

	mystat.hits[n]++;

	// At the present it is the caller that chooses the indexing strategy
	H_store *local_store = checked_new(H_store(n));

	vector<H> tmp_vector; 
	vector<H> annot_vector;
	annot_vector.reserve(this->size());  // TODO: Find a better estimate for number of annotations
	H tmp_h;

	// Step 1: build the rectangles for each sequence (they all have frequency set to 1) and collect them
	for(TProjection::iterator seq=this->begin(); seq!=this->end(); seq++)
	{
		//cout << "LUNGHEZZA SEQUENZA DENTRO LA SEQUENZA: " << (*seq)->size() << endl;
		// First compute the vectors of transition times
		for(vector<EntryPoint>::iterator entry=(*seq)->annotations->begin(); entry!=(*seq)->annotations->end(); entry++)
		{
			tmp_h.low.clear();
			tmp_h.up.clear();
			float base=entry->times.front();
			for(vector<float>::iterator tt=entry->times.begin()+1; tt!=entry->times.end(); tt++)
			{
				float time_gap=*tt-base;
				tmp_h.low.push_back(APPROX_FLOAT(time_gap>tau?(time_gap-tau):0));
				tmp_h.up.push_back(APPROX_FLOAT(*tt-base+tau));
				base=*tt;
			};
			tmp_vector.push_back(tmp_h);
		};
		// Combine transition times for the same transaction
		combine_rectangle_set(tmp_vector,local_store,false,tau);
		// Copy results from previous step to a vector --> BAD: Optimize to avoid multiple copies!
		for(vector<H*>::iterator it=local_store->rectangles.begin(); it!=local_store->rectangles.end(); it++)
			if(*it!=0)
				annot_vector.push_back(**it);
		local_store->clear();
		tmp_vector.clear();
	};
	checked_delete(local_store);

	local_clock.elapse();

	// Step 2
	vector<int> data_points;
	data_points.reserve((int)annot_vector.size());
	for(int i=0; i<(int)annot_vector.size(); i++)
		data_points.push_back(i);
	// Build all bands once
	vector<vector<float> > all_bounds(n);
	for(int i=0; i<n; i++)
	{
		for(vector<H>::iterator it=annot_vector.begin(); it!=annot_vector.end(); it++)
		{
			all_bounds[i].push_back(it->low[i]);
			all_bounds[i].push_back(it->up[i]);
		};
		sort(all_bounds[i].begin(),all_bounds[i].end());
		all_bounds[i].erase(unique(all_bounds[i].begin(),all_bounds[i].end()), all_bounds[i].end());
	};

	// Extract small dense regions
	vector<int> idx(n);
	set<vector<int> > dense;
	brute_force_density_computation2(annot_vector, data_points, all_bounds, idx, n, dense);

	mystat.times_density[n]+=local_clock.elapse();
	mystat.blocks_before[n]+=(int)annot_vector.size();

	//cerr << (int)annot_vector.size() << " blocks computed for " << this->myname() << ". Now coalescing...\n";

	// Simplify density structure -- loosing precise density info
	H_store *working = checked_new(H_store(n));
	
	//// DEBUG: simply copy dense into working
	//for(set<vector<int> >::iterator it=dense.begin(); it!=dense.end(); it++)
	//{
	//	tmp_h.density=min_sup;
	//	tmp_h.low.clear();
	//	tmp_h.up.clear();
	//	for(int i=0; i<n; i++)
	//	{
	//		tmp_h.low.push_back(all_bounds[i][it->at(i)]);
	//		tmp_h.up.push_back(all_bounds[i][it->at(i)+1]);
	//	};
	//	working->insert(tmp_h);
	//};
	//return working;
	coalesce_density_blocks(dense,all_bounds,n,working);

	mystat.times_coalesce[n]+=local_clock.elapse();
	mystat.blocks_after[n]+=(int)working->rectangles.size();
	//cerr << "Coalescing finished!\n";
	return working;
};

void TProjection::evaluate_size(int& nseqs, int& nelems, int& nitems, int& nannots)
{
	nelems=nitems=nannots=0;
	nseqs=(int)this->size();
	for(TProjection::iterator seq=this->begin(); seq!=this->end(); seq++)
	{
		nelems+=(int)(*seq)->size();
		if((*seq)->annotations != 0)
			nannots+=(int)(*seq)->annotations->size();
		for(TSequence::iterator elem=(*seq)->begin(); elem!=(*seq)->end(); elem++)
			nitems+=(int)(*elem)->size();
	};
};

string TProjection::myname() 
{
	ostringstream name;
	if(this->prefix==0 || this->prefix->empty())
		name << "root";
	else
	{
		for(Sequence::iterator el=this->prefix->begin(); el!=this->prefix->end(); el++)
		{
			name << "__" << el->front();
			for(Element::iterator it=el->begin()+1; it!=el->end(); it++)
				name << "_" << (*it);
		};
	};
	return name.str();
};

/// Useful template for writing binary data
template<class T> int mywrite(T x) { TProjection_dump_file->write((char *)&x,sizeof(T)); return sizeof(T); };
//template<class T> int mywrite(T x) { 
//	int pp=(int)TProjection_dump_file.tellp(); 
//	TProjection_dump_file.write((char *)&x,sizeof(T));  
//	cerr << "write " << x << " @ " << pp << "\n";
//	return sizeof(T);
//};


void TProjection::dump_tseq_to_disk(TSequence *tseq)
{
	//            __________elem1______________     _____________elemN___________ 
	// Format = N Time1 n1 Item1_1 ... Item1_n1 ... TimeN nN ItemN_1 ... ItemN_nN 
	//        + Na Time1_1 ... Time1_D IDX1 POS1 ... TimeNa_1 ... TimeNa_D IDXNa POSNa   
	//             \______times______/ \_entry_/     \________times______/ \__entry__/
	//
	// Move to my position

//cerr << "[POS of " << this->myname() << ": actual->" << this->dump_file_size+this->dump_file_start << "]\n"; 
//cerr << "WRITING:" << *tseq << endl;
	//if(this->dump_file_start==8807514)
	//	cout << "##################\n" << *tseq << "\n";
//	cerr << "seekp to " << dump_file_start+dump_file_size << " (" << dump_file_start << " + " << dump_file_size << ")\n";
	//cerr << "---------WRITE-------------\n";

	dump_context(dump_file_id);
	TProjection_dump_file_seekp(dump_file_start+dump_file_size,'E');
	// Write elements
	dump_file_size+=mywrite((int)tseq->size());
	for(TSequence::iterator elem=tseq->begin(); elem!=tseq->end(); elem++)
	{
		dump_file_size+=mywrite((*elem)->time);
		dump_file_size+=mywrite((int)(*elem)->size());
		for(TElement::iterator item=(*elem)->begin(); item!=(*elem)->end(); item++)
			dump_file_size+=mywrite(*item);
	};
	// Write annotations
	dump_file_size+=mywrite((int)tseq->annotations->size());  // "annotations" should be always != 0
	for(vector<EntryPoint>::iterator ann=tseq->annotations->begin(); ann!=tseq->annotations->end(); ann++)
	{
		// Notice: the size of each annotation should be known a priori by the caller
		for(vector<float>::iterator tt=ann->times.begin(); tt!=ann->times.end(); tt++)
			dump_file_size+=mywrite(*tt);
		dump_file_size+=mywrite(ann->entrypoint.idx);
		dump_file_size+=mywrite(ann->entrypoint.position);
	};
	// Update global file size, if needed
	TProjection_dump_size=max(TProjection_dump_size,dump_file_start+dump_file_size);
//cerr << "[POS of " << this->myname() << ": post->" << this->dump_file_size+this->dump_file_start << "]\n"; 

	//if((int)TProjection_dump_file.tellg()!=(dump_file_start+dump_file_size))
	//	cout << "NOTICE: assumed pos=" << (dump_file_start+dump_file_size) << ", real=" << (int)TProjection_dump_file.tellg() << "\n";
	//cerr << "---------------------------\n";

};

/// Useful template for reading binary data
//template<class T> void myread(T& x) { 
//	int pp=(int)TProjection_dump_file.tellg(); 
//	//TProjection_dump_file_seekg(0,ios::end);
//	//int pp2=(int)TProjection_dump_file.tellg();
//	//TProjection_dump_file_seekg(pp);
//	TProjection_dump_file.read((char *)&x,sizeof(T));  
//	cerr << "read " << x << " @ " << pp  << "\n";
//};


template<class T> void myread(T& x) { TProjection_dump_file->read((char *)&x,sizeof(T)); };
void TProjection::retrieve_tseq_from_disk(TSequence* tseq, int prefix_len)
{
	//            __________elem1______________     _____________elemN___________ 
	// Format = N Time1 n1 Item1_1 ... Item1_n1 ... TimeN nN ItemN_1 ... ItemN_nN 
	//        + Na Time1_1 ... Time1_D IDX1 POS1 ... TimeNa_1 ... TimeNa_D IDXNa POSNa   
	//             \______times______/ \_entry_/     \________times______/ \__entry__/
	//
	// Read elements
	//cerr << "---------READ-------------\n";

	dump_context(dump_file_id);
	int N, n;
	//if(dump_file_read_pos>TProjection_dump_size) cerr << "* ";
	//cerr << "seekg to " << dump_file_read_pos << endl;
	TProjection_dump_file_seekg(dump_file_read_pos,'F');
	myread(N);
	if(N<0 || N>MAX_SEQUENCE_LEN)
	{
		cout << "\nSTOP while reading: " << *this->prefix << ", position " << dump_file_read_pos 
			<< " of " << dump_file_start << "-" << (dump_file_start+dump_file_size) 
			<< "\nDump file size: "	<< TProjection_dump_size << "  First read after init: " << just_init;
		this->dump_read_init(N,n);
		cout << "N: " << N << " pref_len: " << n;
		launch_error("Inconsistent sequence lenght met while reading dump file!!");
	};
	just_init=0;
	tseq->reserve(N);
	for(int i=0; i<N; i++)
	{
		TElement *elem=checked_new(TElement);
		tseq->push_back(elem);
		myread(elem->time);
		myread(n);
		elem->resize(n);
		for(TElement::iterator item=elem->begin(); item!=elem->end(); item++)
			myread(*item);
	};
	// Read annotations
	myread(N);
	if(N<1)
	{
		//cerr << "=======================\n" << *this << "==================\n";
		cerr << "Prefix: " << *this->prefix << "\n" << *tseq << endl << "Read N=" << N << endl;
		cerr << "File start pos=" << this->dump_file_start << endl;
		launch_error("Empty annotation read!!");
	};
	if(tseq->annotations==0)
		tseq->annotations=checked_new(vector<EntryPoint>);
	else
		tseq->annotations->clear();
	tseq->annotations->resize(N);
	//  This one was wrong (?)
	//	n=prefix_len>0?prefix_len-1:0;
	n=prefix_len;
	for(vector<EntryPoint>::iterator ann=tseq->annotations->begin(); ann!=tseq->annotations->end(); ann++)
	{
		ann->times.resize(n);
		for(vector<float>::iterator tt=ann->times.begin(); tt!=ann->times.end(); tt++)
			myread(*tt);
		myread(ann->entrypoint.idx);
		myread(ann->entrypoint.position);
	};
	if(TProjection_dump_file->tellg()<0)
	{
		cout << "Moving dump_file_read_pos from " << dump_file_read_pos << " to " 
			 << TProjection_dump_file->tellg() << endl;
	};
	dump_file_read_pos=TProjection_dump_file->tellg();

	//cerr << "--------------------------\n";
//cerr << "READ:" << *tseq << endl;
};

ostream& operator << (ostream& s, TProjection& tp) 
{ 
	s << "Prefix: ";
	if(tp.prefix) 
		s << *tp.prefix << "\n";
	else
		s << "EMPTY\n";
	if(tp.size()==0)
		s << "EMPTY\n";
	int c=1;
	for(TProjection::iterator pt=tp.begin(); pt!=tp.end(); pt++)
		s << c++ << ". " << **pt << endl;
	return s; 
}

ostream& operator << (ostream& s, Sequence& tp) 
{ 
	if(tp.size()==0)
		s << "EMPTY";
	else
	{
		for(Sequence::iterator pt=tp.begin(); pt!=tp.end(); pt++)
			s << *pt << "\t\t\t\t\t";
	};
	return s; 
}

ostringstream& operator << (ostringstream& s, Sequence& tp) 
{ 
	if(tp.size()==0)
		s << "EMPTY";
	else
	{
		for(Sequence::iterator pt=tp.begin(); pt!=tp.end(); pt++)
			s << *pt;
	};
	return s; 
}


ostream& operator << (ostream& s, Element& tp) 
{ 
	if(tp.size()==0)
		s << "(EMPTY)";
	else
	{
		s << "(" << name_mapping.get_string(tp.front());
		for(Element::iterator pt=tp.begin()+1; pt!=tp.end(); pt++)
			s << " " << name_mapping.get_string(*pt);
		s << ")";
	};
	return s; 
}

ostream& operator << (ostream& s, TSequence& tp) 
{ 
	s << "Sequence: " << tp.id << " ";
	if(tp.size()==0)
		s << "EMPTY";
	else
	{
		s << "<";
		for(TSequence::iterator pt=tp.begin(); pt!=tp.end(); pt++)
			s << **pt;
		s << ">";
	};
	s << "\nAnnotations: ";
	if(tp.annotations==0 || tp.annotations->size()==0)
		s << "NONE";
	else{
		for(vector<EntryPoint>::iterator pt=tp.annotations->begin(); pt!=tp.annotations->end(); pt++)
		{
			s << "(times: <";
			for(vector<float>::iterator tt=pt->times.begin(); tt!=pt->times.end(); tt++)
				s << *tt << " ";
			s << ">, entrypoint: [Elem idx: " << pt->entrypoint.idx << ", pos: " << pt->entrypoint.position << "])";
		};
		s << "\n";
	}
	return s; 
}

ostream& operator << (ostream& s, TElement& tp) 
{ 
	s << "(t=" << tp.time << ",";
	if(tp.size()==0)
		s << " EMPTY";
	for(TElement::iterator pt=tp.begin(); pt!=tp.end(); pt++)
		s << " " << name_mapping.get_string(*pt);
	s << ")";
	return s; 
}




