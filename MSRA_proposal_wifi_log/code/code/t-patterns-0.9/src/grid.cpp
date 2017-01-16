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
#include "grid.h"
//#include "regions.h"
#include <algorithm>
using namespace std;

/*
Grid::Grid(const int sizeX, const int sizeY, float lt) {
	insertCell(sizeX,sizeY, lt);
	numOfCell_X=sizeX;
	numOfCell_Z=sizeY;
	side = lt;
}

Grid::Grid(float side, float min_x, float min_y, float max_x, float max_y){

	numOfCell_X = (int)((max_x+side)/side) + 2;
	numOfCell_Z = (int)((max_y+side)/side) + 2;
	side = side;
	insertCell(min_x, min_y);
}

Grid::~Grid() {
	vector<Cell*>::iterator iterCell;
	for(iterCell=grid.begin();iterCell!=grid.end();++iterCell) {
       delete (*iterCell);
	}
	
}

void Grid::insertCell(float min_x, float min_y){
		for(int iX=0; iX<numOfCell_X; iX++) {
			for(int jZ=0; jZ<numOfCell_Z; jZ++) {
				Cell *tmpCell = new Cell(iX,jZ,side);
				grid.push_back(tmpCell);
		}
	}
}

void Grid::insertCell(int numCellX, int numCellY, float side) {
	for(int iX=0; iX<numCellX; iX++) {
		for(int jZ=0; jZ<numCellY; jZ++) {
			Cell *tmpCell = new Cell(iX,jZ,side);
			grid.push_back(tmpCell);
		}
	}
}

void Grid::clear() {	
	vector<Cell*>::iterator iterCell;
	for(iterCell=grid.begin();iterCell!=grid.end();++iterCell) {
       delete (*iterCell);
	}
   	
	grid.clear();
	numOfCell_X=0;
	numOfCell_Z=0;
}

int Grid::getNumCellX() {
	return numOfCell_X;
}

int Grid::getNumCellY() {
	return numOfCell_Z;
}

int Grid::getMaxX_Index(vector<Cell *> vectorCell) {
	Cell *maxXInd = vectorCell.at(0);
	for(int i=0;i<vectorCell.size();i++) {
		if(vectorCell.at(i)->getX_Index() > maxXInd->getX_Index()) {
			maxXInd = vectorCell.at(i);
		}
	}
	return maxXInd->getX_Index();    
}

int Grid::getMaxY_Index(vector<Cell *> vectorCell) {
	Cell *maxZInd = vectorCell.at(0);
	for(int i=0;i<vectorCell.size();i++) {
		if(vectorCell.at(i)->getY_Index() > maxZInd->getY_Index()) {
			maxZInd = vectorCell.at(i);
		}
    }
	return maxZInd->getY_Index();    
}

int Grid::getMinX_Index(vector<Cell *> vectorCell) {
	Cell *minXInd = vectorCell.at(0);
	for(int i=0;i<vectorCell.size();i++) {
		if(vectorCell.at(i)->getX_Index() < minXInd->getX_Index()) {
			minXInd = vectorCell.at(i);
		}
	}
	return minXInd->getX_Index();    
}

int Grid::getMinY_Index(vector<Cell *> vectorCell) {
	Cell *minZInd = vectorCell.at(0);
	for(int i=0;i<vectorCell.size();i++) {
		if(vectorCell.at(i)->getY_Index() <minZInd->getY_Index()){
			minZInd = vectorCell.at(i);
		}
    }
	return minZInd->getY_Index();    
}

float Grid::getX_Size() {
	return (side*numOfCell_X);
}

float Grid::getY_Size() {
	return (side*numOfCell_Z);
}

float Grid::getSide() {
	return side;
}

Cell * Grid::getCell(int X_Index, int Y_Index) {
	return grid[(X_Index*numOfCell_Z)+Y_Index];
}

Cell * Grid::getCurCell(float curPosX, float curPosZ) {
	if (curPosX <  0)
		curPosX = 0;
	if (curPosX >= this->numOfCell_X * side)
		curPosX = (this->numOfCell_X * side)-100;

	if (curPosZ <  0)
		curPosZ = 0;
	if (curPosZ >= this->numOfCell_Z * side)
		curPosZ = (this->numOfCell_Z * side)-100;

	int X_Ind = curPosX/getSide();
	int Z_Ind = curPosZ/getSide();
	//if (X_Ind > numOfCell_X || Z_Ind > numOfCell_Z)
		//exit(1);
	return getCell(X_Ind,Z_Ind);
}

void Grid::compute_density(vector <Cell *> distinct_cells){

	for (vector <Cell *>::iterator cx = distinct_cells.begin(); cx != distinct_cells.end(); cx++){
		getCell((*cx)->X_Index, (*cx)->Y_Index)->noftrajectories++;
	}
}

void Grid::compute_density(Grid * supportGrid){

	for (vector <Cell *>::iterator cx = supportGrid->grid.begin(); cx != supportGrid->grid.end(); cx++){
		if ((*cx)->noftrajectories == 1){
			this->getCell((*cx)->X_Index, (*cx)->Y_Index)->noftrajectories++;
		}
	}
}


void Grid::reset(vector <Cell *> distinct_cells){

	for (vector <Cell *>::iterator cx = distinct_cells.begin(); cx != distinct_cells.end(); cx++){
		getCell((*cx)->X_Index, (*cx)->Y_Index)->noftrajectories = 0;
	}

}

void Grid::reset(){

	for (vector <Cell *>::iterator cx = this->grid.begin(); cx != this->grid.end(); cx++){
		(*cx)->noftrajectories = 0;
	}

}

void Grid::reset_values(){
	for (vector<Cell*>::iterator cx = this->grid.begin(); cx != this->grid.end(); cx++){
		(*cx)->noftrajectories = 0; //set to 0 the grid density
		(*cx)->region = -1; //no region
		(*cx)->checked = 0;
	}
}						
void Grid::print(){
	ofstream cellFile;
	cellFile.open ("cell2.wkt", ios::out);
	
	for (vector <Cell *>::iterator it_cell = this->grid.begin(); it_cell != this->grid.end(); it_cell++){
		
		if ((*it_cell)->noftrajectories == 1){
			cellFile << "POLYGON ((" << (*it_cell)->X_Index* this->side << " " << (*it_cell)->Y_Index * this->side << ", "
			<< (*it_cell)->X_Index * this->side << " " << ((*it_cell)->Y_Index*this->side) + this->side << ", " 
			<< ((*it_cell)->X_Index * this->side)+ this->side  << " " << ((*it_cell)->Y_Index*this->side) + this->side << ", "
			<< ((*it_cell)->X_Index * this->side)+ this->side  << " " << ((*it_cell)->Y_Index*this->side) << ", "
			<< (*it_cell)->X_Index * this->side << " " << (*it_cell)->Y_Index * this->side << "))\n\n";
		}
	}

	cellFile.close();
	cellFile.open ("cell.wkt", ios::out);
	
	for (vector <Cell *>::iterator it_cell = this->grid.begin(); it_cell != this->grid.end(); it_cell++){
		
		if ((*it_cell)->noftrajectories > 1){
			cellFile << "POLYGON ((" << (*it_cell)->X_Index* this->side << " " << (*it_cell)->Y_Index * this->side << ", "
			<< (*it_cell)->X_Index * this->side << " " << ((*it_cell)->Y_Index*this->side) + this->side << ", " 
			<< ((*it_cell)->X_Index * this->side)+ this->side  << " " << ((*it_cell)->Y_Index*this->side) + this->side << ", "
			<< ((*it_cell)->X_Index * this->side)+ this->side  << " " << ((*it_cell)->Y_Index*this->side) << ", "
			<< (*it_cell)->X_Index * this->side << " " << (*it_cell)->Y_Index * this->side << "))\n\n";
		}
	}

	cellFile.close();

}


void Grid::print_cell(int threshold){

	ofstream jump;
	jump.open("regioni.jml", ios::app);

	for (vector<Cell *>::iterator it_cell = grid.begin(); it_cell != grid.end(); it_cell++){
		if ((*it_cell)->noftrajectories > threshold){

			jump << "<feature>\n <geometry>\n   <gml:Polygon>\n    <gml:outerBoundaryIs>\n  <gml:LinearRing>\n      <gml:coordinates>";
			jump << (float)(*it_cell)->X_Index* side << "," << (*it_cell)->Y_Index * side << "\n               ";
			jump << (*it_cell)->X_Index * side << "," << ((*it_cell)->Y_Index*side) + side << "\n              " 
			<< ((*it_cell)->X_Index * side)+ side  << "," << ((*it_cell)->Y_Index*side) + side << "\n          "
			<< ((*it_cell)->X_Index * side)+ side  << "," << ((*it_cell)->Y_Index*side) << "\n                 "
			<< (*it_cell)->X_Index * side << "," << (*it_cell)->Y_Index * side << " </gml:coordinates>\n";
			
			jump <<"</gml:LinearRing>\n         </gml:outerBoundaryIs>\n    </gml:Polygon>\n    </geometry>\n   <property name=\"Region\">" << (*it_cell)->region << "</property>\n </feature>\n\n";
		}
	}

	  jump << "     </featureCollection>\n</JCSDataFile>";

	jump.close();
}
*/

