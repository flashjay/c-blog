#include "blog.h"

#define	VALIDATE_INT(key, val, result)  \
	val = tcmapget2(map, key); \
	if(val == NULL) \
	{ \
		result = 0; \
		tcmapdel(map); \
		log_debug("blog.conf %s not found\r\n", key); \
		exit(0); \
	} \
	else \
	{ \
		result = atoi(val); \
		val    = NULL; \
	}


#define	VALIDATE_STRINT(key, val, result)  \
	val = tcmapget2(map, key); \
	if(val == NULL) \
	{ \
		result = NULL; \
		tcmapdel(map); \
		log_debug("blog.conf %s not found\r\n", key); \
		exit(0); \
	} \
	else \
	{ \
		result = strdup(val); \
		val    = NULL; \
	}


void parse_conf()
{
	FILE   *fp;
	TCLIST *data;
	TCMAP  *map; 
	
	char       *key, *val, buf[256];
	const char *value;
		
	fp = fopen("../conf/blog.conf", "r");
	if(!fp) exit(0) ;
	
	map = tcmapnew();
	memset(buf, 0, sizeof(buf));

	while ((fgets(buf, 255, fp)) != NULL)
	{
		trim(buf);
		if(strlen(buf) == 0) continue;
		if(buf[0] == '#') continue;
		
		data = explode("=", buf);
		if(tclistnum(data) == 2)
		{
			key = strdup(tclistval2(data, 0));
			val = strdup(tclistval2(data, 1));
			
			trim(key);
			trim(val);

			tcmapput(map, key, strlen(key), val, strlen(val));
			safe_free(key);
			safe_free(val);
		}
		
		tclistdel(data);
		memset(buf, 0, sizeof(buf));
	}
	fclose(fp);
	
	VALIDATE_INT("blog_page_size", value, conf.page.blog)
	VALIDATE_INT("admin_page_size", value, conf.page.admin)
	VALIDATE_INT("comment_page_size", value, conf.page.comment)

	VALIDATE_STRINT("db", value, conf.path)
	VALIDATE_STRINT("username", value, conf.username)
	VALIDATE_STRINT("password", value, conf.password)


	tcmapdel(map);
}
