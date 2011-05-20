#include "blog.h"


void assign_tpl_list(tpl_t *tpl, char *sql)
{
	sqlite3_stmt *stmt;
	int          i, rc, type;

	rc = sqlite3_prepare(db, sql, strlen(sql), &stmt, NULL);
	rc = sqlite3_step(stmt);
	
	tpl_select_section(tpl, "data");
	while(rc == SQLITE_ROW)
	{
		for(i=0; i<sqlite3_column_count(stmt); i++)
		{
			type = sqlite3_column_type(stmt, i);
			switch(type)
			{
				case SQLITE_INTEGER:
					tpl_set_field_uint(tpl, sqlite3_column_name(stmt,i), sqlite3_column_int(stmt, i));
					break;
				case SQLITE_TEXT:
					tpl_set_field(tpl, sqlite3_column_name(stmt,i) ,sqlite3_column_text(stmt,i), strlen(sqlite3_column_text(stmt,i)));
					break;
			}
		}

		tpl_append_section(tpl);
		rc = sqlite3_step(stmt);
	}

	tpl_deselect_section(tpl);
	sqlite3_finalize(stmt);
}

void blog_article_list(struct env_t *_SERVER)
{
	FILE   *fp;
	tpl_t  *tpl;
	char   *html;
	char   url[128], path[128];
	int    rc, i, type, len, total;
	

	uint limit[2], cur;
	sqlite3_stmt *stmt;
	const char   *sortlevel, *page;
	

	total     = 0;
	sortlevel = tclistval2(_SERVER->_GET, 2);
	page      = tclistval2(_SERVER->_GET, 3);
	cur       = (page) ? atoi(page) : 1;

#ifdef _BUILD_HTML

	memset(path, 0, sizeof(path));

	snprintf(path, sizeof(path), "./html/list/%s/%u.htm", sortlevel, cur);
	if(file_exists(path))
	{
		printf("X-Accel-Redirect: %s\r\n\r\n", path+1);
		return ;
	}
#endif

	tpl = tpl_alloc();
	if (tpl_load(tpl, "./templets/blog/article_list.html") != TPL_OK)
    {
		printf("Content-type: text/html\r\n\r\n./templets/panel/article_list.html Error loading template file!");
		tpl_free(tpl);
		return ;
    }


	
	memset(url, 0, sizeof(url));
	if((sortlevel) && (strcmp(sortlevel, "all") == 0))
	{
		strcpy(url, "/article-list-all");
		rc   = sqlite3_prepare(db, "SELECT COUNT(*) AS c FROM article;", -1, &stmt, NULL);
	}
	else if((sortlevel) && (is_alpha(sortlevel)) && (strcmp(sortlevel, "html") != 0))
	{
		rc   = sqlite3_prepare(db, "SELECT COUNT(*) AS c FROM article WHERE catid = ?;", -1, &stmt, NULL);
		sqlite3_bind_text(stmt, 1, sortlevel, -1, SQLITE_STATIC);
		
		snprintf(url, sizeof(url), "/article-list-%s", sortlevel);
	}
	else
	{
		strcpy(url, "/article-list-all");
		rc   = sqlite3_prepare(db, "SELECT COUNT(*) AS c FROM article;", -1, &stmt, NULL);
	}

	//获取总数据集大小
	rc = sqlite3_step(stmt);
	if(rc == SQLITE_ROW)
	{
		total = sqlite3_column_int(stmt, 0);
	}
	sqlite3_finalize(stmt);

	//分页
	pager(tpl, total, conf.page.blog, cur, url, limit);

	tpl_set_field_int_global(tpl, "total", total);

	if((sortlevel) && (strcmp(sortlevel, "all") == 0))
	{
		rc = sqlite3_prepare(db, "SELECT art.filename, art.comment_num, art.hit, art.art_id, art.title,  substring(art.content, art.position) AS c, datetime(art.post_time, 'unixepoch') AS dt, cat.sortname, cat.sortdir FROM article AS art LEFT JOIN category AS cat ON cat.sortdir = art.catid ORDER BY art.art_id DESC LIMIT ?, ?;", -1, &stmt, NULL);
		
		sqlite3_bind_int(stmt, 1, limit[0]);
		sqlite3_bind_int(stmt, 2, limit[1]);
	}
	else if((sortlevel) && (is_alpha(sortlevel)) && (strcmp(sortlevel, "html") != 0))
	{
		rc = sqlite3_prepare(db, "SELECT art.filename, art.comment_num, art.hit, art.art_id, art.title,  substring(art.content, art.position) AS c, datetime(art.post_time, 'unixepoch') AS dt, cat.sortname, cat.sortdir FROM article AS art LEFT JOIN category AS cat ON cat.sortdir = art.catid WHERE art.catid = ? ORDER BY art.art_id DESC LIMIT ?, ?;", -1, &stmt, NULL);
		
		sqlite3_bind_text(stmt, 1, sortlevel, -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt,  2, limit[0]);
		sqlite3_bind_int(stmt,  3, limit[1]);
	}
	else
	{
		rc = sqlite3_prepare(db, "SELECT art.filename, art.comment_num, art.hit, art.art_id, art.title,  substring(art.content, art.position) AS c, datetime(art.post_time, 'unixepoch') AS dt, cat.sortname, cat.sortdir FROM article AS art LEFT JOIN category AS cat ON cat.sortdir = art.catid ORDER BY art.art_id DESC LIMIT ?, ?;", -1, &stmt, NULL);
		
		sqlite3_bind_int(stmt, 1, limit[0]);
		sqlite3_bind_int(stmt, 2, limit[1]);
	}
	
	
	
	tpl_select_section(tpl, "data");
	while((rc = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		for(i=0; i<sqlite3_column_count(stmt); i++)
		{
			type = sqlite3_column_type(stmt, i);
			switch(type)
			{
				case SQLITE_INTEGER:
					tpl_set_field_uint(tpl, sqlite3_column_name(stmt,i), sqlite3_column_int(stmt, i));
					break;
				case SQLITE_TEXT:
					tpl_set_field(tpl, sqlite3_column_name(stmt,i) ,sqlite3_column_text(stmt,i), strlen(sqlite3_column_text(stmt,i)));
					break;
			}
		}

		tpl_append_section(tpl);
	}

	tpl_deselect_section(tpl);
	sqlite3_finalize(stmt);
	
	//加载友情链接
	assign_tpl_friendlink(tpl);

	//获取分类数据
	assign_tpl_category(tpl);
	
	//加载推荐数据
	assign_tpl_recommend(tpl);

	html = mspace_malloc(_SERVER->mp, tpl_length(tpl) + 1);
    tpl_get_content(tpl, html);
	
	printf("Content-type: text/html\r\n\r\n%s", html);

#ifdef _BUILD_HTML
	fp = fopen(path, "wb");
	if(fp)
	{
		fwrite(html, sizeof(char), tpl_length(tpl), fp);
		fclose(fp);
	}
	chmod(path, S_IRWXU | S_IRWXG | S_IRWXO);
#endif

	tpl_free(tpl);
	mspace_free(_SERVER->mp, html);
	
	tpl  = NULL;
	
}

void blog_article_read(struct env_t *_SERVER)
{
	FILE         *fp;
	tpl_t        *tpl;
	sqlite3_stmt *stmt;
	int          i, rc, type, len;
	
	char  *html, *sql, path[128];
	const char *id, *sortlevel;
	
	
	sortlevel = tclistval2(_SERVER->_GET, 2);
	id        = tclistval2(_SERVER->_GET, 3);

	if(!sortlevel || !is_alnum(sortlevel))
	{
		puts("Content-type: text/html\r\n\r\n<script>alert('分类参数错误');window.location.href='/';</script>");
		return ;
	}
	if(!id || !is_digit(id))
	{
		puts("Content-type: text/html\r\n\r\n<script>alert('请您选择要阅读的文章');window.location.href='/';</script>");
		return ;
	}



#ifdef _BUILD_HTML
	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "./html/article/%s/%s.htm", sortlevel, id);
	if(file_exists(path))
	{
		printf("X-Accel-Redirect: %s\r\n\r\n", path+1);
		return ;
	}
#endif

	tpl = tpl_alloc();
	if (tpl_load(tpl, "./templets/blog/article_read.html") != TPL_OK)
    {
		printf("Content-type: text/html\r\n\r\n./templets/blog/article_read.html Error loading template file!");
		tpl_free(tpl);
		return ;
    }
	
	rc = sqlite3_prepare(db, "SELECT art.keyword, art.art_id, art.title, art.content, datetime(art.post_time, 'unixepoch') AS dt, cat.sortname, cat.sortdir FROM article AS art LEFT JOIN category AS cat ON cat.sortdir = art.catid WHERE art.art_id = ?;", -1, &stmt, NULL);
	sqlite3_bind_text(stmt, 1, id, -1, SQLITE_STATIC);

	rc = sqlite3_step(stmt);
	
	if(rc == SQLITE_ROW)
	{
		for(i=0; i<sqlite3_column_count(stmt); i++)
		{
			type = sqlite3_column_type(stmt, i);
			switch(type)
			{
				case SQLITE_INTEGER:
					tpl_set_field_uint_global(tpl, sqlite3_column_name(stmt,i), sqlite3_column_int(stmt, i));
					break;
				case SQLITE_TEXT:
					tpl_set_field_global(tpl, sqlite3_column_name(stmt,i) ,sqlite3_column_text(stmt,i), strlen(sqlite3_column_text(stmt,i)));
					break;
			}
		}
	}

	sqlite3_finalize(stmt);


	//加载友情链接
	assign_tpl_friendlink(tpl);

	//获取分类数据
	assign_tpl_category(tpl);
	
	//加载推荐数据
	assign_tpl_recommend(tpl);

	html = mspace_malloc(_SERVER->mp, tpl_length(tpl) + 1);
    tpl_get_content(tpl, html);
	
	printf("Content-type: text/html\r\n\r\n%s", html);

#ifdef _BUILD_HTML	
	fp = fopen(path, "wb");
	if(fp)
	{
		fwrite(html, sizeof(char), tpl_length(tpl), fp);
		fclose(fp);
	}
	chmod(path, S_IRWXU | S_IRWXG | S_IRWXO);
#endif

	tpl_free(tpl);
	mspace_free(_SERVER->mp, html);
	
	tpl  = NULL;
}

