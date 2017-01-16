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

#include <iostream>
#include <map>
#include <iostream>
#include <algorithm>
#include <math.h>
#include <fstream>
#include <string>
#include "db.h"
#include "Tsequences.h"
#include <ctype.h>
#include "debug.h"
#include "output.h"
	

using namespace std;
/*
#ifdef _WIN32
	int OPERATIVE_SYSTEM = 1;
#endif

#ifdef linux
	int OPERATIVE_SYSTEM = 2;
#endif

#ifdef __APPLE__ & __MACH__
	int OPERATIVE_SYSTEM = 0;
#endif
*/

map<string,int> params;


int main (int argc, char * const argv[]) {

	cerr << "+----------------------------------------------------------+\n"
 		 << "| T-pattern Miner v0.9 (c) 2007                            |\n"
		 << "| M. Nanni and F. Pinelli KDDlab, ISTI-CNR, Pisa           |\n"
		 << "| Mirco.Nanni@isti.cnr.it, Fabio.Pinelli@isti.cnr.it       |\n"              
		 << "+----------------------------------------------------------+\n";

	// Step 0: Input parsing and Init
	if (argc==2){
		for(int i=1; i<argc; i++)
			params[argv[i]]=i;
		if (params["-help"]!=0){
			cerr << "Full list of options available for this software: \n\n";
			
			cerr << "\t -density         \t(default value = min_sup) \n"
				  << "\t -epsilon         \t(default value = 0)\n"
				  << "\t -side            \t(default value = 1/100 of bounding box)\n" 
				  << "\t -max_reg_size NN \t(default value = infinite)\n" 
				  << "\t -no_interpolate  \t(default value = OFF)\n"
				  << "\t -rescale_density \t(default value = OFF)\n" 
				  << "\t -time_gap NN     \t(default value = 0)\n"
				  << "\t -space_gap NN    \t(default value = 0)\n\n";
				 
			cerr << "List of debug options: \n\n";
			
			cerr << "\t -max_n_trajs NN  \t(default value = infinite)\n"
				  << "\t -max_n_points NN \t(default value = infinite)\n"
				  << "\t -skip_n_trajs NN \t(default value = 0)\n"
				  << "\t -verbose         \t(default value = OFF)\n";
			exit(-1);
		}
	}
	
	if(argc<4)
	{ 
		cerr << "Syntax:\n\t" << argv[0] << " input_file min_support tau [OPTIONS]\n";
		cerr << "\t *** Type -help for help and full list of options ***\n";
		exit(-1); 
	};
	for(int i=4; i<argc; i++)
		params[argv[i]]=i;
	string admitted_chars(".0123456789");
	
	cerr << "+ Start...\n";
	// Step 1: First database scan (load it and compute frequencies)
	
	ifstream file;
	
	file.open(argv[1], ios::in);
	//read the input file
	if (!file){	
		cout << "cannot find the file\n";
		exit(0);
	}
	file.close();

	int threshold;
	float tau;
	
	tau = (float)atof(argv[3]);
	
	float min_gap = 0;
	int region_version = 0;
	float epsilon = 0;
	int max_n_trajectories=99999999;
	int trajectories_to_skip = 0;
	float min_distance=0;
	float difference_tolerance=0;
	int max_n_points = 99999999;
	int directional=0;
	int without_interpolation = 0;
	int static_v = 0;
	int semi_static=0;
	char * regions_file="";
	int verbose = 0;
	directional = 1;
	float density_percentage=0;
	int max_reg_size = 99999999;

	string open_brake = "(";
	string close_brake = ")";
	string space = " ";

	if (params["-region_version"]!=0  && argc>(params["-region_version"]+1))
		region_version = atoi(argv[params["-region_version"]+1]);
	if (params["-epsilon"]!=0  && argc>(params["-epsilon"]+1))
		epsilon = (float) atof(argv[params["-epsilon"]+1]);
	if (params["-max_reg_size"]!=0  && argc>(params["-max_reg_size"]+1))
		max_reg_size = atoi(argv[params["-max_reg_size"]+1]);
	if ((params["-time_gap"]!=0) && argc>(params["-time_gap"]+1))
		min_gap = (float)atof(argv[params["-time_gap"]+1]);
	if ((params["-max_n_trajs"]!=0) && argc>(params["-max_n_trajs"]+1))
		max_n_trajectories = atoi(argv[params["-max_n_trajs"]+1]);
	if (params["-skip_n_trajs"]!=0  && argc>(params["-skip_first_n_trajs"]+1))
		trajectories_to_skip = atoi(argv[params["-skip_first_n_trajs"]+1]);
	if (params["-space_gap"]!=0  && argc>(params["-space_gap"]+1))
		min_distance = (float)atof(argv[params["-space_gap"]+1]);
	if (params["-max_n_points"]!=0  && argc>(params["-max_n_points"]+1))
		max_n_points = (int)atoi(argv[params["-max_n_points"]+1]);
	if (params["-difference_tolerance"]!=0  && argc>(params["-difference_tolerance"]+1))
		difference_tolerance = (float) atof(argv[params["-difference_tolerance"]+1]);
	if (params["-static"]!=0  && argc>(params["-static"]+1)){
		static_v = 1;
		regions_file = argv[params["-static"]+1];
	}
	if (params["-semi_static"]!=0)
		semi_static = 1;
	if (params["-verbose"]!=0)
		verbose = 1;
	if (params["-no_interpolate"]!=0)
		without_interpolation = 1;
	float side_grid=0;
	if (params["-side"]!=0  && argc>(params["-side"]+1))
		side_grid = (float)atof(argv[params["-side"]+1]);
	if (side_grid ==0 && params["-side"]!=0){
		cout << "##########The grid side must be greater than 0!\n";
		exit(0);
	}
		
	db * DB = new db (argv[1]);
	
	DB->side_grid = side_grid;
	cout << "N. of trajectories loaded: " << DB->size() << endl;
	cout << "Semi_static: " << semi_static <<  " static: " << static_v << endl; 
	if(params["-density"])
	{
		//min_sup=atoi(argv[2]);
		threshold = (int)atoi(argv[params["-density"]+1]);
		density_percentage = (float) ((float)threshold / (float)DB->size());
	}
	
	else {
		threshold = (int)ceil((atof(argv[2])*DB->size())); //argv[2] is the minimum support parameter
	}

	
	int next_name_region=0;
	cout << "Min x: " << DB->min_x << " min y: " << DB->min_y << " max x: " << DB->max_x << " Max y: " << DB->max_y << endl;
	Grid * grid = new Grid (DB->min_x, DB->min_y, DB->max_x, DB->max_y, DB->side_grid);
	grid->print();
	cout << "Side cell: " << grid->side << endl;
	
	if (static_v){
		DB->load_regions(regions_file, grid);
	}
	
	if (without_interpolation){
		cout << "Calculating bound without interpolation\n";
		DB->set_density(grid, epsilon);
	}
	else{
		DB->interpolation(grid, epsilon, threshold, directional);
	}
	
	Grid * supp_grid = new Grid (DB->min_x, DB->min_y, DB->max_x, DB->max_y, DB->side_grid);
	
		for (vector<Trajectory *>::iterator tr = DB->begin(); tr != DB->end(); tr++) {
		supp_grid->reset_values();
		vector <Cell *> distinct_cells = (*tr)->locations(supp_grid);
		grid->compute_density(distinct_cells);
		distinct_cells.clear();
	}
	grid->print(threshold);

	if (!static_v){
		DB->regions = grid->bounded_popular_regions(threshold, 0, 0, max_reg_size);
		cout << "regions.size: " << threshold  << " "  << DB->regions.size() << endl;
	}

	TProjection* input=checked_new(TProjection);
	next_name_region = (int) DB->regions.size(); // next name of region (it is a progressive number)
	//Sequence * prefix_empty = checked_new(Sequence);
	grid->print();
	input = DB->translation(grid);
	int N = input->prefix_support;
	min_sup =(int)ceil(atof(argv[2]) * N);
	
	string support_string="";
	string filename="";
	//my_time1.elapse();
	//my_time2.elapse();

	cout <<"N: " <<N;
	
	if(params["-absolute_support"])
	{
		//min_sup=atoi(argv[2]);
		mystat.sigma=(float)min_sup/(float)N;
	}
	else
	{
		mystat.sigma=(float)atof(argv[3]);
		cout <<" Min sup/density: " << min_sup << "/" << threshold << " ";
	};
	
	cout << " Tau:" << tau << endl; 
	mystat.tau=tau;
	cout << "Number of ROI: " << DB->regions.size() << endl;
	cerr << "| + Generating projections...\n";

	ofstream out_stream("MiSTA.output");
	if(!out_stream)
		launch_error("Cannot open OUTPUT file!");

	vector<TProjection*> proj_heap;
	vector<TProjection*> next_heap;
	vector <int>  freq_patterns;
	freq_patterns.reserve(10);
	for (int i = 0; i<=10; i++){
		freq_patterns.push_back(0);
	}
	next_heap.push_back(input);	
	
	int freq_pat=0;
	int proj_num=0;
	int level=1;
	int density_version=0;
	bool annotation_pruning=(params["-no_annotation_pruning"]==0);
	//bool postscript=(params["-postscript"]!=0);
	
	if(params["-max_erasures"]!=0  && argc>(params["-max_erasures"]+1))
		ERASURE_COUNT_MAX=atoi(argv[params["-max_erasures"]+1]);
	if(params["-max_insertions"]!=0  && argc>(params["-max_insertions"]+1))
		INSERTION_COUNT_MAX=atoi(argv[params["-max_insertions"]+1]);
	if(params["-annot_partitions"]!=0  && argc>(params["-annot_partitions"]+1))
		ANNOTATION_PARTITIONS=atoi(argv[params["-annot_partitions"]+1]);
//	if(params["-inmemory_level"]!=0  && argc>(params["-inmemory_level"]+1))
//		INMEMORY_LEVEL=atoi(argv[params["-inmemory_level"]+1]);
	if(params["-density_version"]!=0  && argc>(params["-density_version"]+1))
		density_version=atoi(argv[params["-density_version"]+1]);
	
	vector <region * > regions_for_level;
	regions_for_level = DB->regions;
	int freq_patterns_level=0;
	int delete_regions=0; //to count the regions not considered
	// Level-wise expansion of sequences. Optimization: revert to depth-first to reduce memory consumption!
	if (!static_v){
		DB->print_regions("regions.output", 0, grid->side);
		DB->print_regions_jump("regions.wkt",grid,threshold);
	}
	if (verbose){ //if the user set the verbose option we print a file for each found region 								
		for (vector<region *>::iterator it_reg = DB->regions.begin(); it_reg != DB->regions.end(); it_reg++){
			if (!(*it_reg)->cuted){
				ostringstream oss2;
				oss2 << (*it_reg)->nOfRegion;
				support_string=oss2.str();
				oss2.clear();
				filename = "regions";
				filename = filename + open_brake + support_string + close_brake + ".wkt";
				ofstream regions_s;
				regions_s.open(filename.c_str(), ios::out);
				if (!(*it_reg)->cuted){
					regions_s << "POLYGON ((" << (*it_reg)->min_x* grid->side << " " << (*it_reg)->min_y * grid->side << ", "
					<< (*it_reg)->min_x * grid->side << " " << ((*it_reg)->max_y*grid->side) + grid->side << ", " 
					<< ((*it_reg)->max_x * grid->side) + grid->side << " " << ((*it_reg)->max_y*grid->side) + grid->side << ", "
					<< ((*it_reg)->max_x * grid->side) + grid->side << " " << ((*it_reg)->min_y*grid->side) << ", "
					<< (*it_reg)->min_x * grid->side << " " << (*it_reg)->min_y * grid->side << "))\n\n";
				}
				regions_s.close();

				filename.clear();
				support_string.clear();
			}
		}
	}	
	
	while(!next_heap.empty())
	
	{
		if (level > 2){
			freq_patterns[level-2]=freq_patterns_level;
			freq_patterns_level=0;
		}
		int lev_count=0;
		proj_heap = next_heap;
		if (level==1)
			freq_patterns[level]= (int)DB->regions.size();
		next_heap.clear();
		for(vector<TProjection*>::iterator exp=proj_heap.begin(); exp!=proj_heap.end(); exp++)
		{
			//cout << *(*exp);
			int h_count=0; // Frequent annotations found for actual pattern
			if((int)((*exp)->prefix_support)>=min_sup) 
			{
				if(!(*exp)->prefix->empty())
				{
					out_stream << *(*exp)->prefix << ": " << (float)(*exp)->prefix_support/(float)N 
						       << "\t\t[abs:" << (int)(*exp)->prefix_support <<  "]\n";
				};
				
				
				if((*exp)->prefix->size()>1) // There are transition times to cluster
				{
					//cout << "PREFIX SIZE >1\n";
					if((*exp)->size()==0)
						(*exp)->retrieve_from_disk();
					
				
					H_store *results;
					
					switch(density_version) {
						case 0:
                     results=(*exp)->extract_dense_rectangles_brute_force2(tau);
							break;
						case 1:
							results=(*exp)->extract_dense_rectangles_brute_force(tau);
							break;
						case 2:
							results=(*exp)->extract_dense_rectangles(tau);
							break;
						default:
							launch_error("Wrong density_version -- valid values are 0,1 and 2!!");
					};

					for(vector<H*>::iterator it=results->rectangles.begin(); it!=results->rectangles.end(); it++){
						if(*it!=0 && (*it)->density>=min_sup){   //TODO: what about deleting non-dense rectangles?
							out_stream << "\t" << **it; 
							h_count++;
						};
					}
					


					if(h_count>0) // If no dense region found => do not even try to project anything from *exp
					{
						freq_pat++;
						freq_patterns_level++;
						// Annotation-based pruning						
						if (annotation_pruning){
							(*exp)->filter_annotations(results,tau);
						}

					} 
					results->clear();
					checked_delete(results);
				};

				// Delete IDX=-2 for length-1 prefix, using the filter_annotation function
				if (annotation_pruning && ((*exp)->prefix->size()==1)){

					(*exp)->filter_annotations(NULL,tau);
				}

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

					vector<TProjection*> *projections = (*exp)->generate_projections();
			//		cout << "N. Projections: " << projections->size() << endl ;
					if (!(static_v || semi_static)){
						vector <float>::iterator t;
						db * DB_projected = new db ();
						
						for (vector<TProjection*>::iterator p = projections->begin(); p != projections->end(); p++){
							vector <Trajectory *> cuted_trajectories;
							
							for (vector <TSequence *>::iterator s = (*p)->begin(); s!= (*p)->end(); s++){
								float small_time = 10000000;
								for (vector<EntryPoint>::iterator a = (*s)->annotations->begin(); a != (*s)->annotations->end(); a++){
									t = a->times.end()-1;
									if (*t < small_time) {
										small_time = *t;
									}
								
								}
								int iter = 0;
								
								Trajectory * tmp_traj = new Trajectory (); 
								small_time = small_time+min_gap;
								//find next item with a time with a difference of min_gap
								while (small_time > DB->at((*s)->id)->time_stamp.at(iter) && iter < (int)DB->at((*s)->id)->time_stamp.size()-1) {
									iter++;
								}
								tmp_traj->sequence.assign(DB->at((*s)->id)->sequence.begin() + iter, DB->at((*s)->id)->sequence.end()); 
								tmp_traj->time_stamp.assign(DB->at((*s)->id)->time_stamp.begin() + iter, DB->at((*s)->id)->time_stamp.end());
								tmp_traj->id = (*s)->id;
								cuted_trajectories.push_back(tmp_traj);
							}
							
							grid->reset_values();
							//compute density for each projections
							DB_projected->clear();
							
							for (vector<Trajectory *>::iterator tr = cuted_trajectories.begin(); tr != cuted_trajectories.end(); tr++) {
								supp_grid->reset_values();								
								vector <Cell *> distinct_cells = (*tr)->locations(supp_grid);
								grid->compute_density(distinct_cells);
								DB_projected->push_back(*tr);
								distinct_cells.clear();
							}
							if (params["-rescale_density"]){
								if (params["-density"]){
									threshold =(int) (density_percentage * DB_projected->size()); //the user value
								}
								else{
									threshold =(int)ceil(atof(argv[2]) * DB_projected->size()); //linked to the minimum support threshold
								}
							}
							DB_projected->regions = grid->bounded_popular_regions(threshold, next_name_region, 0, max_reg_size);
							//cout << "Regions Founded: " << DB_projected->regions.size() << endl;
							//compare all regions founded with the last region of the prefix
							Element t= (*p)->prefix->back();
							int last_id = t.back();
							region * last_region = regions_for_level[last_id];
							
							for (vector <region *>::iterator reg = DB_projected->regions.begin(); reg!=DB_projected->regions.end(); reg++){
								regions_for_level.push_back(*reg);
								//cout << "Parameters: " << difference_tolerance << " " << min_distance << endl; 
								if ((*reg)->compare(last_region, difference_tolerance) || (*reg)->distance(last_region, min_distance, grid->side)){
									(*reg)->cuted=1;
									for (vector <Cell *>::iterator cx = (*reg)->begin(); cx != (*reg)->end();cx++){
										grid->getCell((*cx)->X_Index, (*cx)->Y_Index)->region=-1;
									}
									delete_regions++;
								}
							}
							
							
							DB_projected->print_regions("regions.output", level, grid->side);

							//**to write the file pattern could be a method of the object DB** 
							
							if (verbose){						
								//cycle on prefix...
								ostringstream prefix_oss;
								prefix_oss << *(*p)->prefix;
								string token="";
								string containment="";
								ifstream prefix_file;
								support_string=prefix_oss.str();
								
								filename = "regions";
								filename = filename + support_string + ".wkt";
								prefix_file.open(filename.c_str(), ios::in);
								
								if (!prefix_file){
									cout << "Cannot open the file: " << filename << endl;
									exit(-1);
								}
								while (!prefix_file.eof()){
									prefix_file >> token;
									if (prefix_file.eof()){
										break;
									}
									containment = containment + space + token;
									
								}
								
								filename.clear();
								support_string.clear();
								prefix_file.close();
								
								string prefix = prefix_oss.str();
								for (vector<region *>::iterator it_reg = DB_projected->regions.begin(); it_reg != DB_projected->regions.end(); it_reg++){
									if (!(*it_reg)->cuted){
										ostringstream oss2;
										oss2 << (*it_reg)->nOfRegion;
										support_string=oss2.str();
										oss2.clear();
										filename = "regions";
										filename = filename + prefix + open_brake + support_string + close_brake + ".wkt";
										ofstream regions_s;
										regions_s.open(filename.c_str(),ios::out);
										regions_s << containment << "\n";
										if (!(*it_reg)->cuted){
											//(*it_reg)->bound(grid->side);
											regions_s << "POLYGON ((" << (*it_reg)->min_x* grid->side << " " << (*it_reg)->min_y * grid->side << ", "
											<< (*it_reg)->min_x * grid->side << " " << ((*it_reg)->max_y*grid->side) + grid->side << ", " 
											<< ((*it_reg)->max_x * grid->side) + grid->side << " " << ((*it_reg)->max_y*grid->side) + grid->side << ", "
											<< ((*it_reg)->max_x * grid->side) + grid->side << " " << ((*it_reg)->min_y*grid->side) << ", "
											<< (*it_reg)->min_x * grid->side << " " << (*it_reg)->min_y * grid->side << "))\n\n";
										}
										regions_s.close();
										filename.clear();
										support_string.clear();
									}
								}
								prefix_oss.clear();
								containment.clear();
							}	
							
							next_name_region = next_name_region+((int)DB_projected->regions.size());
							
							Sequence *Seq = (*p)->prefix;
							int c_traj=0;

							for (vector <TSequence *>::iterator tseq = (*p)->begin(); tseq!= (*p)->end(); tseq++, c_traj++){
								DB_projected->at(c_traj)->translation(*tseq, grid, *p);
								//cout <<  **tseq;
							}
							
							 (*p)->prefix = Seq;
							
						}
						delete DB_projected;
					}
					proj_num += (int)projections->size();
					lev_count +=(int)projections->size();
					
					for(vector<TProjection*>::iterator it=projections->begin(); it!=projections->end(); it++){
						next_heap.push_back(*it);
					}
				};
			};
 			checked_delete(*exp);
 		};
		
		cerr << "| | " << level++ << "-prefix: " << lev_count << " \n";
	};

	out_stream.close();

	//mystat.time=my_time1.elapse();
	mystat.Nproj=proj_num;
	cerr << "| + Projections done\n";
	cerr << "+ Done in " << mystat.time << " secs.\n" << proj_num << " projections generated.\n" << freq_pat 
		 << " frequent sequences found " << "regions cuted: " << delete_regions << endl;

	//mystat.print();
	freq_patterns[1]=(int)DB->regions.size();
	cerr << "TEST: " << next_name_region << " ";
	for (int i=1 ; i <= (int)freq_patterns.size()-1 ; i++){
		cerr << freq_patterns[i] << " " ;
	}
	
    return 0;
}



