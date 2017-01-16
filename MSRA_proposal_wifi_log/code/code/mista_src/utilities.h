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
 #define time_unit 1000000
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
		return (float)delta/time_unit;
	};
};

#endif



