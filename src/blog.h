#ifndef __BLOG_H__
#define __BLOG_H__

#include <fcgi_config.h>
#include <fcgi_stdio.h>

#include <ftw.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <stdarg.h>	
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h> 
#include <dirent.h>
#include <libgen.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <uuid/uuid.h>

#include <sqlite3.h>
#include <tcutil.h>
#include "dlmalloc.h"
#include "log.h"
#include "env.h"
#include "rbtree.h"
#include "tpllib.h"
#include "util.h"
#include "assign.h"
#include "module/blog/click.h"
#include "module/blog/comment.h"
#include "module/blog/article.h"

#include "module/panel/user.h"
#include "module/panel/comment.h"
#include "module/panel/article.h"
#include "module/panel/category.h"
#include "module/panel/friendlink.h"


#define safe_free(x)     if(x){free(x);x=NULL;}

struct page_t
{
	uint16_t blog;       //前台的列表页的 每页显示数
	uint16_t admin;      //后台的列表页的 每页显示数
	uint16_t comment;    //前台的评论页的 每页显示数
};

struct conf_t
{
	//数据库路径
	char *path;
	char *username;
	char *password;
	struct page_t page;
};

/*

缓存文章的点击数和评论数  

*/ 

struct click_t
{
	struct rb_node node;
	
	uint           id;
	ulong          hit;
	ulong          comment;
};


struct filter_t
{
	char    *search;
	char    *replace;
	size_t  len;
};

static struct filter_t filter[] = {
	{"\r\n",  "<br />", 2  },
	{"<",     "&lt;",   1  },
	{">",     "&gt;",   1  },
	{"\"",    "&quot;", 1  },
	{"'",     "&quot;", 1  },
	{ NULL,   NULL,     0  }
};


sqlite3 *db;

struct rb_root rr;

char session_id[40];

void parse_conf();

struct conf_t conf;

#endif

