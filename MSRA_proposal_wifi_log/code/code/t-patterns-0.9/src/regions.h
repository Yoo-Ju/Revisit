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

#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
//#include "grid.h"
#include "cell.h"

#pragma once
using namespace std;

class region : public vector <Cell *> {
public:
	
	int max_x; //max index on X
	int max_y; //max index on Y
	int min_x; //min index on y
	int min_y; //min index on y
	int density;
	int nOfRegion; //progressive number
	int refreshed; 
	int level;
	int owner;
	int cuted;
	
	region (vector<Cell *> v_region);
	region();
	//vector<region *> region::divide_region(Grid * grid, int threshold, int first_new_region, bool region_cells_only);
	vector <Cell *> sort();
	bool compare(region * last, float difference_tolerance);
	bool contains(Cell * c);
	bool distance(region* last, float min_distance, float side);
	void print (int threshold);
	void bound();
	void bound(float side);
	void checkDensity();
	
};

//vector <Cell *> compute_regions (Cell * startCell, Grid * Grid, int threshold, vector <Cell *> v_region, int count);
//vector <Cell *> rectangle (Cell * startCell, Grid * grid, int threshold, vector <Cell *> v_region, int count, int direction);
//vector <Cell *> expands(vector <Cell *> sub_regions, int direction, Grid * grid, int threshold);


