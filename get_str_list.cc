#include <string>
#include <iostream>
#include <list>
using namespace std;

static bool get_next_token(const string& s, size_t& pos, const char* delims, string& token)
{
	int start = s.find_first_not_of(delims, pos);
	int end;
	if (start < 0){
		pos = s.size();
		return false;
	}

	end = s.find_first_of(delims, start);
	if (end >= 0)
		pos = end +1;
	else {
		pos = end = s.size();
	}
	token = s.substr(start, end - start);
	return true;
}

void get_str_list(const string& str, const char* delims, list<string>& str_list)
{
	size_t pos = 0;
	string token;
	str_list.clear();

	while (pos < str.size()) {
		if (get_next_token(str, pos, delims, token)) {
			if (token.size() > 0) {
				str_list.push_back(token);
			}
		}
	}
}
void get_str_list(const string& str, list<string>& str_list)
{
	const char* delims = ";,= \t";
	return get_str_list(str, delims, str_list);
}
int main()
{
	list<string> apis;
	get_str_list("s3, swift, swift_auth, admin", apis);
	return 0;
}
