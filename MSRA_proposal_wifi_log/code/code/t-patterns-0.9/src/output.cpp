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

#include "output.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;


void print_postscript_grid(char * filename, Grid * grid, vector<region *> regions, int threshold){

	ofstream ps(filename);
	float PS_BORDER = (float)0.00001;
	float size_x=100;
	float size_y=100;
	float border_x=5;
	float border_y=5;
	float step_x=(float)(size_x-border_x)/grid->numOfCell_X;
	float step_y=(float)(size_y-border_y)/grid->numOfCell_Y;

	// Header
	ps << "%!PS-Adobe-2.0 EPSF-3.0" << endl
		<< "%%Creator: MiSTA 1.1" << endl
		<< "%%For: GhostView" << endl
		<< "%%Title: Density grid" << endl
		<< "%%BoundingBox: 0 0 " << size_x << " " << size_y << endl
		<< "%%EndComments" << endl << endl;

	// Compute max and min density
	float mind=100000000;
	float maxd=-100000000;

	for(map<pair<int, int > , Cell*>::iterator it=grid->begin(); it!=grid->end(); it++)
	{
		mind=((*it).second->noftrajectories<mind)?(*it).second->noftrajectories:mind;
		maxd=((*it).second->noftrajectories>maxd)?(*it).second->noftrajectories:maxd;
	}
	
	// Draw axis
	ps << ((step_x+step_y)/10) << " setlinewidth\n";
	ps << "newpath\n" << "0 " << (border_y/2) << " moveto\n";
	ps << size_x << " " << (border_y/2) << " lineto\n";
	ps << (size_x-border_x/2) << " " << border_y << " lineto\n";
	ps << size_x << " " << (border_y/2) << " moveto\n";
	ps << (size_x-border_x/2) << " " << 0 << " lineto\n";
	ps << (border_x/2) << " 0 moveto\n";
	ps << (border_x/2) << " " << size_y << " lineto\n";
	ps << border_x << " " << (size_y-border_y/2) << " lineto\n";
	ps << (border_x/2) << " " << size_y << " moveto\n";
	ps << 0 << " " << (size_y-border_y/2) << " lineto\n";
	ps << "closepath 0 setgray stroke\n";

	// Draw cells
	float gray;
	int prev_density=-1;
	for(map<pair<int, int > , Cell*>::iterator it=grid->begin(); it!=grid->end(); it++)
	{
		float x0=border_x+step_x*(*it).second->X_Index;
		float y0=border_y+step_y*(*it).second->Y_Index;
		if((*it).second->noftrajectories != prev_density)
		{
			prev_density=(*it).second->noftrajectories;
			gray = (1-log(1+((float)prev_density-mind)/maxd));
			if(prev_density>=threshold) 
			{ 
				ps << gray << " 0 0 setrgbcolor\n";

			} else 
			{
				ps << gray << " setgray\n";			
			}
		}
		ps << x0 << " " << y0 << " " << (step_x-PS_BORDER) << " " << (step_y-PS_BORDER) << " rectfill\n";
		//ps << " fill\n";
	};

	// Draw regions
	ps << " 0.2 0 0 setrgbcolor\n";
	ps << ((step_x+step_y)/5) << " setlinewidth\n";
	for(vector<region *>::iterator it=regions.begin(); it!=regions.end(); it++)
	{
		(*it)->bound();
		float x0, x1, y0, y1;
		x0=border_x+step_x*(*it)->min_x;
		x1=border_x+step_x*((*it)->max_x+1)-PS_BORDER;
		y0=border_y+step_y*(*it)->min_y;
		y1=border_y+step_y*((*it)->max_y+1)-PS_BORDER;
		ps << "newpath\n";
		ps << " " << x0*100 << " " << y0*100 << " moveto\n";
		ps << " " << x1*100 << " " << y0*100 << " lineto\n";
		ps << " " << x1*100 << " " << y1*100 << " lineto\n";
		ps << " " << x0*100 << " " << y1*100 << " lineto\n";
		ps << " closepath\n";
		ps << " stroke\n";
	};


	ps.close();
					
}

