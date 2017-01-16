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

#include "db.h"
#include "output.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <math.h>
#include <stdio.h>
#include <algorithm>
/*
#ifdef _WIN32
	#include <malloc.h>
	OPERATIVE_SYSTEM_DB=1;
	malloc_statistics_t my_malloc;
	float memory;
	float MB = 1024*1024;
#endif

#ifdef linux
	int OPERATIVE_SYSTEM_DB = 2;
#endif

#ifdef __APPLE__ & __MACH__
	#include <malloc/malloc.h>
	int OPERATIVE_SYSTEM_DB = 0;
	malloc_statistics_t my_malloc;
	float memory;
	float MB = 1024*1024;
#endif
*/



using namespace std;

/*
malloc_statistics_t +;
float memory;
float MB = 1024*1024;
*/

point::point( ){
	x = 0 ;
	y = 0;
	m = 0;
	q = 0;
}

point::point(float _x ,float _y){
	x = _x;
	y = _y;
}

void point::interpolation(point * _p){
	float dy = _p->y - y;
	float dx = _p->x - x;
	if (dx == 0 || dy == 0 ){
		if (dx == 0){//y=y1
			m = 0;
			q = y;
		}
		if (dy == 0){//x=x1
			m = 0;
			q = x;
		}
	}
	else {
		m = (_p->y - y) / (_p->x - x);
		q = -(x * ((_p->y - y) / (_p->x - x))) + y;
	}
}

void point::set (float _x ,float _y){
	this->x = _x;
	this->y = _y;
}
bound * point::set_bound_first(float epsilon, Grid * grid){
	bound * bd = new bound();
	//float size = grid->getSide();
	
	bd->min_x_index = grid->getCurCell (this->x - epsilon, this->y - epsilon)->getX_Index();
	bd->min_y_index = grid->getCurCell (this->x - epsilon, this->y - epsilon)->getY_Index();
	bd->max_x_index = grid->getCurCell (this->x + epsilon, this->y + epsilon)->getX_Index();
	bd->max_y_index = grid->getCurCell (this->x + epsilon, this->y + epsilon)->getY_Index();

	/*
	for (int i = bd->min_x_index; i<=bd->max_x_index; i++){
		for (int j = bd->min_y_index; j<=bd->max_y_index; j++){
			grid->getCell(i,j)->noftrajectories=1;
		}
	}
	*/
	return bd;


}
bound * point::set_bound(float epsilon, Grid * grid){
	
	bound * bd = new bound();
	float size = grid->getSide();
	int less_size_x=0;
	int less_size_y=0;
	switch ((int)this->time) {
		case 1:
			less_size_x=1;
			break;
		case 2:
			less_size_y=1;
			break;
		default:
			less_size_x=0;
			less_size_y=0;
	}
	
	//float resize_x=size*less_size_x;
	float resize_y=size*less_size_y;
	
	bd->min_x_index = grid->getCurCell (this->x - epsilon/*-resize_x*/, this->y - epsilon)->getX_Index();
	bd->min_y_index = grid->getCurCell (this->x - epsilon, this->y - epsilon/*-resize_y*/)->getY_Index();
	bd->max_x_index = grid->getCurCell (this->x + epsilon/*-resize_x*/, this->y + epsilon)->getX_Index();
	bd->max_y_index = grid->getCurCell (this->x + epsilon, this->y + epsilon-resize_y)->getY_Index();
	/*
	for (int i = bd->min_x_index; i<=bd->max_x_index; i++){
		for (int j = bd->min_y_index; j<=bd->max_y_index; j++){
			grid->getCell(i,j)->noftrajectories=1;
		}
	}
	*/
	return bd;
}

