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

#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include <iostream>
#include <math.h>
#include <time.h>

using namespace std;

#define APPROX_FLOAT(a) ((float)(floor((float)(a)*100+0.5))/100)
// Define MAX_INTERGER as a 30 bit unsigned int (not 31 to avoid troubles with roundings)
extern int MAX_INTEGER;

//#define APPROX_FLOAT(a) a

/// Stops execution and print out error message
void launch_error(const char *msg);

#ifdef WIN32
 #define time_unit 1000
#else
 #define time_unit 1000
#endif

class my_clock 
{
	clock_t my_time;
public:
	float elapse()
	{
		clock_t tmp=clock();
		long delta=static_cast<long>(tmp-my_time);
		my_time=tmp;
		return (float)delta/CLOCKS_PER_SEC;
	};
};

#endif



