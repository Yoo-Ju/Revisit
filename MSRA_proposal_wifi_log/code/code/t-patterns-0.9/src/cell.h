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
#include <vector>
#pragma once
using namespace std;

class Cell {

public:

	/*************************************** VARIABILI PRIVATE ********************************************/
	int X_Index; //indice sull'asse X
	int Y_Index; //indice sull'asse Z
	float side; //dimensione del side
	int noftrajectories;
	int checked;
	int region;
	/*************************************** FINE VARIABILI PRIVATE ****************************************/

	/****************************************** METODI PUBBLICI *********************************************/

	Cell(int X_Ind, int Y_Ind, float lt); //costruttore con i due parametri che specificano gli indici della cella
	Cell(int X_Ind, int Y_Ind);
	~Cell(); //distruttore
	Cell ();
	int getX_Index(); //restituisce l'indice sull'asse X della cella
	int getY_Index(); //restituisce l'indice sull'asse Z della cella
	int checkCell(int threshold);	
	float getCenterCell_X(); //restituisce la coordinata X del centro della generica cella (i,*)
	float getCenterCell_Z(); //restituisce la coordinata Z del centro della generica cella (*,j)
	float getSide(); //restituisce la dimensione del side della cella
	float getVertexX(int i); //restituisce la componente X dell'i-esimo vertice 
	float getVertexY(int i); //restituisce la componente Y dell'i-esimo vertice 
	float getVertexZ(int i); //restituisce la componente Z dell'i-esimo vertice 
	void setNofTrajectories();
	int isDense(int threshold);
	/****************************************** FINE METODI PUBBLICI ****************************************/

};