// Needed to sort cells by decreasing density
class cell_density_greater_than 
{ 
public:
	bool operator ()(const Cell * a, const Cell * b) 
	{ 
		return ((a->noftrajectories<b->noftrajectories) || 
			   ((a->noftrajectories==b->noftrajectories) && ((a->X_Index<b->X_Index) || 
			   ((a->X_Index==b->X_Index)&& a->Y_Index<b->Y_Index)))); 
	};
};


void print_checked_matrix(Grid * grid)
{
	cerr << "Checked matrix (" << grid->numOfCell_X << "x" << grid->numOfCell_Y << "):\n";
	for(int i = 0; i<grid->numOfCell_X; i++)
	{
		for(int j = 0; j<grid->numOfCell_Y; j++)
			cerr << " " << grid->getCell(i,j)->checked;
		cerr << endl;
	}
}

// Partition region into rectangles:
//		- as square as possible
//		- no bounded size
//

vector<region *> Grid::popular_regions(int threshold, int first_new_region, bool region_cells_only){
// WARNING: cells in grid must be set to region=-1 and checked=0
	vector <Cell *> sub_region;
	vector <Cell *> sorted;
	vector <region *> regions;
	vector <region *> reg_hor;
	vector<Cell *> lines;
	region * dense_cells = new region();
	const int INREGION = -2;
	for (map<pair<int, int > , Cell*>::iterator cx = this->begin(); cx != this->end(); cx++){
		if ((*cx).second->noftrajectories >= threshold) 
			{
				dense_cells->push_back((*cx).second);
				(*cx).second->region=INREGION;
			}
	}
	// Higher density cells first
	std::sort(dense_cells->begin(), dense_cells->end(), cell_density_greater_than());
	// Reset "used" flag for all cells
	dense_cells->bound();
	int new_region=first_new_region;

	for (vector<Cell *>::iterator cx = dense_cells->begin(); cx != dense_cells->end(); cx++)
	{
		if((*cx)->checked==0) // I can use this as starting cell!
		{
			int xl,xu,yl,yu; // upper and lower edges of the rectangle
			int x,y, x0, y0, x1, y1, dx, dy;
			int rectangle_sum_density=(*cx)->noftrajectories;
			(*cx)->region=new_region;
			(*cx)->checked=1;
			xl=xu=(*cx)->getX_Index();
			yl=yu=(*cx)->getY_Index();

			while (rectangle_sum_density>0)
			{
				int best_density=threshold-1; // No extension that yields less than "threshold" is accepted
				int best_direction=-1;
				int best_x0, best_y0, best_x1, best_y1, best_dx, best_dy;

				// Try the 4 possible directions: right, up, left, down
				for(int dir=0; dir<4; dir++) 
				{
					switch (dir)
					{
					case 0:
						x0=x1=xu+1;
						y0=yl; y1=yu;
						dx=1; dy=0;
						break;
					case 1:
						x0=xl; x1=xu;
						y0=y1=yu+1;
						dx=0; dy=1;
						break;
					case 2:
						x0=x1=xl-1;
						y0=yl; y1=yu;
						dx=1; dy=0;
						break;
					case 3:
						x0=xl; x1=xu;
						y0=y1=yl-1;
						dx=0; dy=1;
						break;
					}

					// Check out-of-bounds
					if((x0>=0) && (y0>=0) && (x1<this->numOfCell_X) && (y1<this->numOfCell_Y))
					{
						int sum_density=0;
						int count_dense=0;
						bool ok=true;
						for(x=x0, y=y0; (x<=x1) && (y<=y1) && ok; x+=dy, y+=dx) // dx==1 => Have to scan the y!!
						{
							Cell * cell_pt=this->getCell(x,y);
							if((cell_pt->checked!=0) || 
							   ((cell_pt->region!=INREGION) && region_cells_only )) { ok=false; }
							else { sum_density+=cell_pt->noftrajectories; }
							if(cell_pt->noftrajectories>=threshold) count_dense++;
						}
						//int local_average_density=sum_density/(dx*(yu-yl+1)+dy*(xu-xl+1));
						int my_average_density=(rectangle_sum_density+sum_density)/((xu-xl+1+dx)*(yu-yl+1+dy));
						if(ok && (my_average_density>best_density) && (count_dense>0)) 
						{ 
							best_direction=dir; 
							best_density=my_average_density; 
							best_x0 = x0;
							best_y0 = y0;
							best_x1 = x1;
							best_y1 = y1;
							best_dx = dx;
							best_dy = dy;
						}
					}
				}
				// Choose best movement
				if(best_direction>=0)
				{
					for(x=best_x0, y=best_y0; (x<=best_x1) && (y<=best_y1); x+=best_dy, y+=best_dx) // Trick: dx/dy
					{
						Cell * cell_pt=this->getCell(x,y);
						rectangle_sum_density+=cell_pt->noftrajectories;
						cell_pt->checked=1;
						cell_pt->region=new_region;
					}

					switch (best_direction) 
					{
						case 0: xu++; break;
						case 1: yu++; break;
						case 2: xl--; break;
						case 3: yl--; break;
					}
				}
				else { rectangle_sum_density=-1; }
			}

			// Create new region
			sub_region.clear();
			for(int x=xl; x<=xu; x++)
				for(int y=yl; y<=yu; y++)
					sub_region.push_back(this->getCell(x,y));
			region * reg = new region(sub_region);
			reg->nOfRegion=new_region++;
			regions.push_back(reg);
		}
	}

	return regions;
}


