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
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <math.h>
#include <algorithm>
#include "regions.h"
#define DEBUG(x)  
// #define DEBUG(x) x

using namespace std;

region::region (vector <Cell *> v_region){
	for (vector<Cell *>::iterator it_cell = v_region.begin(); it_cell != v_region.end(); it_cell++) {
		this->push_back(*it_cell);
	}
	this->cuted = 0;
}

region::region(){
	nOfRegion = -1;
	cuted = 0;
}

//a region is accepted if is different to the other at least difference_tolerance percentage
bool region::compare(region* last, float difference_tolerance){
	int count_cell = 0;
	for (vector<Cell *>::iterator cx = this->begin(); cx != this->end(); cx++) {
		if (last->contains(*cx))
			count_cell++;
	}
	if (count_cell <= last->size()*difference_tolerance){
		return 0;
	}
	else {
		return 1;
	}

}
//if the minimun distance threshold between regions is overflow the region is accepted otherwise is cancelled
bool region::distance(region* last, float min_distance, float side){
	
	float x1;
	float x2;
	float y1;
	float y2;
	
	float distance=0;
	
	this->bound();
	
	if (this->min_x > last->max_x){
		x1 = last->max_x * side;
		x2 = this->min_x * side;
		//right
	}
	else {
		if (this->max_x < last->min_x){
			x1 = last->min_x * side;
			x2 = this->max_x * side;
			//left
		}
		else {
			x1 = 0;
			x2 = 0;
			//overlaping
		}
	}
	
	if (this->min_y > last->max_y){
		y1 = last->max_y * side;
		y2 = this->min_y * side;
		//above
	}
	else {
		if (this->max_y < last->min_y){
			y1 = last->min_y * side;
			y2 = this->max_y * side;
			//left
		}
		else {
			y1 = 0;
			y2 = 0;
			//overlaping
		}
	}
	distance = sqrtf (((x1-x2)*(x1-x2))+ ((y1-y2)*(y1-y2)));

	if (distance <= min_distance) {
		return 1;
	}
	else{
		return 0;
	}
}

bool region::contains(Cell * c){
	bool test=0;
	for (vector <Cell *>::iterator cx = this->begin(); cx!=this->end();cx++){
		if (c->X_Index == (*cx)->X_Index && c->Y_Index == (*cx)->Y_Index)
			test=1;
	}
	return test;
}

vector <Cell *> region::sort(){

	int i = 0;
	int change = 0;
	int count = 0;
	while (i != (int)this->size()) {
		count = i;
		while(count < (int)this->size()){
			if (this->at(i)->Y_Index > this->at(count)->Y_Index) {
				  
				Cell *swap_p = this->at(i);
				this->at(i) = this->at(count);
				this->at(count) = swap_p;
				change = 1;
				count = ((int)this->size())+1;
			}
			else 
				count++;
				change = 0;
		}
		if (change || count == (int)this->size()){
				i++;
				change = 0;
		}
	}
	i = 0;
	count = 0;
	
	while (i != (int)this->size()){
		count  = i;
		this->at(i)->checked = 0;
		while(count < (int)this->size()){
			if (this->at(i)->Y_Index == this->at(count)->Y_Index && this->at(i)->X_Index > this->at(count)->X_Index) {
					Cell *swap_cell = this->at(i);
					this->at(i) = this->at(count);
					this->at(count) = swap_cell;
					change = 1;
					count = ((int)this->size())+1;
			}
			else 
				count++;
				change = 0;
		}
		
		if (change || count == (int)this->size()){
				i++;
				change = 0;
		}
	
	}
	return *this;	

}


void region::print (int threshold){
	
	ofstream jump;
	jump.open("regioni.jml", ios::app);


	
	for (vector<Cell *>::iterator it_cell = this->begin(); it_cell != this->end(); it_cell++){
		if ((*it_cell)->noftrajectories >= threshold){
			float side = (*it_cell)->getSide();
			jump << "<feature>\n <geometry>\n   <gml:Polygon>\n    <gml:outerBoundaryIs>\n  <gml:LinearRing>\n      <gml:coordinates>";
			jump << (float)(*it_cell)->X_Index* side << "," << (*it_cell)->Y_Index * side << "\n               ";
			jump << (*it_cell)->X_Index * side << "," << ((*it_cell)->Y_Index*side) + side << "\n              " 
			<< ((*it_cell)->X_Index * side)+ side  << "," << ((*it_cell)->Y_Index*side) + side << "\n          "
			<< ((*it_cell)->X_Index * side)+ side  << "," << ((*it_cell)->Y_Index*side) << "\n                 "
			<< (*it_cell)->X_Index * side << "," << (*it_cell)->Y_Index * side << " </gml:coordinates>\n";
			
			jump <<"</gml:LinearRing>\n         </gml:outerBoundaryIs>\n    </gml:Polygon>\n    </geometry>\n   <property name=\"Region\">" << nOfRegion << "</property>\n </feature>\n\n";
		
		}
	}

	jump.close();

}

void region::bound(){
	min_y = 1000000;
	min_x = 1000000;
	max_x = 0;
	max_y = 0;
	for (vector<Cell *>::iterator iter = this->begin(); iter != this->end(); iter++){
		if ((*iter)->X_Index > max_x)
			max_x = (*iter)->X_Index;
		if ((*iter)->X_Index < min_x)
			min_x = (*iter)->X_Index;
		if ((*iter)->Y_Index < min_y)
			min_y = (*iter)->Y_Index;
		if ((*iter)->Y_Index > max_y)
			max_y = (*iter)->Y_Index;
	}
	//max_x = max_x + side;
	//max_y = max_y + side;	
}


void region::bound(float side){
	min_y = 1000000;
	min_x = 1000000;
	max_x = 0;
	max_y = 0;
	for (vector<Cell *>::iterator iter = this->begin(); iter != this->end(); iter++){
		if ((*iter)->X_Index > max_x)
			max_x = (*iter)->X_Index;
		if ((*iter)->X_Index < min_x)
			min_x = (*iter)->X_Index;
		if ((*iter)->Y_Index < min_y)
			min_y = (*iter)->Y_Index;
		if ((*iter)->Y_Index > max_y)
			max_y = (*iter)->Y_Index;
	}
	max_x = max_x + 1;
	max_y = max_y + 1;	
}

void region::checkDensity(){
	for (vector<Cell *>::iterator iter = this->begin(); iter != this->end(); iter++){
		density = density + (*iter)->noftrajectories;
	}
}