bound * point::directional_bound(float epsilon, Grid * grid){ //based on direction of the generation point

	bound * bd = new bound();
	int direction = (int)this->time;
	float size = grid->side;
	//cout << "\t\tpoint: (" << this->x << " " << this->y << ")" << endl;
	
	switch (direction) {
	case 0: //pos_inc_x
		bd->min_x_index = grid->getCurCell(this->x, this->y)->getX_Index();
		bd->min_y_index = grid->getCurCell(this->x, this->y-epsilon)->getY_Index();
		bd->max_x_index = grid->getCurCell(this->x+epsilon, this->y)->getX_Index();
		bd->max_y_index = grid->getCurCell(this->x, this->y+epsilon)->getY_Index();
		 
		break;
	case 1: //pos_inc_y
		bd->min_x_index = grid->getCurCell(this->x-epsilon, this->y)->getX_Index();
		bd->min_y_index = grid->getCurCell(this->x, this->y)->getY_Index();
		bd->max_x_index = grid->getCurCell(this->x+epsilon, this->y)->getX_Index();
		bd->max_y_index = grid->getCurCell(this->x, this->y+epsilon)->getY_Index();
		 
		break;
	case 2: //pos_dec_x
		if (epsilon >= size/2)
			bd->min_x_index = grid->getCurCell(this->x, this->y-size)->getX_Index();
		else
			bd->min_x_index = grid->getCurCell(this->x-size, this->y-size)->getX_Index();
		bd->min_y_index = grid->getCurCell(this->x, this->y-epsilon)->getY_Index();
		if (epsilon >= size/2)
			bd->max_x_index = grid->getCurCell(this->x, this->y-size)->getX_Index();
		else
			bd->max_x_index = grid->getCurCell(this->x-size, this->y-size)->getX_Index();
		bd->max_y_index = grid->getCurCell(this->x, this->y+epsilon)->getY_Index();
		//cout << "x\n" ;
		//cout << "\t\t\tBound: ( " << bd->min_x_index << " " << bd->min_y_index << ", " << bd->max_x_index << " " << bd->max_y_index << ")\n";
		break;
	case 3: //pos_dec_y
		bd->min_x_index = grid->getCurCell(this->x-epsilon, this->y)->getX_Index();
		if (epsilon >= size/2)
			bd->min_y_index = grid->getCurCell(this->x, this->y-size)->getY_Index();
		else 
			bd->min_y_index = grid->getCurCell(this->x-size, this->y-size)->getY_Index();
		bd->max_x_index = grid->getCurCell(this->x+epsilon, this->y)->getX_Index();
		if (epsilon >= size/2)
			bd->max_y_index = grid->getCurCell(this->x, this->y-size)->getY_Index();
		else
			bd->max_y_index = grid->getCurCell(this->x-size, this->y-size)->getY_Index();
	
		//cout << "y\n" ;
		//cout << "\t\t\tBound: ( " << bd->min_x_index << " " << bd->min_y_index << ", " << bd->max_x_index << " " << bd->max_y_index << ")\n";
		 		
		break;
	case 4: //neg_inc_x
		bd->min_x_index = grid->getCurCell(this->x-size, this->y)->getX_Index();
		bd->min_y_index = grid->getCurCell(this->x, this->y-epsilon)->getY_Index();
		bd->max_x_index = grid->getCurCell(this->x+epsilon-size, this->y)->getX_Index();
		bd->max_y_index = grid->getCurCell(this->x, this->y+epsilon)->getY_Index();
		 
		break;
	case 5: //neg_inc_y
		bd->min_x_index = grid->getCurCell(this->x-epsilon, this->y)->getX_Index();
		bd->min_y_index = grid->getCurCell(this->x, this->y)->getY_Index();
		bd->max_x_index = grid->getCurCell(this->x+epsilon, this->y)->getX_Index();
		bd->max_y_index = grid->getCurCell(this->x, this->y+epsilon)->getY_Index();

		break;
	case 6: //neg_dec_x
		bd->min_x_index = grid->getCurCell(this->x, this->y)->getX_Index();
		bd->min_y_index = grid->getCurCell(this->x, this->y-epsilon)->getY_Index();
		bd->max_x_index = grid->getCurCell(this->x+epsilon, this->y)->getX_Index();
		bd->max_y_index = grid->getCurCell(this->x, this->y+epsilon)->getY_Index();
		break;
	case 7: //neg_dec_y
		bd->min_x_index = grid->getCurCell(this->x-epsilon, this->y-size)->getX_Index();
		if (epsilon !=0)
			bd->min_y_index = grid->getCurCell(this->x, this->y-epsilon)->getY_Index();
		else
			bd->min_y_index = grid->getCurCell(this->x, this->y-size-epsilon)->getY_Index();
		bd->max_x_index = grid->getCurCell(this->x+epsilon, this->y)->getX_Index();
		bd->max_y_index = grid->getCurCell(this->x, this->y-size)->getY_Index();
		break;
	default:
		break;
	}
	
	//cout << "\t\t\tBound: ( " << bd->min_x_index << " " << bd->min_y_index << ", " << bd->max_x_index << " " << bd->max_y_index << ")\n";
	/*
	for (int i = bd->min_x_index; i<=bd->max_x_index; i++){
		for (int j = bd->min_y_index; j<=bd->max_y_index; j++){
			//cout << "\t\t\tCell: ( " << i << " " << j << ")\n";
			grid->getCell(i,j)->noftrajectories=1;
		}
	}
	*/
	return bd;

} 
bound::bound(){

	this->min_x_index = 0;
	this->min_y_index = 0;
	this->max_x_index = 0;
	this->max_y_index = 0;
	this->direction=0; //could be 0 1 2 0: normal starting point
					   //				1: less size on x
					   //				2: less size on y
}

vector <Cell *> bound::bounding_cells(Grid * grid){
	vector <Cell *> returnSet;
	for (int i = this->min_x_index; i<=this->max_x_index; i++){
		for (int j = this->min_y_index; j<=this->max_y_index; j++){
			returnSet.push_back(grid->getCell(i,j));
		}
	}
	
	return returnSet;
}


void bound::bounding_cells(Grid * grid, vector <Cell *> returnSet){
	for (int i = this->min_x_index; i<=this->max_x_index; i++){
		for (int j = this->min_y_index; j<=this->max_y_index; j++){
			returnSet.push_back(grid->getCell(i,j));
		}
	}
}


db::db(){}

db::db(char * file_input){

	vector <Trajectory *> tmp_traj;
	vector <int> index;
	vector <int> iRegion;
	ifstream file;
	string token;
	
	int count;
	int count_point=0;
	int status;
	float x;
	float y;
	this->max_x = 0;
	this->max_y= 0;
	this->min_x=999999;
	this->min_y=999999;
	float time;
	int test = 0;
	int traj_id = 0;
	this->side_grid = 0;
	file.open(file_input);
	
	while (!file.eof())
	{
		
		Trajectory * new_trajectory = new Trajectory();
		status = 1;
		count = 0;
		test++;
		token.clear();
		file >> token;
		if (token.empty()){
			break;
		}
		string token2 = token;
		new_trajectory->id = traj_id;
		//atoi(token.c_str());
		file >> token;
		new_trajectory->snapshot = atoi(token.c_str());
		while ((count <= ((new_trajectory->snapshot)*3)-1)){
			file >> token;
			switch (status){
				case 1: 
					count++;
					count_point++;
					time = (float) atof(token.c_str());
					status = 2;
					break;
				case 2: 
					count++;
					x = (float)atof(token.c_str());
					if (x > max_x)
						max_x = x;
					if (x < min_x)
						min_x = x;
					status = 3;
					break;
				case 3: 
					if(count == (new_trajectory->snapshot*3)-1){
						count++;
						y = (float) atof(token.c_str());
						if (y > max_y)
							max_y = y;
						if (y < min_y)
							min_y = y;
						point * p = new point ( );
						p->x = x; p->y = y;
						p->time = time;
						new_trajectory->T_points.push_back(p);
	
					}
					else {
						count++;
						y =(float) atof(token.c_str());
						if (y > max_y)
							max_y = y;
						if (y < min_y)
							min_y = y;
						point * p = new point ( );
						p->x = x; p->y = y;
						p->time = time;
						new_trajectory->T_points.push_back(p);
					
					}
					
					status = 1;
					break;
			}
		}
		new_trajectory->print();
		this->push_back(new_trajectory);
		traj_id++;
		count = 0;
	
	}

}