// Like "Grid::popular_regions" but enforcing max size of regions (=max side length of the rectangle)
vector<region *> Grid::bounded_popular_regions(int threshold, int first_new_region, bool region_cells_only, int max_size){
// WARNING: cells in grid must be set to region=-1 and checked=0
	vector <Cell *> sub_region;
	vector <Cell *> sorted;
	vector <region *> regions;
	vector <region *> reg_hor;
	vector<Cell *> lines;
	region * dense_cells = new region();
	const int INREGION = -2;
	for (map<pair<int, int > , Cell*>::iterator cx = this->begin(); cx != this->end(); cx++){
		if ((*cx).second->noftrajectories >= threshold) 
			{
				dense_cells->push_back((*cx).second);
				(*cx).second->region=INREGION;
			}
	}
	// Higher density cells first
	std::sort(dense_cells->begin(), dense_cells->end(), cell_density_greater_than());
	// Reset "used" flag for all cells
	dense_cells->bound();
	int new_region=first_new_region;

	for (vector<Cell *>::iterator cx = dense_cells->begin(); cx != dense_cells->end(); cx++)
	{
		if((*cx)->checked==0) // I can use this as starting cell!
		{
			int xl,xu,yl,yu; // upper and lower edges of the rectangle
			int x,y, x0, y0, x1, y1, dx, dy;
			int rectangle_sum_density=(*cx)->noftrajectories;
			(*cx)->region=new_region;
			(*cx)->checked=1;
			xl=xu=(*cx)->getX_Index();
			yl=yu=(*cx)->getY_Index();

			while (rectangle_sum_density>0)
			{
				int best_density=threshold-1; // No extension that yields less than "threshold" is accepted
				int best_direction=-1;
				int best_x0, best_y0, best_x1, best_y1, best_dx, best_dy;

				// Try the 4 possible directions: right, up, left, down
				for(int dir=0; dir<4; dir++) 
				{
					switch (dir)
					{
					case 0:
						x0=x1=xu+1;
						y0=yl; y1=yu;
						dx=1; dy=0;
						break;
					case 1:
						x0=xl; x1=xu;
						y0=y1=yu+1;
						dx=0; dy=1;
						break;
					case 2:
						x0=x1=xl-1;
						y0=yl; y1=yu;
						dx=1; dy=0;
						break;
					case 3:
						x0=xl; x1=xu;
						y0=y1=yl-1;
						dx=0; dy=1;
						break;
					}

					// Check out-of-bounds & max size
					if((x0>=0) && (y0>=0) && (x1<this->numOfCell_X) && (y1<this->numOfCell_Y)
					    && (x1-x0<=max_size) && (y1-y0<=max_size))
					{
						int sum_density=0;
						int count_dense=0;
						bool ok=true;
						for(x=x0, y=y0; (x<=x1) && (y<=y1) && ok; x+=dy, y+=dx) // dx==1 => Have to scan the y!!
						{
							Cell * cell_pt=this->getCell(x,y);
							if((cell_pt->checked!=0) || 
							   ((cell_pt->region!=INREGION) && region_cells_only )) { ok=false; }
							else { sum_density+=cell_pt->noftrajectories; }
							if(cell_pt->noftrajectories>=threshold) count_dense++;
						}
						//int local_average_density=sum_density/(dx*(yu-yl+1)+dy*(xu-xl+1));
						int my_average_density=(rectangle_sum_density+sum_density)/((xu-xl+1+dx)*(yu-yl+1+dy));
						if(ok && (my_average_density>best_density) && (count_dense>0)) 
						{ 
							best_direction=dir; 
							best_density=my_average_density; 
							best_x0 = x0;
							best_y0 = y0;
							best_x1 = x1;
							best_y1 = y1;
							best_dx = dx;
							best_dy = dy;
						}
					}
				}
				// Choose best movement
				if(best_direction>=0)
				{
					for(x=best_x0, y=best_y0; (x<=best_x1) && (y<=best_y1); x+=best_dy, y+=best_dx) // Trick: dx/dy
					{
						Cell * cell_pt=this->getCell(x,y);
						rectangle_sum_density+=cell_pt->noftrajectories;
						cell_pt->checked=1;
						cell_pt->region=new_region;
					}

					switch (best_direction) 
					{
						case 0: xu++; break;
						case 1: yu++; break;
						case 2: xl--; break;
						case 3: yl--; break;
					}
				}
				else { rectangle_sum_density=-1; }
			}

			// Create new region
			sub_region.clear();
			for(int x=xl; x<=xu; x++)
				for(int y=yl; y<=yu; y++)
					sub_region.push_back(this->getCell(x,y));
			region * reg = new region(sub_region);
			reg->nOfRegion=new_region++;
			regions.push_back(reg);
		}
	}

	return regions;
}

