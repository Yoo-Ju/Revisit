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

#include "debug.h"
#include <vector>
#include <string>
#include <map>

using namespace std;

const string ELEMENT_SEPARATOR = "*";
const string SEQUENCE_SEPARATOR = "#";

class name_map
{
	vector<string> id2string;
	map<string,int> string2id;
	bool active;

public:

	name_map() { active=1; };
	void enable() { active=1; };
	void disable() { active=0; };
	void clear() { id2string.clear(); string2id.clear();};
	/// Returns a new id, if string is not known, and -1 or -2 when special separator are met
	int assign_id(string st)
	{
		if(st==ELEMENT_SEPARATOR) return -1;
		if(st==SEQUENCE_SEPARATOR) return -2;
		if(active)
		{
			if(string2id[st]==0) 
			{
				id2string.push_back(st);
				string2id[st]=static_cast<int>(id2string.size());
			};
			return string2id[st]-1;
		} else return atoi(st.c_str());
	};

	/// Returns -1 if string is not known
	int get_id(string st)
	{
		return string2id[st]-1;
	};

	/// Returns "#unknown id#" if id is not known
	string get_string(int id)
	{
		char tmp[20];
		if(active)
		{
			if(id<0 || id>=static_cast<int>(id2string.size()))
				return "#unknown id#";
			else
				return id2string[id];
		}
		else { sprintf(tmp,"%d",id); return tmp;};
	};
};