void db::interpolation(Grid * grid, float epsilon, int threshold, int directional){

	point * next_p;
	int vertical=0;
	int first_point = 1;
	
	int generated_points = 0;
	int total_snapshot =0;
	int skiped_points=0;
		
	//Grid * supportGrid = new Grid (this->min_x, this->min_y, this->max_x, this->max_y, this->side_grid);
	//for each points for each trajectories we calculate the interpolation values
	for (vector <Trajectory *>::iterator new_trajectory = this->begin(); new_trajectory != this->end(); new_trajectory++ ){
		first_point=1;
		
		//cout << "Trajectory number: " << supporttraj++ << " with n.snapshots: " << (*new_trajectory)->snapshot << " ";
				
		total_snapshot+= (*new_trajectory)->snapshot;
		for (vector <point *>::iterator _p = (*new_trajectory)->T_points.begin(); _p != (*new_trajectory)->T_points.end()-1; _p++){
			if (_p+1 != (*new_trajectory)->T_points.end()){
				next_p = *(_p+1);
			}
		//	cout << count_point++ << " " << (*_p)->x <<  " " << (*_p)->y << " next_p: " << next_p->x << " " << next_p->y << endl;
			(*_p)->interpolation(next_p);
			
			if (first_point){
				first_point=0;
				//(*new_trajectory)->sequence.push_back(grid->getCurCell((*_p)->x,(*_p)->y));
				(*new_trajectory)->time_stamp.push_back((*_p)->time);
				(*new_trajectory)->sequence.push_back((*_p)->set_bound_first(epsilon, grid));
				generated_points++;
			}
			if ( grid->getCurCell((*_p)->x,(*_p)->y)->X_Index !=  grid->getCurCell(next_p->x,next_p->y)->X_Index || grid->getCurCell((*_p)->x,(*_p)->y)->Y_Index !=  grid->getCurCell(next_p->x,next_p->y)->Y_Index){
				if ((*_p)->m != 0){
				//normal piece of trajectory
					if ((*_p)->m > 0){
						if ((*_p)->x < next_p->x){
							(*new_trajectory)->find_cells_pos_inc(*_p, next_p, grid, epsilon, directional);
						}
						else{
							(*new_trajectory)->find_cells_pos_dec(*_p, next_p, grid, epsilon, directional);
						}
					}
					
					else {
						
						if ((*_p)->x < next_p->x){
							(*new_trajectory)->find_cells_neg_dec(*_p, next_p, grid, epsilon, directional);
						}
						else{
							(*new_trajectory)->find_cells_neg_inc(*_p, next_p, grid, epsilon, directional);
						}
					}
				}
				else {
					if ((*_p)->q == (*_p)->x){
						vertical = 0;	//horizontal
						(*new_trajectory)->find_cells_horizontal(*_p, next_p, grid, epsilon);
					}
					else {
						vertical = 1; //vertical
						(*new_trajectory)->find_cells_vertical(*_p, next_p, grid, epsilon);
					}
				}
			}
			else {
				 skiped_points++;
			}

		}
		//cout << endl;
		(*new_trajectory)->print();
		//grid->compute_density (supportGrid);
		//supportGrid->reset();
		generated_points+= (int)(*new_trajectory)->sequence.size();
		
	}
	
	//delete supportGrid;
	/*
	if (OPERATIVE_SYSTEM_DB == 0){
		malloc_zone_statistics (NULL, &my_malloc);
		memory = my_malloc.size_in_use/MB;
		cout << "\tMemory in use: " << memory << endl
		<< "\tgenerated points: " << generated_points << endl 
		<< "\tn. of snapshot: " << total_snapshot << "-" << skiped_points << " = " << total_snapshot-skiped_points << endl;
	}
	*/
	//grid->print();
	vector <region *> dummy;
	print_postscript_grid("density.eps", grid, dummy, threshold);
	
	
	print_postscript_grid("regions.eps", grid, this->regions, threshold);

}

