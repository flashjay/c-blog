#include "blog.h"


/*

SQLite3 操作
http://www.tokesoft.net/topics/TS20100222_02.html
/usr/local/spawn-fcgi/bin/spawn-fcgi -f ./blog -s /tmp/fcgi-blog.sock

gcc -o blog blog.c conf.c log.c util.c env.c tpllib.c rbtree.c assign.c dlmalloc.c ./module/blog/comment.c ./module/blog/click.c ./module/panel/comment.c ./module/panel/article.c ./module/panel/user.c ./module/panel/friendlink.c ./module/blog/article.c ./module/panel/category.c -I./ -I/usr/local/tokyocabinet/include -I/usr/local/fcgi/include -I/usr/local/sqlite/include/ /usr/local/tokyocabinet/lib/libtokyocabinet.a /usr/local/fcgi/lib/libfcgi.a  /usr/local/sqlite/lib/libsqlite3.a -O2 -lpthread -ldl -lm -lz -luuid -lbz2 -D_BUILD_HTML -DMSPACES -DUSE_DL_PREFIX

*/


struct cmd_t
{
	char *module;
	char *action;
	void (*handler)(struct env_t *_SERVER);
};


struct cmd_t panel_cmd_t[] = {
	{"user",         "quit",     panel_user_quit         },
	{"user",         "login",    panel_user_login        },
	{"comment",      "list",     panel_comment_list      },
	{"comment",      "delete",   panel_comment_delete    },
	{"article",      "list",     panel_article_list      },
	{"article",      "clean",    panel_article_clean     },
	{"article",      "index",    panel_article_index     },
	{"article",      "insert",   panel_article_insert    },
	{"article",      "delete",   panel_article_delete    },
	{"article",      "update",   panel_article_update    },
	{"article",      "upload",   panel_article_upload    },
	{"category",     "move",     panel_category_move     },
	{"category",     "list",     panel_category_list     },
	{"category",     "insert",   panel_category_insert   },
	{"category",     "delete",   panel_category_delete   },
	{"category",     "update",   panel_category_update   },
	{"friendlink",   "move",     panel_friendlink_move   },
	{"friendlink",   "list",     panel_friendlink_list   },
	{"friendlink",   "insert",   panel_friendlink_insert },
	{"friendlink",   "delete",   panel_friendlink_delete },
	{"friendlink",   "update",   panel_friendlink_update },
	{ NULL,          NULL,       NULL                    }
};


struct cmd_t blog_cmd_t[] = {
	{"click",     "once",     blog_click_once     },
	{"click",     "count",    blog_click_count    },
	{"article",   "read",     blog_article_read   },
	{"article",   "list",     blog_article_list   },
	{"comment",   "list",     blog_comment_list   },
	{"comment",   "insert",   blog_comment_insert },
	{ NULL,        NULL,      NULL                }
};

static void init();

void term_handler(int num) 
{
	int    rc;
	sqlite3_stmt     *stmt;
	struct rb_node   *node;
	struct click_t   *recs;


	//同步
	sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, NULL);
	for (node = rb_first(&rr); node; node = rb_next(node))
	{
		recs = rb_entry(node, struct click_t, node);
		
		rc = sqlite3_prepare(db, "UPDATE article SET hit = ?, comment_num = ? WHERE art_id = ?;", -1, &stmt, NULL); 

		sqlite3_bind_int(stmt, 1, recs->hit);
		sqlite3_bind_int(stmt, 2, recs->comment);
		sqlite3_bind_int(stmt, 3, recs->id);
		
		sqlite3_step(stmt); 
		sqlite3_finalize(stmt); 
	}
	sqlite3_exec(db, "COMMIT;", 0, 0, NULL);
	sqlite3_close(db);

	log_finish();
	safe_free(conf.path);
	safe_free(conf.username);
	safe_free(conf.password);

	exit(0);
}