// Constructor
Grid::Grid(float min_x, float min_y, float max_x, float max_y, float side){

	this->min_x= min_x;
	this->max_x= max_x;
	this->min_y=min_y;
	this->max_y=max_y;
	
	if (side == 0){
		this->size_x =  ((this->max_x - this->min_x)/100);
		this->size_y =  ((this->max_y - this->min_y)/100);

		if (this->size_x > this->size_y)
			this->side = this->size_x;
		else 
			this->side = this->size_y;

	}
	else 
		this->side = side;

	int X_Index=0;
	int Y_Index=0;
	
	for (float i = this->min_x-this->side; i <= this->max_x+(2*this->side); i+=this->side){
		//Y_Index = 0;
		for (float j = this->min_y-this->side; j <= this->max_y+(2*this->side); j+=this->side){
			X_Index = (int) (i/this->side);
			Y_Index = (int) (j/this->side);
			Cell * cx_tmp = new Cell(X_Index,Y_Index);
			std::pair < int ,int > mypair=std::pair < int ,int > ( X_Index, Y_Index) ;
			std::pair < std::pair < int ,int > , Cell* > entry;
			entry.first=mypair;
			entry.second=cx_tmp;
			//cout <<  i << " " << X_Index << " " << j  << " "  << (j/this->side) << " "<< Y_Index << " "  << this->side << endl;
			this->insert(entry);
			
		}
	}
	
	this->numOfCell_X=X_Index;
	this->numOfCell_Y=Y_Index;


}


