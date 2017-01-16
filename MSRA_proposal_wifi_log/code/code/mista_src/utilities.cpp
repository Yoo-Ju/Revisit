#include "utilities.h"

int MAX_INTEGER=(int)floor(pow((float)2,(int)(8*sizeof(int)-2))); 

void launch_error(const char *msg) 
{
	cerr << "\n################\nERROR: " << msg << "\n################\n";
	exit(-1);
};



