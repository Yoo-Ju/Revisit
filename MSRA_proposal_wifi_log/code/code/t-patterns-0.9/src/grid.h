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
#include "cell.h"
#include "regions.h"

#pragma once

class Grid: public map < pair <int,int>,Cell *> {
	
	public:

		float size_x;
		float size_y;
		float side;
		int numOfCell_X;
		int numOfCell_Y;
		float min_x;
		float min_y;
		float max_x;
		float max_y;
		
		Grid(float min_x, float min_y, float max_x, float max_y, float side);
		Grid(float min_x, float min_y, float max_x, float max_y);
		Cell * getCurCell(float x, float y);
		Cell * getCell(int X_Index, int Y_Index); 
		
		int getMaxX_Index(); //restituisce l'indice X massimo tra un vector di celle
		int getMaxY_Index(); //restituisce l'indice Z massimo tra un vector di celle
		int getMinX_Index(); //restituisce l'indice X minimo tra un vector di celle
		int getMinY_Index(); //restituisce l'indice Z minimo tra un vector di celle

		float getX_Size(); //restituisce la dimensione della griglia lungo l'asse X
		float getY_Size(); //restituisce la dimensione della griglia lungo l'asse Z
		float getSide(); //restituisce la dimensione del side di una singola cella
		
		
		void compute_density(vector <Cell *> distinct_cells);
		void compute_density(Grid * supportGrid);
		void reset(vector <Cell *> distinct_cells);
		void reset();
		void reset_values();
		void print();
		void print(int threshold);
		
		vector<region *> popular_regions(int threshold, int first_new_region, bool region_cells_only);
		vector<region *> bounded_popular_regions(int threshold, int first_new_region, bool region_cells_only, int max_size);

};

// Older version 
/*
class Grid {


public:

	int numOfCell_X; //numero di celle lungo l'asse X
	int numOfCell_Z; //numero di celle lungo l'asse Z
	float side; //dimensione del side di una cella
	double mm[16];
	double mp[16];
	std::vector<Cell*> grid; //vector di oggetti Cell che rappresenta la griglia



	Grid(const int sizeX=10, const int sizeY=10, const float lt=1.0);//costruttore con 3 parametri di default: n.di celle lungo le X, lungo le Z e side della cella// 
	Grid(float side, float min_x, float min_y, float max_x, float max_y);
	~Grid();//distruttore
	
	void insertCell(int numCellX, int numCellY, float side); //inserisce le celle nel vector grid
	void Grid::insertCell(float min_x, float min_y);
	void clear(); //ripulisce il vettore grid
	void restart(); //resetta i parametri di tutte le celle
	
	int getNumCellX(); //restituisce il numero di celle lungo l'asse X
	int getNumCellY(); //restituisce il numero di celle lungo l'asse Z
	int getNumPicked(); //restituisce il numero di celle picked
	int getMaxX_Index(std::vector<Cell *> vectorCell); //restituisce l'indice X massimo tra un vector di celle
	int getMaxY_Index(std::vector<Cell *> vectorCell); //restituisce l'indice Z massimo tra un vector di celle
	int getMinX_Index(std::vector<Cell *> vectorCell); //restituisce l'indice X minimo tra un vector di celle
	int getMinY_Index(std::vector<Cell *> vectorCell); //restituisce l'indice Z minimo tra un vector di celle

	float getX_Size(); //restituisce la dimensione della griglia lungo l'asse X
	float getY_Size(); //restituisce la dimensione della griglia lungo l'asse Z
	float getSide(); //restituisce la dimensione del side di una singola cella
	
	Cell *getCell(int X_Index, int Y_Index); //restituisce la cella in posizione (X_Index,Y_Index) sulla griglia
	Cell *getCurCell(float curPosX, float curPosZ); //restituisce la cella corrente in base alla posizione assoluta
	void compute_density(vector <Cell *> distinct_cells);
	void compute_density(Grid * supportGrid);
	void reset(vector <Cell *> distinct_cells);
	void reset();
	void reset_values();
	void print();
	void print_cell(int threshold);
	
	vector<region *> popular_regions(int threshold, int first_new_region, bool region_cells_only);
	

};
*/



