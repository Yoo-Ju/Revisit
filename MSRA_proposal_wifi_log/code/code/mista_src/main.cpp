#include "debug.h"
#include "Tsequences.h"
#include <iostream>
#include <algorithm>
#include <math.h>
#include <fstream>

using namespace std;

map<string,int> params;

my_clock my_time1, my_time2, my_time3;

void wait(const char *msg)
{
	cerr << msg << endl;
	cerr << "<Press ENTER to continue>";
	char c;
	cin.get(c);
};

int INMEMORY_LEVEL = 0;
int PS_BORDER = 4;
/// Main program
int main(int argc, char * argv[]) 
{
	cerr << "+------------------------------------------------+\n"
 		 << "| MiSTA v1.2 (c) 2007  Mirco.Nanni @ isti.cnr.it |\n"
		 << "| MIning Sequences with Temporal Annotations     |\n"
		 << "+------------------------------------------------+\n";

	// Step 0: Input parsing and Init
	if(argc<4)
	{ 
		cerr << "Syntax:\n\t" << argv[0] << " input_file min_support tau [OPTIONS]\n";
		cerr << "OPTIONS:-webtas | -waitkey | -absolute_support | \n"
			 << "         -no_annotation_pruning | -log | -KDTREE \n"
			 << "         -max_erasures NN | -max_insertions NN | -dot \n"
			 << "         -annot_partitions NN | -inmemory_level NN\n"
			 << "         -density_version NN | -postscript | -max_time_gap NN\n";
		cerr << "----------------------------------------------------------\n";
		exit(-1); 
	};
	for(int i=4; i<argc; i++)
		params[argv[i]]=i;
	string admitted_chars(".0123456789");
	if(find(admitted_chars.begin(),admitted_chars.end(),argv[2][0]) == admitted_chars.end())
		launch_error("Invalid \"min_sup\" value: positive real value expected!");
	if(find(admitted_chars.begin(),admitted_chars.end(),argv[3][0]) == admitted_chars.end())
		launch_error("Invalid \"tau\" value: positive real value expected!");
	
	cerr << "+ Start...\n";
	// Step 1: First database scan (load it and compute frequencies)
	TProjection* input=checked_new(TProjection); 
	cerr << "| Loading \"" << argv[1] << "\"..."; cerr.flush(); 
	my_time1.elapse();
	my_time2.elapse();

// wait("\nNot loaded, yet...");

	if(params["-webtas"])
		input->load_file_simple_tas(argv[1]);
	else
		input->load_file_tas(argv[1]);
	int N = input->prefix_support;
	if(params["-absolute_support"])
	{
		min_sup=atoi(argv[2]);
		mystat.sigma=(float)min_sup/(float)N;
	}
	else
	{
		min_sup=(int)ceil(atof(argv[2]) * N);
		mystat.sigma=(float)atof(argv[2]);
	};
	float tau=(float)atof(argv[3]);
	mystat.tau=tau;

	cerr << "Done in " << my_time2.elapse() << " secs (" << N << " transactions).\n";

//cout << "DUMP:  Start=" << input->dump_file_start << " Size=" << input->dump_file_size << endl;
//my_clock tmp_time;
//tmp_time.elapse();
//input->dump_on_disk();
//cout << "Dump in " << tmp_time.elapse() << " secs.\n";

//wait("Should be not in main memory...");
//tmp_time.elapse();

//	 input->retrieve_from_disk();

//cout << "Recovered in " << tmp_time.elapse() << " secs.\n";
//  cout << *input;

//wait("DATASET only in memory...");

	cerr << "| + Generating projections...\n";

	std::ofstream out_stream;
	out_stream.open("MiSTA.output",std::ios_base::out | std::ios_base::app);
	if(!out_stream)
		launch_error("Cannot open OUTPUT file!");

	vector<TProjection*> proj_heap;
	vector<TProjection*> next_heap;

	next_heap.push_back(input);

	int freq_pat=0;
	int proj_num=0;
	int level=1;
	int density_version=0;
	float max_time_gap = 1e30; // Default: almost infinite max time gap
	bool annotation_pruning=(params["-no_annotation_pruning"]==0);
	bool log_on=(params["-log"]!=0);
	bool dot_graph=(params["-dot"]!=0);
	bool postscript=(params["-postscript"]!=0);
	indexing_strategy my_index_strategy=HOMEMADE; // Default: use my simple index
	if(params["-max_erasures"]!=0  && argc>(params["-max_erasures"]+1))
		ERASURE_COUNT_MAX=atoi(argv[params["-max_erasures"]+1]);
	if(params["-max_insertions"]!=0  && argc>(params["-max_insertions"]+1))
		INSERTION_COUNT_MAX=atoi(argv[params["-max_insertions"]+1]);
	if(params["-annot_partitions"]!=0  && argc>(params["-annot_partitions"]+1))
		ANNOTATION_PARTITIONS=atoi(argv[params["-annot_partitions"]+1]);
	if(params["-inmemory_level"]!=0  && argc>(params["-inmemory_level"]+1))
		INMEMORY_LEVEL=atoi(argv[params["-inmemory_level"]+1]);
	if(params["-density_version"]!=0  && argc>(params["-density_version"]+1))
		density_version=atoi(argv[params["-density_version"]+1]);
	if(params["-max_time_gap"]!=0  && argc>(params["-max_time_gap"]+1))
		max_time_gap=(float)atof(argv[params["-max_time_gap"]+1]);	
	if(params["-KDTREE"]!=0)
		my_index_strategy=KDTREE;

	cerr << "==> max_time_gap = " << max_time_gap << endl;

	ofstream log_file;
	ofstream dot_file;
	if(log_on) 
	{
		log_file.open("MiSTA.log");
		log_file << "#d\tAnn.\tOutHs\tTrueHs\tTime\tTprune\n";
	};

	if(dot_graph)
	{
		dot_file.open("MiSTA.dot");
		dot_file << "digraph G {\n";
	};

	// Level-wise expansion of sequences. Optimization: revert to depth-first to reduce memory consumption!
	while(!next_heap.empty())
	{
		int lev_count=0;
		proj_heap = next_heap;
		next_heap.clear();
		for(vector<TProjection*>::iterator exp=proj_heap.begin(); exp!=proj_heap.end(); exp++)
		{

			// DEBUG
			std::cout << "\n--- Prefix: " << *(*exp)->prefix << " ---\n";
			for(map<int,int>::iterator it=(*exp)->item_freq_enlarge.begin(); it!=(*exp)->item_freq_enlarge.end(); it++)
				std::cout << "  enl_freq(" << it->first << ")=" << it->second << endl;
			for(map<int,int>::iterator it=(*exp)->item_freq_extend.begin(); it!=(*exp)->item_freq_extend.end(); it++)
				std::cout << "  ext_freq(" << it->first << ")=" << it->second << endl;
			// END DEBUG

			if(dot_graph)
			{
				int a,b,c,d;
				(*exp)->evaluate_size(a,b,c,d);
				dot_file << (*exp)->myname() << "  [label=\"" << *(*exp)->prefix << "\\n" << (*exp)->prefix_support << "->" << a << "\\nEl:" << b << ",It:" << c << ",An:" << d << "\"];\n";
			};

			int h_count=0; // Frequent annotations found for actual pattern
			if((int)((*exp)->prefix_support)>=min_sup) 
			{
				if(!(*exp)->prefix->empty())
				{
					std::cout << *(*exp)->prefix << ": " << (float)(*exp)->prefix_support/(float)N 
						       << "\t\t[abs:" << (int)(*exp)->prefix_support <<  "]" << endl;
					//freq_pat++;
				};

				if((*exp)->prefix->size()>1) // There are transition times to cluster
				{
					if((*exp)->size()==0)
						(*exp)->retrieve_from_disk();

					int mycc=0;
					if(log_on) 
					{
						for(TProjection::iterator it=(*exp)->begin(); it!=(*exp)->end(); it++) 
							mycc+=(int)(*it)->annotations->size();
						std::cout << ((int)(*exp)->prefix->size()-1) << "\t"  << mycc << "\t";
						my_time3.elapse();
					};

					H_store *results;
					switch(density_version) {
						case 0:
                   			results=(*exp)->extract_dense_rectangles_brute_force2(tau, my_index_strategy);
							break;
						case 1:
							results=(*exp)->extract_dense_rectangles_brute_force(tau, my_index_strategy);
							break;
						case 2:
							results=(*exp)->extract_dense_rectangles(tau, my_index_strategy);
							break;
						default:
							launch_error("Wrong density_version -- valid values are 0,1 and 2!!");
					};

					if(log_on) 
					{
						int unempty_cc=0;
						for(vector<H*>::iterator it=results->rectangles.begin(); it!=results->rectangles.end(); it++)
							if(*it!=0) unempty_cc++;
						// log_file << unempty_cc;
						log_file << (int)results->rectangles.size() << "\t" << unempty_cc << "\t" << my_time3.elapse();
					};
					for(vector<H*>::iterator it=results->rectangles.begin(); it!=results->rectangles.end(); it++)
						if(*it!=0 && (*it)->density>=min_sup)
							{
							std::cout << " " << (++h_count) << ". Density:" << (*it)->density << ". " << **it << endl; 
							}


					if(h_count>0) // If no dense region found => do not even try to project anything from *exp
					{
						if(dot_graph)
							dot_file << "  " << (*exp)->myname() << " [color=green,style=filled];\n";
						freq_pat++;
						// Annotation-based pruning
						if (annotation_pruning) 
							(*exp)->filter_annotations(results,tau);
						if(postscript && (*exp)->prefix->size()==3)
						{
							string fname=(*exp)->myname()+".ps";
							ofstream ps(fname.c_str());
							float minx, maxx, miny, maxy, mind, maxd;
							mind=minx=miny=10000000;
							maxd=maxx=maxy=-10000000;
							for(vector<H*>::iterator it=results->rectangles.begin(); it!=results->rectangles.end(); it++)
								if(*it!=0 && (*it)->density>=min_sup)
								{
									minx=min(minx,(*it)->low[0]);
									miny=min(miny,(*it)->low[1]);
									maxx=max(maxx,(*it)->up[0]);
									maxy=max(maxy,(*it)->up[1]);
									mind=min(mind,(float)(*it)->density);
									maxd=max(maxd,(float)(*it)->density);
								};
							if(maxx==minx)
								maxx=minx+1;
							if(maxy==miny)
								maxy=miny+1;
							if(maxd==mind)
								maxd=mind+1;

							ps << "%!PS-Adobe-2.0" << endl
								<< "%%Creator: MiSTA 1.1" << endl
								<< "%%For: GhostView" << endl
								<< "%%Title: Density plot" << endl
								<< "%%BoundingBox: 10 10 580 830" << endl
								<< "%%EndComments" << endl << endl;

							for(vector<H*>::iterator it=results->rectangles.begin(); it!=results->rectangles.end(); it++)
								if(*it!=0 && (*it)->density>=min_sup)
								{
									float x0, x1, y0, y1;
									x0=10+570*((*it)->low[0]-minx)/(maxx-minx);
									x1=10+570*((*it)->up[0]-minx)/(maxx-minx)-PS_BORDER;
									y0=10+820*((*it)->low[1]-miny)/(maxy-miny);
									y1=10+820*((*it)->up[1]-miny)/(maxy-miny)-PS_BORDER;
									ps << "newpath\n";
									ps << " " << x0 << " " << y0 << " moveto\n";
									ps << " " << x1 << " " << y0 << " lineto\n";
									ps << " " << x1 << " " << y1 << " lineto\n";
									ps << " " << x0 << " " << y1 << " lineto\n";
									ps << " closepath\n";
									ps << " " << (0.5*(1-((float)(*it)->density-mind)/(maxd-mind))) 
										<< " setgray\n";
									ps << " fill\n";
								};
							ps << "showpage\n";
							ps.close();
						};
					} else
						if(dot_graph)
							dot_file << "  " << (*exp)->myname() << " [color=red,style=filled];\n";
					if(log_on)
						log_file << "\t" << my_time3.elapse() << endl;
					results->clear();
					checked_delete(results);
				};

				if((h_count>0) || ((*exp)->prefix->size()<2))
					// If no frequent annotation was found, do not project 
					// -- unless we are working on a 0- or 1-prefix
				{
					// Update item frequency within projection and delete unfrequent items
					// Notice: "filter_annotation" could have deleted a significant amount of items/sequences
					//// OUTDATED: not needed any more
					//if((*exp)->prefix->size()>0) // Frequencies for 0-prefixes got from loading phase
					//	(*exp)->set_item_freq();
					//(*exp)->filter_items();

					//cerr << "ENTER...\n";
					bool disk_in=(*exp)->size()==0;
					bool disk_out=(int)(*exp)->prefix->size()<INMEMORY_LEVEL;
					vector<TProjection*> *projections = (*exp)->generate_projections(disk_in,disk_out,max_time_gap);
					//cerr << "...EXIT\n";

					//for(vector<TProjection*>::iterator pr=projections->begin(); pr!=projections->end(); pr++)
					//	cout << "=================\n" << **pr;

					if(dot_graph)
					{
						for(vector<TProjection*>::iterator pr=projections->begin(); pr!=projections->end(); pr++)
						{
							dot_file << "  " << (*exp)->myname() << " -> " << (*pr)->myname() << ";\n";
						};
					};
					proj_num += (int)projections->size();
					lev_count +=(int)projections->size();
					for(vector<TProjection*>::iterator it=projections->begin(); it!=projections->end(); it++)
					{
						next_heap.push_back(*it);
						// cout << "HERRR" << **it << "YAY" << endl;
					}
				};
			};
 			checked_delete(*exp);
 		};
		cerr << "| | " << level++ << "-prefix: " << lev_count << " projections in " << my_time2.elapse() << " secs.\n";
// wait("PROJECTIONS...");
	};

	out_stream.close();
	if(log_on) log_file.close();

	if(dot_graph)
	{
		dot_file << "}\n";
		dot_file.close();
	};

	mystat.time=my_time1.elapse();
	mystat.Nproj=proj_num;
	cerr << "| + Projections done\n";
	cerr << "+ Done in " << mystat.time << " secs.\n" << proj_num << " projections generated.\n" << freq_pat 
		<< " frequent sequences found.\n";

	mystat.print();

	if(params["-waitkey"])
		wait("");

	//cout << "-----------------------\nNew: " << new_counter 
	//	 << "\nDelete: " << delete_counter 
	//	 << "\n-----------------------\n";

	//cout << "Allocation - Deallocation count:\n";
	//cout << "TElements: " << elem_count << " / " << elem_count_t << " = " << (float)elem_count*100/elem_count_t;
	//cout << "\nEntryPoints: " << entry_count << " / " << entry_count_t << " = " << (float)entry_count*100/entry_count_t << endl;
	return 0;
}