int main(int argc, char *argv[])
{	
	init();
	
	signal(SIGHUP,  term_handler);
	signal(SIGUSR1, term_handler);
	signal(SIGTERM, term_handler);
	signal(SIGCHLD, term_handler);

    while (FCGI_Accept() >= 0)
	{
		mspace mp;
		int    n, i;
		bool   lookup;
		struct env_t *_SERVER  = NULL;
		const  char  *type     = NULL;
		const  char  *module   = NULL;
		const  char  *action   = NULL;

		
		lookup  = false;
		mp      = create_mspace(2048, 0);
		_SERVER = mspace_malloc(mp, sizeof(struct env_t));
		if(_SERVER == NULL)
			continue;


		_SERVER->mp       = mp;
		_SERVER->_GET     = tclistnew2(6);
		_SERVER->_FILES   = tclistnew2(6);
		_SERVER->_POST    = tcmapnew2(6);
		_SERVER->_COOKIE  = tcmapnew2(6);

		fcgi_init_headers(_SERVER);

		n = tclistnum(_SERVER->_GET);
		if(n < 2)
		{
			puts("Location:/\r\n\r\n");
			tclistdel(_SERVER->_GET);
			tclistdel(_SERVER->_FILES);
			tcmapdel(_SERVER->_POST);
			tcmapdel(_SERVER->_COOKIE);
			destroy_mspace(mp);
			continue;
		}
		
		type = tclistval2(_SERVER->_GET, 0);
		
		//后台管理界面
		if(strcasecmp(type, "panel") == 0)
		{
			module = tclistval2(_SERVER->_GET, 1);
			action = tclistval2(_SERVER->_GET, 2);
			
			//检查用户已登陆, user模块不用检查
			if((strcasecmp(module, "user") != 0) && (!panel_user_is_login(_SERVER)))
			{
				puts("Content-type: text/html\r\n\r\n<script>alert('您还未登陆!');top.location.href='/panel-user-login.html';</script>");

				tclistdel(_SERVER->_GET);
				tclistdel(_SERVER->_FILES);
				tcmapdel(_SERVER->_POST);
				tcmapdel(_SERVER->_COOKIE);
				destroy_mspace(mp);
				continue;
			}
			for (i = 0; panel_cmd_t[i].module; i++) 
			{
				if ((strcasecmp(module, panel_cmd_t[i].module) == 0) && (strcasecmp(action, panel_cmd_t[i].action) == 0)) 
				{
					panel_cmd_t[i].handler(_SERVER);
					lookup = true;
					break;
				}
			}
		}
		else
		{
			//前台界面
			module = tclistval2(_SERVER->_GET, 0);
			action = tclistval2(_SERVER->_GET, 1);
			for (i = 0; blog_cmd_t[i].module; i++) 
			{
				if ((strcasecmp(module, blog_cmd_t[i].module) == 0) && (strcasecmp(action, blog_cmd_t[i].action) == 0)) 
				{
					blog_cmd_t[i].handler(_SERVER);
					lookup = true;
					break;
				}
			}
		}
		

		if(!lookup) puts("Content-type: text/html\r\n\r\n<script>alert('not found module');window.location.href='/';</script>");

		tclistdel(_SERVER->_GET);
		tclistdel(_SERVER->_FILES);
		tcmapdel(_SERVER->_POST);
		tcmapdel(_SERVER->_COOKIE);
		destroy_mspace(mp);
    }
	

    return 0;
}

static void sqlite3_substring(sqlite3_context *context, int argc, sqlite3_value **argv)
{
	int    len;
	const  u_char *text;
	
	switch(sqlite3_value_type(argv[0]))
	{
		case SQLITE_TEXT:{
			text = sqlite3_value_text(argv[0]);
			len  = sqlite3_value_int(argv[1]);
			
			sqlite3_result_text(context, (char *)&text[0], len, SQLITE_TRANSIENT);
		}
		break;

		default: 
			sqlite3_result_null(context);
		break;
	}
}

static void init()
{
	int            rc;
	pthread_t      id;
	pthread_attr_t attr;
		
	log_init();
	parse_conf();
	
	log_debug("blog.init.path = %s\r\n", conf.path);
	rc = sqlite3_open(conf.path, &db);
	if(rc != SQLITE_OK) 
	{
		log_debug("sqlite3_open error = %d\r\n", rc);
        sqlite3_close(db);
        exit(0);
    }

	sqlite3_create_function(db, "substring", 2, SQLITE_UTF8, NULL, &sqlite3_substring, NULL, NULL);
	
	pthread_create(&id, NULL, blog_click_sync, NULL);

	init_hex_table();
}