TProjection * db::translation(Grid * grid){
	
	map<int,int> local_support;
	TProjection* input	=checked_new(TProjection);
	int position = 0;
	int item_count=0;
	int cycle=0;
	int written_tseq=0;
	bound * previousRegion = new bound ();
	
	vector <Cell *> bound_cell;
	Cell * tmp_cell = new Cell();
	for (vector<Trajectory *>::iterator traj = this->begin(); traj != this->end(); traj++, position++){
			
		name_mapping.disable();
		TSequence * tseq;
		TElement * telem;
		tseq = checked_new(TSequence);
		tseq->annotations=checked_new(vector<EntryPoint>);
		tseq->id = (*traj)->id ;// position;
		(*tseq->annotations).push_back(EntryPoint()); // Default entrypoint=(0,0) with an empty vector of times 
		cycle=0; //to avoid empty previousRegion
		vector <bound *>::iterator bd_sequence = (*traj)->sequence.begin();
		if (this->regions.size() != 0){
			for (vector<float>::iterator t = (*traj)->time_stamp.begin(); t != (*traj)->time_stamp.end(); t++){
				telem = checked_new(TElement);
				
				telem->time = *t;
				//Recover Cells of the bound
				bound_cell=(*bd_sequence)->bounding_cells(grid);
				//translation of each cell of the bound
				for ( vector <Cell *>::iterator cell = bound_cell.begin(); cell!= bound_cell.end(); cell++) {
					tmp_cell = grid->getCell((*cell)->X_Index,(*cell)->Y_Index);
					bool test = false;
					for (TElement::iterator it = telem->begin(); (it != telem->end()) && !test; it++){
						if (*it == tmp_cell->region)
							test=true;
					}
				
					if ((!find_region(previousRegion, tmp_cell->region, grid, cycle)) && (tmp_cell->region != -1) && !test ){ 
						telem->push_back(tmp_cell->region);	
						//telem->time = *t;
					}
				
				//previousRegion = tmp_previous_region;
				}
			
				for(TElement::iterator it=telem->begin(); it!=telem->end(); it++,item_count++)
				{
					local_support[*it]++;
				};
				bd_sequence++;
				previousRegion = *(bd_sequence-1);
			
				if (telem->size()!= 0)
					tseq->push_back(telem);
				cycle++;
				bound_cell.clear();
			}
			
		}
		int seq_len=(int)tseq->size();
		mystat.avg_seq_len+=(float)seq_len;
		if(seq_len<=MAX_SEQUENCE_LEN  && seq_len>1) // Update countings
		{
		//input->dump_tseq_to_disk(seq);
			written_tseq++;
			for(map<int,int>::iterator pt=local_support.begin(); pt!=local_support.end(); pt++)
			{
				input->item_freq_extend[pt->first]++;
			};
	};
	if(seq_len>0)
		input->prefix_support++;

	mystat.Nseq=input->prefix_support;
	mystat.avg_seq_len = mystat.avg_seq_len / mystat.Nseq;

	input->non_empty_sequences=written_tseq;
	input->push_back(tseq);
	
	//checked_delete(tseq);
	}
	return input;

}

void db::set_density(Grid * grid, float epsilon){
	//for each point we set to 1 the density of the cell where the point is, with also the epsilon-bound
	Grid * supportGrid = new Grid (this->min_x, this->min_y, this->max_x, this->max_y, this->side_grid);
	
	for (vector <Trajectory *>::iterator new_trajectory = this->begin(); new_trajectory != this->end(); new_trajectory++ ){
		for (vector <point *>::iterator _p = (*new_trajectory)->T_points.begin(); _p != (*new_trajectory)->T_points.end(); _p++) {
			(*new_trajectory)->sequence.push_back((*_p)->set_bound(epsilon, supportGrid));
		}
	}

	grid->compute_density(supportGrid);
	supportGrid->reset();

}

void db::load_regions(char * file_input, Grid * grid){

	ifstream file;
	file.open(file_input, ios::in);
	string token;
	
	int count = 0;
	
	if (!file){	
		cout << "cannot find the regions file\n";
		exit(0);
	}
	
	while(!file.eof()){
		region * rg = new region ();
		if (count ==0 )
			file >> token;
			rg->nOfRegion = atoi(token.c_str());
			count = 1;
		if (count == 1){
			file >> token;
			rg->min_x = atoi(token.c_str());
			rg->min_x = (int)(rg->min_x/grid->side); // CHECK: the cast was (float)
			count=2;
		}
		if (count==2){
			file >> token;
			rg->min_y = atoi(token.c_str());
			rg->min_y = (int)(rg->min_y/grid->side); // CHECK: the cast was (float)
			count=3;
		}
		if (count==3){
			file >> token;
			rg->max_x = atoi(token.c_str());
			rg->max_x = (int)(rg->max_x/grid->side); // CHECK: the cast was (float)
			count=3;
		}
		if (count==3){
			file >> token;
			rg->max_y = atoi(token.c_str());
			rg->max_y = (int)(rg->max_y/grid->side); // CHECK: the cast was (float)
			count=0;
		}
		this->regions.push_back(rg);	
	}
	
	for (vector <region *>::iterator it = this->regions.begin(); it != this->regions.end(); it++){
		for (int i = (*it)->min_x; i <= (*it)->max_x; i++){
			for (int j = (*it)->min_y; j <= (*it)->max_y; j++){
				std::map< std::pair < int ,int > , Cell* >::iterator ite_find=grid->find (std::pair< int, int > ( i, j ));
				(*ite_find).second->region = (*it)->nOfRegion;
			}
		}
	}
	
	file.close();
}

void db::print_regions(char * filename, int n_execution, float side){
	ofstream regions_out_stream;
	if (n_execution==0)
		regions_out_stream.open(filename, ios::out);
	else 
		regions_out_stream.open(filename, ios::app);
	for (vector<region *>::iterator reg = this->regions.begin(); reg!=this->regions.end(); reg++) {
		(*reg)->bound();
		regions_out_stream << (*reg)->nOfRegion << " ";
		regions_out_stream << (*reg)->min_x*side << " " << (*reg)->min_y*side << " ";
		regions_out_stream << (*reg)->max_x*side << " " << (*reg)->max_y*side << endl;
	}
		
	regions_out_stream.close();
}