Grid::Grid(float min_x, float min_y, float max_x, float max_y){

	this->min_x= min_x;
	this->max_x= max_x;
	this->min_y=min_y;
	this->max_y=max_y;

	this->size_x =  ((this->max_x - this->min_x)/100);
	this->size_y =  ((this->max_y - this->min_y)/100);

	if (this->size_x > this->size_y)
		this->side = this->size_x;
	else 
		this->side = this->size_y;
	
	int X_Index=0;
	int Y_Index=0;
	/*
	for (float i = this->min_x-this->side; i <= this->max_x+(2*this->side); i+=this->side, X_Index++){
		Y_Index = 0;
		for (float j = this->min_y-this->side; j <= this->max_y+(2*this->side); j+=this->side, Y_Index++){
			Cell * cx_tmp = new Cell(X_Index,Y_Index);
			std::pair < int ,int > mypair=std::pair < int ,int > ( X_Index, Y_Index) ;
			std::pair < std::pair < int ,int > , Cell* > entry;
			entry.first=mypair;
			entry.second=cx_tmp;
			cout <<  (i/this->side) << " " << X_Index << " " << (j/this->side)  << " " << Y_Index << " "  << this->side << endl;
			this->insert(entry);
			
		}
	}
	*/
	
	for (float i = this->min_x; i <= this->max_x+(2*this->side); i+=this->side){
		//Y_Index = 0;
		for (float j = this->min_y; j <= this->max_y+(2*this->side); j+=this->side){
			X_Index = (int) (i/this->side);
			Y_Index = (int) (j/this->side);
			Cell * cx_tmp = new Cell(X_Index,Y_Index);
			std::pair < int ,int > mypair=std::pair < int ,int > ( X_Index, Y_Index) ;
			std::pair < std::pair < int ,int > , Cell* > entry;
			entry.first=mypair;
			entry.second=cx_tmp;
			//cout <<  i << " " << X_Index << " " << j  << " "  << (j/this->side) << " "<< Y_Index << " "  << this->side << endl;
			this->insert(entry);
			
		}
	}
	
	this->numOfCell_X=X_Index;
	this->numOfCell_Y=Y_Index;
}