void PSscript (H_store * results, vector<TProjection*>::iterator exp){

	string fname=(*exp)->myname()+".ps";
	ofstream ps(fname.c_str());
	int PS_BORDER = 4;
	float minx, maxx, miny, maxy, mind, maxd;
	mind=minx=miny=10000000;
	maxd=maxx=maxy=-10000000;
	for(vector<H*>::iterator it=results->rectangles.begin(); it!=results->rectangles.end(); it++)
		if(*it!=0 && (*it)->density>=min_sup)
		{
			minx=min(minx,(*it)->low[0]);
			miny=min(miny,(*it)->low[1]);
			maxx=max(maxx,(*it)->up[0]);
			maxy=max(maxy,(*it)->up[1]);
			mind=min(mind,(float)(*it)->density);
			maxd=max(maxd,(float)(*it)->density);
		};
	if(maxx==minx)
		maxx=minx+1;
	if(maxy==miny)
		maxy=miny+1;
	if(maxd==mind)
		maxd=mind+1;

	ps << "%!PS-Adobe-2.0" << endl
		<< "%%Creator: MiSTA 1.1" << endl
		<< "%%For: GhostView" << endl
		<< "%%Title: Density plot" << endl
		<< "%%BoundingBox: 10 10 580 830" << endl
		<< "%%EndComments" << endl << endl;

	for(vector<H*>::iterator it=results->rectangles.begin(); it!=results->rectangles.end(); it++)
		if(*it!=0 && (*it)->density>=min_sup)
		{
			float x0, x1, y0, y1;
			x0=10+570*((*it)->low[0]-minx)/(maxx-minx);
			x1=10+570*((*it)->up[0]-minx)/(maxx-minx)-PS_BORDER;
			y0=10+820*((*it)->low[1]-miny)/(maxy-miny);
			y1=10+820*((*it)->up[1]-miny)/(maxy-miny)-PS_BORDER;
			ps << "newpath\n";
			ps << " " << x0 << " " << y0 << " moveto\n";
			ps << " " << x1 << " " << y0 << " lineto\n";
			ps << " " << x1 << " " << y1 << " lineto\n";
			ps << " " << x0 << " " << y1 << " lineto\n";
			ps << " closepath\n";
			ps << " " << (0.5*(1-((float)(*it)->density-mind)/(maxd-mind))) 
				<< " setgray\n";
			ps << " fill\n";
		};
	ps << "showpage\n";
	ps.close();
					
}

void d_graph(vector<TProjection*>::iterator exp){
	int a,b,c,d;
	ofstream graph_file;
	graph_file.open("MiSTA.dot", ios::app);
	(*exp)->evaluate_size(a,b,c,d);
	graph_file << (*exp)->myname() << "  [label=\"" << *(*exp)->prefix << "\\n" << (*exp)->prefix_support << "->" << a << "\\nEl:" << b << ",It:" << c << ",An:" << d << "\"];\n";
	graph_file.close();	
}

void d_graph (vector<TProjection*>::iterator exp, string str){

	ofstream graph_file;
	graph_file.open("MiSTA.dot", ios::app);
	graph_file << (*exp)->myname() << str;
	graph_file.close();
}

void d_graph (vector<TProjection*>* projections, vector<TProjection*>::iterator exp){
	ofstream graph_file;
	graph_file.open("MiSTA.dot", ios::app);
	
	for(vector<TProjection*>::iterator pr=projections->begin(); pr!=projections->end(); pr++)
	{
		graph_file << "  " << (*exp)->myname() << " -> " << (*pr)->myname() << ";\n";
	};
	graph_file.close();				

}

void d_graph(string str, int position){
	ofstream graph_file;
	if (position) 
		graph_file.open("MiSTA.dot", ios::app);
	else 
		graph_file.open("MiSTA.dot");
	
	graph_file << str;
	graph_file.close();
	
}

void c_log(string str, int position){

	ofstream log_file;
	if (position) 
		log_file.open("MiSTA.log", ios::app);
	else 
		log_file.open("MiSTA.log");
	
	log_file << str;
	log_file.close();


}

void c_log (vector<TProjection*>::iterator exp, int mycc){
	ofstream log_file;
	log_file.open("MiSTA.log", ios::app);
	for(TProjection::iterator it=(*exp)->begin(); it!=(*exp)->end(); it++) 
		mycc+=(int)(*it)->annotations->size();
	log_file << ((int)(*exp)->prefix->size()-1) << "\t"  << mycc << "\t";
	log_file.close();				
}


void c_log(H_store * results, float time){

	ofstream log_file;
	log_file.open("MiSTA.log", ios::app);
	
	int unempty_cc=0;
	for(vector<H*>::iterator it=results->rectangles.begin(); it!=results->rectangles.end(); it++)
		if(*it!=0) unempty_cc++;
	log_file << (int)results->rectangles.size() << "\t" << unempty_cc << "\t" << time;
					
	log_file.close();
}

void c_log(float time){

	ofstream log_file;
	log_file.open("MiSTA.log", ios::app);
	log_file << "\t" << time << endl;				
	log_file.close();

}