void db::print_regions_jump(string filename, Grid * grid, int threshold){
	ofstream regions_s;
	
	regions_s.open (filename.c_str(), ios::out);

	for (vector <region *>::iterator it_reg = this->regions.begin(); it_reg != this->regions.end(); it_reg++){
		if (!(*it_reg)->cuted){
			//(*it_reg)->bound(grid->side);
			regions_s << "POLYGON ((" << (*it_reg)->min_x* grid->side << " " << (*it_reg)->min_y * grid->side << ", "
			<< (*it_reg)->min_x * grid->side << " " << ((*it_reg)->max_y*grid->side) + grid->side << ", " 
			<< ((*it_reg)->max_x * grid->side) + grid->side << " " << ((*it_reg)->max_y*grid->side) + grid->side << ", "
			<< ((*it_reg)->max_x * grid->side) + grid->side << " " << ((*it_reg)->min_y*grid->side) << ", "
			<< (*it_reg)->min_x * grid->side << " " << (*it_reg)->min_y * grid->side << "))\n\n";
		}
	}
	regions_s.close();


}

class x_greater_than 
{ 
public:
	bool operator ()(const point * a, const point * b) 
	{ 
		return ((a->x<=b->x)); 
	};
};

class x_less_than 
{ 
public:
	bool operator ()(const point * a, const point * b) 
	{ 
		return ((a->x>b->x)); 
	};
};



Trajectory::Trajectory(){
}

void Trajectory::find_cells_horizontal(point * p, point * next_p, Grid * grid, float epsilon){
	
	point * p_start = new point( p->x, p->y);
	point * p_end = new point( next_p->x, next_p->y);
	
	//index for find cells
	int index_x_start;
	int index_x_end;
	//int index_y = grid->getCurCell(p->x, p->y)->getY_Index();
	int back=0;
	
	//variables for menaging time information
	float c_time = 0;
	float time = next_p->time - p->time;
	float side = grid->getSide();
	float speed=sqrt (pow((p->x - next_p->x), (float) 2) + pow ((p->y - next_p->y), (float)2))/time;
	
	
	if (p_start->x > p_end->x){ //horizontal line with the first point is to left of the end point
		p_end->set(p->x, p->y);
		p_start->set(next_p->x, next_p->y);
		back = 1;
	}
	if (!back){
		index_x_start = grid->getCurCell(p_start->x, p_start->y)->getX_Index();
		index_x_end = grid->getCurCell(p_end->x, p_end->y)->getX_Index();
	}
	else {
		index_x_start = grid->getCurCell(p_start->x, p_start->y)->getX_Index();
		index_x_end = grid->getCurCell(p_end->x, p_end->y)->getX_Index();
	}

	for (index_x_start++; index_x_start <= index_x_end; index_x_start++){
		point * n_point = new point ();
		n_point->set(index_x_start*side, p->y);
		if (!back)
			n_point->time=0;
		else
			n_point->time=1;
		//this->sequence.push_back(grid->getCell(index_x_start, index_y));
		//grid->getCell(index_x_start, index_y)->noftrajectories=1;
		this->sequence.push_back(n_point->set_bound(epsilon, grid));
		c_time = p->time + sqrt(pow((n_point->x - p->x), (float)2) + pow((n_point->y - p->y), (float)2))/speed;
		this->time_stamp.push_back(c_time);
		delete n_point;
	}
	
	delete p_start;
	delete p_end;

}

void Trajectory::find_cells_vertical(point * p, point * next_p, Grid * grid, float epsilon){

	point * p_start = new point( p->x, p->y);
	point * p_end = new point( next_p->x, next_p->y);
	
	//index for find cells
	int index_y_start;
	int index_y_end;
	//int index_x = grid->getCurCell(p->x, p->y)->getX_Index();
	int back = 0;
	//variables for menaging time information
	float c_time = 0;
	float time = next_p->time - p->time;
	float side = grid->getSide();
	float speed=sqrt (pow((p->x - next_p->x), (float) 2) + pow ((p->y - next_p->y), (float)2))/time;
	
	if (p_start->y > p_end->y){ //vertical line with the first point is to up of the end point
		p_end->set(p->x, p->y);
		p_start->set(next_p->x, next_p->y);
		back = 1;
	}
	
	if (!back){
		index_y_start = grid->getCurCell(p_start->x, p_start->y)->getY_Index();
		index_y_end = grid->getCurCell(p_end->x, p_end->y)->getY_Index();
	}
	else {
		index_y_start = grid->getCurCell(p_start->x, p_start->y)->getY_Index();
		index_y_end = grid->getCurCell(p_end->x, p_end->y)->getY_Index();
	}
	for (index_y_start++; index_y_start <= index_y_end; index_y_start++){
		point * n_point = new point ();
		n_point->set(p->x, index_y_start*side);
		//this->sequence.push_back(grid->getCell(index_x, index_y_start));
		//grid->getCell(index_x, index_y_start)->noftrajectories=1;
		if (!back)
			n_point->time=0;
		else
			n_point->time=2;
		this->sequence.push_back(n_point->set_bound(epsilon, grid));
		c_time = p->time + sqrt(pow((n_point->x - p->x), (float)2) + pow((n_point->y - p->y), (float)2))/speed;
		this->time_stamp.push_back(c_time);
		delete n_point;
	}
	
	delete p_start;
	delete p_end;
}

