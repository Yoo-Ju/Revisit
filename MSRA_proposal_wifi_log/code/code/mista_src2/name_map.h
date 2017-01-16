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



