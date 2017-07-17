#include <iostream>
#include <string>
using namespace std;

char strMatch( const char *str1, const char *str2 )  
{  
	int slen1 = strlen(str1);  
	int slen2 = strlen(str2);

	char matchmap[128][128];
	memset(matchmap, 0, 128*128);  
	int i, j, k;  
	int lbound = 0;
	int upbound = 0;
	for(i = 0; i< slen1; ++i)  
	{
		int bMatched = 0;
		int upthis = upbound;
		for(j = lbound; j<=upthis ; ++j)
		{
			if(str1[i] == str2[j] || str2[j] == '?')
			{ 
				matchmap[i][j] = 1;
				if(0 == bMatched)
				{
					lbound = j+1;
				}
				upbound = j+1;
				bMatched = 1;
				if(i == slen1 - 1)
				{
					for(k = j+1 ; k < slen2 && '*' == str2[k] ; ++k)
					{
						matchmap[i][k] = 1;
					}
				}
			}else if(str2[j] == '*')
			{
				if(0 == bMatched)
				{
					lbound = j;
				}
				for(k = i; k< slen1; ++k)
				{
					matchmap[k][j] = 1;  
				}
				k = j;
				while( '*' == str2[++k] )
				{
					matchmap[i][k] = 1;
				}
				if(str1[i] == str2[k] || str2[k] == '?')
				{
					matchmap[i][k] = 1;
					upbound = k+1;
					if(i == slen1 - 1)
					{
						for(k = k+1 ; k < slen2 && '*' == str2[k] ; ++k)
						{
							matchmap[i][k] = 1;
						}
					}
				}else{
					upbound = k;
				}
				bMatched = 1;
			}
		}
		if(!bMatched )
		{
			return 0;

		}
	}
	return matchmap[slen1-1][slen2-1];  
}  
int main()
{
	const char* str1 = "img.userdomain";
	const char* str2 = "*.userdomain";
    printf("%d\n", strMatch( str1, str2 ));
	return 0;
}