void Trajectory::find_cells_pos_inc(point * p, point * next_p, Grid * grid, float epsilon, int directional){

	point * p_start = new point( p->x, p->y);
	point * p_end = new point( next_p->x, next_p->y);
	
	//index for find cells
	int index_y_start;
	int index_y_end;
	int index_x_start, index_x_end;
	//variables for menaging time information
	float c_time = 0;
	float time = next_p->time - p->time;
	float side = grid->getSide();
	float speed=sqrt (pow((p->x - next_p->x), (float) 2) + pow ((p->y - next_p->y), (float)2))/time;
	float x,y;
	 
	vector <point *> tmp_points;
	
		
	index_y_start = grid->getCurCell(p_start->x, p_start->y)->getY_Index();
	index_y_end = grid->getCurCell(p_end->x, p_end->y)->getY_Index();

	index_x_start = grid->getCurCell(p_start->x, p_start->y)->getX_Index();
	index_x_end = grid->getCurCell(p_end->x, p_end->y)->getX_Index();

	//we found all intersect points before with x axis than on y axis
	for (index_x_start++; index_x_start <= index_x_end; index_x_start++){
		y= (p->m * (index_x_start*side)) + p->q;
		point *n_point = new point ();
		n_point->set((index_x_start*side), y);
		n_point->time=0;
		tmp_points.push_back(n_point);
		//grid->getCurCell(n_point->x, n_point->y)->noftrajectories=1;

	}	
	for (index_y_start++; index_y_start<=index_y_end; index_y_start++){
		x= ((index_y_start*side)-p->q)/p->m;
		point *n_point = new point ();
		n_point->set(x, index_y_start*side);
		if (directional)
			n_point->time=1;
		else 
			n_point->time=0;
		tmp_points.push_back(n_point);
		//grid->getCurCell(n_point->x, n_point->y)->noftrajectories=1;
	}
	
	//we sort the found points depending on decreasing variable 
	std::sort(tmp_points.begin(), tmp_points.end(), x_greater_than());
	//set the time values
	for (vector<point *>::iterator pp = tmp_points.begin(); pp!=tmp_points.end(); pp++){
		c_time = p->time + sqrt(pow(((*pp)->x - p->x), (float)2) + pow(((*pp)->y - p->y), (float)2))/speed;
		this->time_stamp.push_back(c_time);
		//this->sequence.push_back(grid->getCurCell((*pp)->x, (*pp)->y));
		if (directional)
			this->sequence.push_back((*pp)->directional_bound(epsilon, grid));
		else
			this->sequence.push_back((*pp)->set_bound(epsilon, grid));
		delete *pp;
	}
	
	delete p_start;
	delete p_end;
}


void Trajectory::find_cells_neg_inc(point * p, point * next_p, Grid * grid, float epsilon, int directional){

	point * p_start = new point( p->x, p->y);
	point * p_end = new point( next_p->x, next_p->y);
		
	//index for find cells
	int index_y_start;
	int index_y_end;
	int index_x_start, index_x_end;
	//variables for menaging time information
	float c_time = 0;
	float time = next_p->time - p->time;
	float side = grid->getSide();
	float speed=sqrt (pow((p->x - next_p->x), (float) 2) + pow ((p->y - next_p->y), (float)2))/time;
	float x,y;
	 
	
	vector <point *> tmp_points;
	
	index_y_start = grid->getCurCell(p_start->x, p_start->y)->getY_Index();
	index_y_end = grid->getCurCell(p_end->x, p_end->y)->getY_Index();

	index_x_start = grid->getCurCell(p_start->x, p_start->y)->getX_Index();
	index_x_end = grid->getCurCell(p_end->x, p_end->y)->getX_Index();

	//we found all intersect points before with x axis than on y axis
	for ( ; index_x_start > index_x_end; index_x_start--){
		y= (p->m * ((index_x_start)*side)) + p->q;
		point *n_point = new point ();
		n_point->set((index_x_start*side), y);
		if (directional)
			n_point->time=4;
		else
			n_point->time = 1; //the point variable time is used as flag for remembering that the cell crossed is changed of -side 
		tmp_points.push_back(n_point);
		//grid->getCurCell(n_point->x-side, n_point->y)->noftrajectories=1;
	}
	for (index_y_start++; index_y_start<=index_y_end; index_y_start++){
		x= (((index_y_start)*side)-p->q)/p->m;
		point *n_point = new point ();
		n_point->set(x, index_y_start*side);
		if (directional)
			n_point->time=5;
		else
			n_point->time=0;
		tmp_points.push_back(n_point);
		//grid->getCurCell(n_point->x, n_point->y)->noftrajectories=1;
	}
	
	//we sort the found points depending on decreasing variable 
	std::sort(tmp_points.begin(), tmp_points.end(), x_less_than());	
	//set the time values and push back the cell
	for (vector<point *>::iterator pp = tmp_points.begin(); pp!=tmp_points.end(); pp++){
		c_time = p->time + sqrt(pow(((*pp)->x - p->x), (float)2) + pow(((*pp)->y - p->y), (float)2))/speed;
		this->time_stamp.push_back(c_time);
			//this->sequence.push_back(grid->getCurCell((*pp)->x-side, (*pp)->y));
		if (directional)
			this->sequence.push_back((*pp)->directional_bound(epsilon, grid));
		else
			this->sequence.push_back((*pp)->set_bound(epsilon, grid));
			//this->sequence.push_back(grid->getCurCell((*pp)->x, (*pp)->y));
		delete *pp;
	}
	
	delete p_start;
	delete p_end;
}