int Grid::getMaxX_Index(){
	
	return this->numOfCell_X;
}


int Grid::getMaxY_Index(){
	return this->numOfCell_Y;
}

int Grid::getMinX_Index(){
	return 0;
}

int Grid::getMinY_Index(){
	return 0;	
}

Cell * Grid::getCurCell(float x, float y){
	
	if (x < this->min_x)
		x=this->min_x+this->side;
	if (x > this->max_x)
		x=this->max_x-this->side;
	
	if (y < this->min_y)
		y=this->min_y+this->side;
	if (y > this->max_y)
		y=this->max_y-this->side;

	int X_Index = (int) ((x) / this->side);
	int Y_Index = (int) ((y) / this->side);

	std::map< std::pair < int ,int > , Cell* >::iterator ite_find=this->find (std::pair< int, int > ( X_Index, Y_Index ));
	
	return (*ite_find).second;
	
	
}

Cell * Grid::getCell(int X_Index, int Y_Index){
	
	
	if (X_Index > this->numOfCell_X)
		X_Index=this->numOfCell_X;
	if (Y_Index > this->numOfCell_Y)
		Y_Index=this->numOfCell_Y;
	
	std::pair < int ,int > mypair=std::pair < int ,int > ( X_Index, Y_Index);
	
	std::map< std::pair < int ,int > , Cell* >::iterator cx= this->find(mypair);
	
	return (*cx).second;
}

void Grid::compute_density(vector <Cell *> distinct_cells){
	map< pair < int ,int > , Cell* >::iterator nc;
	for (vector<Cell *>::iterator cx = distinct_cells.begin(); cx!=distinct_cells.end(); cx++){
		pair < int ,int > mypair=pair < int ,int > ( (*cx)->X_Index, (*cx)->Y_Index);
		nc = this->find(mypair);
		(*nc).second->noftrajectories++;
	}
}

void Grid::compute_density(Grid * supportGrid){
	map<pair<int, int > , Cell*>::iterator cx;
	map<pair<int, int > , Cell*>::iterator nc;	
	for (cx = supportGrid->begin(); cx!= supportGrid->end(); cx++){
		if ((*cx).second->noftrajectories == 1 ){
			pair < int ,int > mypair=pair < int ,int > ( (*cx).second->X_Index, (*cx).second->Y_Index);
			nc = this->find(mypair);
			(*nc).second->noftrajectories++;
		}
	}
}

