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
#include "grid.h"
#include "regions.h"
#include "Tsequences.h"

#pragma once

using namespace std;

class bound {
  public:
  
  int min_x_index, min_y_index, max_x_index, max_y_index, direction; //the direction could be value since 1..6
  
  bound();
  vector <Cell *> bounding_cells(Grid * grid);
  void bounding_cells(Grid * grid, vector <Cell *> returnSet);
};

class point {
  public:
	float x,y,m,q, time;
	point ();
	point (float _x, float _y);
	void interpolation(point * p);
	void set(float _x, float _y);
	bound * set_bound_first(float epsilon, Grid * grid);
	bound * set_bound(float epsilon, Grid * grid);   //starting from a point we found the touched cells with a square of size epsilon	
	bound * directional_bound(float epsilon, Grid * grid); //based on direction of the generation point
};

class Trajectory {
 	
  public:
	vector <point *> T_points; //points (the lenght of this vector == snapshot)
	int id; //id
	int snapshot; //n. of snapshots
	vector <float> time_stamp; //vector time
	//vector <Cell *> sequence; //sequence of crossed cells
	vector <bound *> sequence;
	//methods
	Trajectory();
	void find_cells_vertical(point * p, point * next_p, Grid * grid, float epsilon); //find cell for a vertical line
	void find_cells_horizontal(point * p, point * next_p, Grid * grid, float epsilon); //find cell for a horizontal line
	void find_cells_pos_inc(point * p, point * next_p, Grid * grid, float epsilon, int directional);
	void find_cells_neg_inc(point * p, point * next_p, Grid * grid, float epsilon, int directional);
	void find_cells_neg_dec(point * p, point * next_p, Grid * grid, float epsilon, int directional);
	void find_cells_pos_dec(point * p, point * next_p, Grid * grid, float epsilon, int directional);
	vector <Cell *> locations(Grid * grid);
	void translation(TSequence * tseq, Grid * grid, TProjection * tp);
	void print();
};


class db: public vector <Trajectory *>  {
  public:
  
	vector <Trajectory *> tmp_traj;
	vector <region *> regions;
	db();
	float max_x, min_x, max_y, min_y, side_grid;
	db(char * file);
	void interpolation(Grid * grid, float epsilon, int threshold, int directional);
	TProjection * translation(Grid * grid);
	void set_density(Grid * grid, float epsilon); 
	void load_regions(char * file, Grid * grid);
	void print_regions(char * filename, int n_execution, float side);
	void print_regions_jump(string filename, Grid * grid, int threshold);
};

bool find_region (bound * previous_bound, int region, Grid * grid, int cycle);


