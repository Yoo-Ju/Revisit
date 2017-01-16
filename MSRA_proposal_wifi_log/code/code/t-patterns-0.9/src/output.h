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
#include <iostream>
#include <fstream>
#include "Tsequences.h"
#include "grid.h"

using namespace std;

void print_postscript_grid(char * filename, Grid * grid, vector<region *> regions, int threshold);
void PSscript (H_store * results, vector<TProjection*>::iterator exp);
void d_graph (vector<TProjection*>::iterator exp);
void d_graph (vector<TProjection*>::iterator exp, string str);
void d_graph (vector<TProjection*>* projections, vector<TProjection*>::iterator exp);
void d_graph(string str, int position);
void c_log(string str, int position);
void c_log(vector<TProjection*>::iterator exp, int mycc);
void c_log(H_store * results, float time);
void c_log(float time);