void Trajectory::find_cells_pos_dec(point * p, point * next_p, Grid * grid, float epsilon, int directional){

	point * p_start = new point( p->x, p->y);
	point * p_end = new point( next_p->x, next_p->y);
		
	//index for find cells
	int index_y_start;
	int index_y_end;
	int index_x_start, index_x_end;
	//variables for menaging time information
	float c_time = 0;
	float time = next_p->time - p->time;
	float side = grid->getSide();
	float speed=sqrt (pow((p->x - next_p->x), (float) 2) + pow ((p->y - next_p->y), (float)2))/time;
	float x,y;
	
	vector <point *> tmp_points;
	
	index_y_start = grid->getCurCell(p_start->x, p_start->y)->getY_Index();
	index_y_end = grid->getCurCell(p_end->x, p_end->y)->getY_Index();

	index_x_start = grid->getCurCell(p_start->x, p_start->y)->getX_Index();
	index_x_end = grid->getCurCell(p_end->x, p_end->y)->getX_Index();
	//we found all intersect points before with x axis than on y axis
	for ( ; index_x_start > index_x_end; index_x_start--){
		y= (p->m * ((index_x_start)*side)) + p->q;
		point *n_point = new point ();
		n_point->set((index_x_start*side), y);
		if (directional)
			n_point->time = 2;
		else 
			n_point->time=0;
		tmp_points.push_back(n_point);
		//grid->getCurCell(n_point->x-side, n_point->y)->noftrajectories=1;
		
	}
	for ( ; index_y_start>index_y_end; index_y_start--){
		x= (((index_y_start)*side)-p->q)/p->m;
		point *n_point = new point ();
		n_point->set(x, index_y_start*side);
		if (directional)
			n_point->time=3;
		else
			n_point->time = 2;
		tmp_points.push_back(n_point);
		//grid->getCurCell(n_point->x, n_point->y-side)->noftrajectories=1;
	}
	
	//we sort the found points depending on decreasing variable 
	std::sort(tmp_points.begin(), tmp_points.end(), x_less_than());
	//set the time values and push back the cell
	for (vector<point *>::iterator pp = tmp_points.begin(); pp!=tmp_points.end(); pp++){
		c_time = p->time + sqrt(pow(((*pp)->x - p->x), (float)2) + pow(((*pp)->y - p->y), (float)2))/speed;
		this->time_stamp.push_back(c_time);
		//this->sequence.push_back(grid->getCurCell((*pp)->x-side, (*pp)->y));
		if (directional)
			this->sequence.push_back((*pp)->directional_bound(epsilon, grid));
		else
			this->sequence.push_back((*pp)->set_bound(epsilon, grid));
		//this->sequence.push_back(grid->getCurCell((*pp)->x, (*pp)->y-side));
		delete *pp;
	}
	
	delete p_start;
	delete p_end;

}
void Trajectory::find_cells_neg_dec(point * p, point * next_p, Grid * grid, float epsilon, int directional){

	point * p_start = new point( p->x, p->y);
	point * p_end = new point( next_p->x, next_p->y);
	
	//index for find cells
	int index_y_start;
	int index_y_end;
	int index_x_start, index_x_end;
	//variables for menaging time information
	float c_time = 0;
	float time = next_p->time - p->time;
	float side = grid->getSide();
	float speed=sqrt (pow((p->x - next_p->x), (float) 2) + pow ((p->y - next_p->y), (float)2))/time;
	float x,y; 
	
	vector <point *> tmp_points;

	index_y_start = grid->getCurCell(p_start->x, p_start->y)->getY_Index();
	index_y_end = grid->getCurCell(p_end->x, p_end->y)->getY_Index();

	index_x_start = grid->getCurCell(p_start->x, p_start->y)->getX_Index();
	index_x_end = grid->getCurCell(p_end->x, p_end->y)->getX_Index();
	
	//we found all intersect points before with x axis than on y axis
	for (index_x_start++; index_x_start <= index_x_end; index_x_start++){
		y= (p->m * ((index_x_start)*side)) + p->q;
		point *n_point = new point ();
		n_point->set((index_x_start*side), y);
		if (directional)
			n_point->time=6;
		else
			n_point->time=0;
		tmp_points.push_back(n_point);
		//grid->getCurCell(n_point->x, n_point->y)->noftrajectories=1;
	}
	for ( ; index_y_start>index_y_end; index_y_start--){
		x= (((index_y_start)*side)-p->q)/p->m;
		point *n_point = new point ();
		n_point->set(x, index_y_start*side);
		if (directional)
			n_point->time=7;
		else
			n_point->time = 2;
		tmp_points.push_back(n_point);
		//grid->getCurCell(n_point->x, n_point->y-side)->noftrajectories=1;
	}
	
	//we sort the found points depending on decreasing variable 
	std::sort(tmp_points.begin(), tmp_points.end(), x_greater_than());
	//set the time values and push back the cell
	for (vector<point *>::iterator pp = tmp_points.begin(); pp!=tmp_points.end(); pp++){
		c_time = p->time + sqrt(pow(((*pp)->x - p->x), (float)2) + pow(((*pp)->y - p->y), (float)2))/speed;
		this->time_stamp.push_back(c_time);
			//this->sequence.push_back(grid->getCurCell((*pp)->x, (*pp)->y-side));
		if (directional)
			this->sequence.push_back((*pp)->directional_bound(epsilon, grid));
		else
			this->sequence.push_back((*pp)->set_bound(epsilon, grid));
			//this->sequence.push_back(grid->getCurCell((*pp)->x, (*pp)->y));
		delete *pp;
	}
	delete p_start;
	delete p_end;
}