void Grid::reset(vector <Cell *> distinct_cells){
	map<pair<int, int > , Cell*>::iterator nc;	
	
	for (vector <Cell *>::iterator cx = distinct_cells.begin(); cx != distinct_cells.end(); cx++){
		pair < int ,int > mypair=pair < int ,int > ( (*cx)->X_Index, (*cx)->Y_Index);
		nc = this->find(mypair);
		(*nc).second->noftrajectories = 0;
	}
}
void Grid::reset(){
	for (map<pair<int, int > , Cell*>::iterator cx = this->begin(); cx != this->end(); cx++){
		(*cx).second->noftrajectories = 0;
	}


}
void Grid::reset_values(){
	
	for (map<pair<int, int > , Cell*>::iterator cx = this->begin(); cx != this->end(); cx++){
		(*cx).second->noftrajectories = 0; //set to 0 the grid density
		(*cx).second->region = -1; //no region
		(*cx).second->checked = 0;
	}
	
		
}
void Grid::print(){
	ofstream cellFile;
	cellFile.open ("cell2.wkt", ios::out);

	for (map<pair<int, int > , Cell*>::iterator it_cell = this->begin(); it_cell != this->end(); it_cell++){
		
		if ((*it_cell).second->noftrajectories == 0){
			cellFile << "POLYGON ((" << (*it_cell).second->X_Index* this->side << " " << (*it_cell).second->Y_Index * this->side << ", "
			<< (*it_cell).second->X_Index * this->side << " " << ((*it_cell).second->Y_Index*this->side) + this->side << ", " 
			<< ((*it_cell).second->X_Index * this->side)+ this->side  << " " << ((*it_cell).second->Y_Index*this->side) + this->side << ", "
			<< ((*it_cell).second->X_Index * this->side)+ this->side  << " " << ((*it_cell).second->Y_Index*this->side) << ", "
			<< (*it_cell).second->X_Index * this->side << " " << (*it_cell).second->Y_Index * this->side << "))\n\n";
		}
	}

	cellFile.close();
	cellFile.open ("cell.wkt", ios::out);
	
	for (map<pair<int, int > , Cell*>::iterator it_cell = this->begin(); it_cell != this->end(); it_cell++){
		
		if ((*it_cell).second->noftrajectories > 1){
			cellFile << "POLYGON ((" << (*it_cell).second->X_Index* this->side << " " << (*it_cell).second->Y_Index * this->side << ", "
			<< (*it_cell).second->X_Index * this->side << " " << ((*it_cell).second->Y_Index*this->side) + this->side << ", " 
			<< ((*it_cell).second->X_Index * this->side)+ this->side  << " " << ((*it_cell).second->Y_Index*this->side) + this->side << ", "
			<< ((*it_cell).second->X_Index * this->side)+ this->side  << " " << ((*it_cell).second->Y_Index*this->side) << ", "
			<< (*it_cell).second->X_Index * this->side << " " << (*it_cell).second->Y_Index * this->side << "))\n\n";
		}
	}

	cellFile.close();
}

void Grid::print(int threshold){
	ofstream cellFile;
	cellFile.open ("cell_threshold.wkt", ios::out);

	for (map<pair<int, int > , Cell*>::iterator it_cell = this->begin(); it_cell != this->end(); it_cell++){
		
		if ((*it_cell).second->noftrajectories >= threshold){
			cellFile << "POLYGON ((" << (*it_cell).second->X_Index* this->side << " " << (*it_cell).second->Y_Index * this->side << ", "
			<< (*it_cell).second->X_Index * this->side << " " << ((*it_cell).second->Y_Index*this->side) + this->side << ", " 
			<< ((*it_cell).second->X_Index * this->side)+ this->side  << " " << ((*it_cell).second->Y_Index*this->side) + this->side << ", "
			<< ((*it_cell).second->X_Index * this->side)+ this->side  << " " << ((*it_cell).second->Y_Index*this->side) << ", "
			<< (*it_cell).second->X_Index * this->side << " " << (*it_cell).second->Y_Index * this->side << "))\n\n";
		}
	}

	cellFile.close();
	
}


float Grid::getSide(){
	return this->side;
}