/*
void blog_article_index(struct env_t *_SERVER)
{
	uint  total;
	tpl_t *tpl;
	char  *html;
	FILE   *fp;

#ifdef _BUILD_HTML
	if(file_exists("./html/index.html"))
	{
		puts("X-Accel-Redirect: /html/index.html\r\n\r\n");
		return ;
	}
#endif

	tpl = tpl_alloc();
	if (tpl_load(tpl, "./templets/blog/article_list.html") != TPL_OK)
    {
		puts("Content-type: text/html\r\n\r\n./templets/blog/article_list.html Error loading template file!");
		tpl_free(tpl);
		return ;
    }
	
	//获取总数据集大小
	total = dataset_count("SELECT COUNT(*) AS c FROM article");
	
	//加载友情链接
	assign_tpl_friendlink(tpl);

	//获取分类数据
	assign_tpl_category(tpl);
	//加载推荐数据
	assign_tpl_recommend(tpl);

	assign_tpl_list(tpl, "SELECT art.filename, art.art_id, art.title, substring(art.content, art.position) AS c, datetime(art.post_time, 'unixepoch') AS dt, cat.sortname, cat.sortdir FROM article AS art LEFT JOIN category AS cat ON cat.sortdir = art.catid  ORDER BY art.art_id DESC LIMIT 0, 20;");

	html = mspace_malloc(_SERVER->mp, tpl_length(tpl) + 1);
    tpl_get_content(tpl, html);
	
	printf("Content-type: text/html\r\n\r\n%s", html);

#ifdef _BUILD_HTML	
	fp = fopen("./html/index.html", "wb");
	if(fp)
	{
		fwrite(html, sizeof(char), tpl_length(tpl), fp);
		fclose(fp);
	}
#endif

	tpl_free(tpl);
	mspace_free(_SERVER->mp, html);
	
	tpl  = NULL;
}
*/

