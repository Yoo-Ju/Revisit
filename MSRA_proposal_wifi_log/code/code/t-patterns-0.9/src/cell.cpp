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

#include "cell.h"
#include <vector>
#include <stdlib.h>
#include <iostream>

using namespace std;


Cell::Cell(){
	X_Index = 0;
	Y_Index = 0;
	side = 0;
	noftrajectories = 0;
	checked = 0;
	region = -1;
	
}

Cell::Cell(int X_Ind, int Y_Ind) {

	X_Index = X_Ind;
	Y_Index = Y_Ind;
	side = 0;
	noftrajectories = 0;
	checked = 0;
	region = -1;

}
Cell::Cell(int X_Ind, int Y_Ind, float lt) {
	
	X_Index = X_Ind;
	Y_Index = Y_Ind;
	side = lt;
	noftrajectories = 0;
	checked = 0;
	region = -1;
}

Cell::~Cell() {}

int Cell::checkCell(int threshold){

	if (noftrajectories < threshold)
		return 0;
	else {
		if (checked)
			return 0;
		else {
			return 1;
		}
	}

}

int Cell::getX_Index() {
	
	return X_Index;
}

int Cell::getY_Index() {
	return Y_Index;
}

float Cell::getCenterCell_X() {
	return (X_Index*side)+(side/2);
}

float Cell::getCenterCell_Z() {
	return (Y_Index*side)+(side/2);
}

float Cell::getSide() {
	return side;
}

float Cell::getVertexX(int i) {
	return 0;
}

float Cell::getVertexZ(int i) {
	return 0;
}

float Cell::getVertexY(int i) {
	return 0;
}

void Cell::setNofTrajectories (){
	noftrajectories=1;
}

int Cell::isDense(int threshold){
	if (noftrajectories > threshold)
		return 1;
	else 
		return 0;
}


