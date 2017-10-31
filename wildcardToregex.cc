#include <iostream>
#include <regex>
#include <string>
using namespace std;

void wildcardToregex(string& regexstr, string& wildcardstr)
{
	//string regexstr = "";
	if ( wildcardstr.empty() )
		regexstr = ".*";
	//convert wildcard-exp to regular-exp
	for (int i=0; i < wildcardstr.length(); i++)
	{
		switch(wildcardstr.at(i))
		{
			case '*':
				regexstr += ".*";
				break;
			case '?':
				regexstr += ".";
//			case '.':
//				regexstr += "\\.";
			default:
				regexstr += wildcardstr.at(i);
		}
	}
}

int main()
{
	string setting = "*.baidu.*";
	string regexstr = "";
	wildcardToregex(regexstr, setting);
	std::cout<<"convert result:"<<regexstr<<std::endl;
	
	std::string referer("obs.baidu.com");

	string pattern = regexstr;
    
    std:regex_constants::syntax_option_type optype = std::regex_constants::ECMAScript;
    optype = std::regex_constants::icase;
	std::regex e(pattern, optype);

	if (std::regex_match(referer, e))
		std::cout<<"string obj matched"<<std::endl;
	else
		std::cout<<"Not matched"<<std::endl;
	return 0;
}

