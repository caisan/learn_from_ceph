#include <string>
#include <iostream>
#include <regex>
using namespace std;

string& convert_regex(string& pattern, const string& t, const string& alpha )
{
	while(true)
	{
		string::size_type pos(0);
		if ((pos = pattern.find(t)) != string::npos)
		{
			pattern.replace(pos, t.length(), alpha);

		}
		else
			break;

	}
	return pattern;


}
string& replace_all_distinct(string&   str,const   string&   old_value,const   string&   new_value)   
{
	for(string::size_type pos(0);pos!=string::npos;pos+=new_value.length()){
		if((pos=str.find(old_value,pos))!=string::npos)
			str.replace(pos,old_value.length(),new_value);
		else
			break;   

	}
	return
		str;   

}   
int main()
{
	std::string s ("WwW.baidu.com");
	std::string pattern(".*.baidu.com"); 
//	pattern = replace_all_distinct(pattern, "?", "[[:alnum:]].");
    std:regex_constants::syntax_option_type optype = std::regex_constants::ECMAScript;
	optype = std::regex_constants::icase; 

	std::regex e(pattern, optype);
	if (std::regex_match (s,e))
		std::cout<<"string obj matched";
	return 0;

}
