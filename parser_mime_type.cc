#include <iostream>
#include <map>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
using namespace std;

/*parser mime type file*/


const char *ext_map = "mime.types";
static map<string, string> ext_mime_map;

void parse_mime_map_line(const char* start, const char* end)
{
	char line[end - start+1];
	strncpy(line, start, end - start);
	line[end - start] = '\0';
	char* l = line;
#define DELIME " \t\n\r"
	while(isspace(*l)) 
		l++;
	char *mime = strsep(&l, DELIMS);
	if (!mime)
		return;
	char* ext;
	do {
		ext = strsep(&l, DELIMS);
		if (ext && *ext) {
			ext_mime_map[ext] = mime;
		}
	} while(ext);
}

void parse_mime_map(const char* buf)
{
	const char* start = buf, *end = buf;
	while (*end) {
		while ( *end && *end != '\n' ) {
			end++;
		}
		parse_mime_map_line(start, end);
		end++;
		start = end;
	}
}

static int mime_map_init(const char *ext_map)
{
	int fd = open(ext_map, O_RDONLY);
	char *buf = NULL;
	int ret ;
	if (fd < 0){
		ret = -errno;
		return ret;
	}
	struct stat st;
	ret = fstat(fd, &st);
	if (ret < 0){
		ret = -errno;
		goto done;
	}
	buf = (char*)malloc(st.st_size + 1);
	if (!buf) {
		ret = -ENOMEM;
		goto done;
	}
	ret = read(fd, buf, st.st_size + 1);
	if (ret != st.st_size){
		free(buf);
		close(fd);

	}
	buf[st.st_size] = '\0';
	parse_mime_map(buf);
	ret = 0;
done:
	free(buf);
	close(fd);
	return ret;
}