vector <Cell *> Trajectory::locations(Grid * grid){
	vector <Cell *> tmp;
	
	for (vector <bound *>::iterator bd = this->sequence.begin(); bd != this->sequence.end(); bd++){
		for (int i = (*bd)->min_x_index; i<=(*bd)->max_x_index; i++){
			for (int j = (*bd)->min_y_index; j<=(*bd)->max_y_index; j++){
				if (grid->getCell(i,j)->noftrajectories==0){
					tmp.push_back(grid->getCell(i,j));
					grid->getCell(i, j)->noftrajectories=1;
				}
			}
		}
	}
	
	return tmp;
}

void Trajectory::translation(TSequence * tseq, Grid * grid, TProjection * tp){
	
	bound * previousRegion = new bound ();
	vector <Cell *> bound_cell;
	Cell * tmp_cell = new Cell();
	
	vector <int> tmp_previous_region;
	tseq->clear();
	TElement * telem;
	map<int,int> local_support;
	int item_count=0;
	int cycle = 0;
	vector <bound *>::iterator bd;
	if (!this->sequence.empty())
	  bd = this->sequence.begin();
	for (vector<float>::iterator t = this->time_stamp.begin(); t != this->time_stamp.end(); t++){
		telem = checked_new(TElement);
		tmp_previous_region.clear();
		
		bound_cell = (*bd)->bounding_cells(grid);
		for ( vector <Cell *>::iterator cell = bound_cell.begin(); cell!= bound_cell.end(); cell++) {
			telem->time = *t;
			tmp_cell = grid->getCell((*cell)->X_Index,(*cell)->Y_Index);

			if (!find_region(previousRegion, tmp_cell->region, grid, cycle) && tmp_cell->region != -1){ 
				telem->push_back(tmp_cell->region);
				//telem->time = *t;
				tmp_previous_region.push_back(tmp_cell->region);
			}
		}
		bd++;
		cycle++;
		previousRegion = *(bd-1);

		for(TElement::iterator it=telem->begin(); it!=telem->end(); it++, item_count++)
		{
			local_support[*it]++;
		};
	
		if (!telem->empty()){
//			cout << "insert telem\n";
			tseq->push_back(telem);
		}
		
		//previousRegion = tmp_previous_region;
	}
	int seq_len=(int)tseq->size();
	mystat.avg_seq_len+=(float)seq_len;
	if(seq_len<=MAX_SEQUENCE_LEN  && seq_len>1) // Update countings
	{
		for(map<int,int>::iterator pt=local_support.begin(); pt!=local_support.end(); pt++)
		{
			tp->item_freq_extend[pt->first]++;
		}
	}
	
	vector<EntryPoint>::iterator a;
	int i;
	
	for(a = tseq->annotations->begin(); a != tseq->annotations->end() && (*a).entrypoint.idx==-1; a++){
		(*a).entrypoint.position = 0;
	}; 
	
	for (i=0 ; a != tseq->annotations->end() && i < (int)tseq->size(); ) {
		float ta = a->times.back();
		float ts = tseq->at(i)->time;
		if (ts > ta) {
			(*a).entrypoint.idx = i-1;
			if (i==0) {
				(*a).entrypoint.position = 0;
			}
			else {
				(*a).entrypoint.position = ((int)tseq->at(i-1)->size())-1;
			}
			a++;
		}
		else {
			i++;
		}
		
	}
	
	for ( ; a != tseq->annotations->end(); a++) {
		(*a).entrypoint.idx = -2;
	}
	
	//tseq->annotations->erase(a, tseq->annotations->end());
	/*
		//cout << "PER LA SEQUENZA: " << tseq->id << endl;
	for ( vector<EntryPoint>::iterator a = tseq->annotations->begin(); a != tseq->annotations->end(); a++) {
		//cout << (*a).entrypoint.idx <<  endl;
		float t = a->times.back();
		int i = 0;
		if (tseq->size() != 0) {
			//cout << tseq->size() << " "  <<  *t << endl;
			while ((t >= tseq->at(i)->time) && (i < tseq->size()-1)) {
				i++;
				//cout << "RICERCA TEMPO: " << i << " " << tseq->at(i)->time << endl;
			}
			if (i == 0)
				(*a).entrypoint.idx = -1;
			else {
				(*a).entrypoint.idx = i-1;
			}
		}
	}
	*/
}

void Trajectory::print(){

	ofstream trajectory_file;
	int last = (int) this->T_points.size();
	trajectory_file.open("Line.wkt", ios::app);
	trajectory_file << "LINESTRING (";
	
	for (vector<point *>::iterator p = this->T_points.begin(); p != this->T_points.end()-1;p++){
			trajectory_file << (*p)->x << " " << (*p)->y << ", ";
	}
	trajectory_file << this->T_points.at(last-1)->x << " " << this->T_points.at(last-1)->y << ")\n\n";
	trajectory_file.close();
}


bool find_region (bound *previous_bound, int region, Grid * grid, int cycle){
	
	vector <Cell *> previous_regions;
	Cell * tmp_cell;
	if (cycle){
		for (int i = previous_bound->min_x_index; i<=previous_bound->max_x_index; i++){
			for (int j = previous_bound->min_y_index; j<=previous_bound->max_y_index; j++){
				previous_regions.push_back(grid->getCell(i,j));
			}
		}
		
		for (vector<Cell *>::iterator it = previous_regions.begin(); it!= previous_regions.end(); it++){
			tmp_cell = grid->getCell((*it)->X_Index, (*it)->Y_Index);
			
			if (tmp_cell->region == region){
				//it = previous_regions.end()-1;
				return true;
			}
		}
	}
	return false;
}




