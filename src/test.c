#include <stdio.h>
#include <string.h>
#include <tcutil.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>



#define	VALIDATE_STRINT(key, val, result)  \
	val = tcmapget2(res, key); \
	if(val == NULL) \
		tcmapdel(res); \
	else \
	{ \
		result = strdup(val); \
		val    = NULL; \
	}



int main(int argc, char *argv[])
{
	TCMAP *res;
	char  *data;
	const char *val;
	
	res = tcmapnew();
	tcmapput2(res, "foo", "hop");
    tcmapput2(res, "bar", "step");
    tcmapput2(res, "baz", "jump");
	

	VALIDATE_STRINT("foo", val, data)

	printf("foo = %s\r\n", data);
	
	//tcmapdel(res);
	return 0;
}
