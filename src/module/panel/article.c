#include "blog.h"


void make_article_index(struct env_t *_SERVER)
{
	uint  total;
	tpl_t *tpl;
	char  *html;
	FILE   *fp;

	int  rc, i, type;
	uint limit[2];
	sqlite3_stmt *stmt;

	tpl = tpl_alloc();
	if (tpl_load(tpl, "./templets/blog/article_list.html") != TPL_OK)
    {
		puts("Content-type: text/html\r\n\r\n./templets/blog/article_list.html Error loading template file!");
		tpl_free(tpl);
		return ;
    }
	
	//获取总数据集大小
	total = dataset_count("SELECT COUNT(*) AS c FROM article");
	
	tpl_set_field_int_global(tpl, "total", total);
	//分页
	pager(tpl, total, conf.page.blog, 1, "/article-list-all", limit);

	//获取分类数据
	assign_tpl_category(tpl);
	//加载推荐数据
	assign_tpl_recommend(tpl);

	//加载友情链接
	assign_tpl_friendlink(tpl);
	rc = sqlite3_prepare(db, "SELECT art.filename, art.comment_num, art.hit, art.art_id, art.title,  substring(art.content, art.position) AS c, datetime(art.post_time, 'unixepoch') AS dt, cat.sortname, cat.sortdir FROM article AS art LEFT JOIN category AS cat ON cat.sortdir = art.catid ORDER BY art.art_id DESC LIMIT ?;", -1, &stmt, NULL);
		
	sqlite3_bind_int(stmt, 1, conf.page.blog);


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

	html = mspace_malloc(_SERVER->mp, tpl_length(tpl) + 1);
    tpl_get_content(tpl, html);
	
	//printf("Content-type: text/html\r\n\r\n%s", html);
	
	fp = fopen("./index.htm", "wb");
	if(fp)
	{
		fwrite(html, sizeof(char), tpl_length(tpl), fp);
		fclose(fp);
	}
	
	chmod("./index.htm", S_IRWXU | S_IRWXG | S_IRWXO);
	
	tpl_free(tpl);
	mspace_free(_SERVER->mp, html);
	
	tpl  = NULL;
}

void panel_article_index(struct env_t *_SERVER)
{
	make_article_index(_SERVER);
	puts("Expires: 0\r\nCache-control: private\r\nX-Accel-Redirect: /templets/panel/article_index.htm\r\n\r\n");
}

int fn(const char *file, const struct stat *sb, int flag) 
{ 
     if(flag == FTW_F)   
		 remove(file);

     return 0; 
}

void panel_article_clean(struct env_t *_SERVER)
{
	ftw("./html", fn, 500); 
	puts("Expires: 0\r\nCache-control: private\r\nX-Accel-Redirect: /templets/panel/article_clean.htm\r\n\r\n");
}

void panel_article_list(struct env_t *_SERVER)
{
	tpl_t  *tpl;
	char   url[128];
	char   *html, *sql;
	int    rc, i, type, total;
	

	uint limit[2], cur;
	sqlite3_stmt   *stmt;
	const char     *sortlevel, *page;
	
	
	tpl = tpl_alloc();
	if (tpl_load(tpl, "./templets/panel/article_list.html") != TPL_OK)
    {
		printf("Content-type: text/html\r\n\r\n./templets/panel/article_list.html Error loading template file!");
		tpl_free(tpl);
		return ;
    }

	total     = 0;
	sortlevel = tclistval2(_SERVER->_GET, 3);
	page      = tclistval2(_SERVER->_GET, 4);

	memset(&url, 0, sizeof(url));
	if((sortlevel) && (strcmp(sortlevel, "all") == 0))
	{
		strcpy(url, "/panel-article-list-all");
		rc   = sqlite3_prepare(db, "SELECT COUNT(*) AS c FROM article;", -1, &stmt, NULL);
	}
	else if((sortlevel) && (is_alpha(sortlevel)) && (strcmp(sortlevel, "html") != 0))
	{
		rc   = sqlite3_prepare(db, "SELECT COUNT(*) AS c FROM article WHERE catid = ?;", -1, &stmt, NULL);
		sqlite3_bind_text(stmt, 1, sortlevel, -1, SQLITE_STATIC);
		
		snprintf(url, sizeof(url), "/panel-article-list-%s", sortlevel);
	}
	else
	{
		strcpy(url, "/panel-article-list-all");
		rc   = sqlite3_prepare(db, "SELECT COUNT(*) AS c FROM article;", -1, &stmt, NULL);
	}

	rc = sqlite3_step(stmt);
	if(rc == SQLITE_ROW)
	{
		total = sqlite3_column_int(stmt, 0);
	}
	sqlite3_finalize(stmt);
	
	
	cur = (page) ? atoi(page) : 1;

	//分页
	pager(tpl, total, conf.page.admin, cur, url, limit);

	tpl_set_field_int_global(tpl, "total", total);

	if((sortlevel) && (strcmp(sortlevel, "all") == 0))
	{
		rc = sqlite3_prepare(db, "SELECT art.comment_num, art.hit, art.art_id, art.title, art.content, datetime(art.post_time, 'unixepoch') AS dt, cat.sortname, cat.sortdir FROM article AS art LEFT JOIN category AS cat ON cat.sortdir = art.catid ORDER BY art.art_id DESC LIMIT ?, ?;", -1, &stmt, NULL);
		
		sqlite3_bind_int(stmt, 1, limit[0]);
		sqlite3_bind_int(stmt, 2, limit[1]);
	}
	else if((sortlevel) && (is_alpha(sortlevel)) && (strcmp(sortlevel, "html") != 0))
	{
		rc = sqlite3_prepare(db, "SELECT art.comment_num, art.hit, art.art_id, art.title, art.content, datetime(art.post_time, 'unixepoch') AS dt, cat.sortname, cat.sortdir FROM article AS art LEFT JOIN category AS cat ON cat.sortdir = art.catid WHERE art.catid = ? ORDER BY art.art_id DESC LIMIT ?, ?;", -1, &stmt, NULL);
		
		sqlite3_bind_text(stmt, 1, sortlevel, -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt,  2, limit[0]);
		sqlite3_bind_int(stmt,  3, limit[1]);
	}
	else
	{
		rc = sqlite3_prepare(db, "SELECT art.comment_num, art.hit, art.art_id, art.title, art.content, datetime(art.post_time, 'unixepoch') AS dt, cat.sortname, cat.sortdir FROM article AS art LEFT JOIN category AS cat ON cat.sortdir = art.catid ORDER BY art.art_id DESC LIMIT ?, ?;", -1, &stmt, NULL);
		
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
	

	html = mspace_malloc(_SERVER->mp, tpl_length(tpl) + 1);
    tpl_get_content(tpl, html);
	
	printf("Content-type: text/html\r\n\r\n%s", html);

	tpl_free(tpl);
	mspace_free(_SERVER->mp, html);
	
	tpl  = NULL;
	
}


void panel_article_insert(struct env_t *_SERVER)
{
	time_t       visit;
	tpl_t        *tpl;
	TCLIST       *arr;
	char         *html, path[128], buf[128];
	
	sqlite3_int64   id;
	sqlite3_stmt    *stmt;
	int             rc, i, pos, n, len;
	const char      *title, *sortlevel, *content, *recommend, *keyword, *filename, *file;

	if(tcmapget2(_SERVER->_POST, "Article_Insert"))
	{
		visit     = time((time_t*)0);
		file      = tcmapget2(_SERVER->_COOKIE, "file");
		title     = tcmapget2(_SERVER->_POST,   "title");
		content   = tcmapget2(_SERVER->_POST,   "content");
		keyword   = tcmapget2(_SERVER->_POST,   "keyword");
		filename  = tcmapget2(_SERVER->_POST,   "filename");
		sortlevel = tcmapget2(_SERVER->_POST,   "sortlevel");
		recommend = tcmapget2(_SERVER->_POST,   "recommend");
		
		pos = strpos(content, "<!-- idx -->");
		pos = (pos == -1) ? strlen(content) : pos;
		rc  = sqlite3_prepare(db, "INSERT INTO article(title, content, catid, keyword, filename, recommend, post_time, position, hit, comment_num) VALUES(?, ?, ?, ?, ?, ?, ?, ?, 0, 0);", -1, &stmt, NULL); 
		
		sqlite3_bind_text(stmt, 1, title,     -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, content,   -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 3, sortlevel, -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 4, keyword,   -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 5, filename,  -1, SQLITE_STATIC);

		sqlite3_bind_int(stmt, 6, atoi(recommend));
		sqlite3_bind_int(stmt, 7, visit);
		sqlite3_bind_int(stmt, 8, pos);

		
		sqlite3_step(stmt); 
		sqlite3_finalize(stmt); 
		
		id = sqlite3_last_insert_rowid(db);
				
		if(file)
		{
			memset(buf,  0, sizeof(buf));
			memset(path, 0, sizeof(path));
			
			strftime(buf, sizeof(buf), "%Y/%m/%d", localtime(&visit));

			//如果有上传的文件,就把上传的文件,从临时目录移动目标目录
			len = snprintf(path, sizeof(path), "./attachment/%s/%llu/", buf, id);
			
			if(!is_dir(path)) mkpath(_SERVER->mp, path, 0777);
			if(!strchr(file, '|'))
			{
				memset(buf,  0, sizeof(buf));
				snprintf(buf, sizeof(buf), "./files/%s", file);
				strcat(path, file);
				
			//	log_debug("panel_article_insert buf = %s\tpath = %s\r\n", buf, path);
				if(file_exists(buf)) rename(buf, path);
			}
			else
			{
				arr = explode("|", (char *)file);
				n   = tclistnum(arr);
				for(i=0; i<n; i++)
				{
					memset(buf,  0, sizeof(buf));
					filename = tclistval2(arr, i);
					snprintf(buf, sizeof(buf), "./files/%s", filename);

					path[len] = '\0';
					strcat(path, filename);

		//			log_debug("panel_article_insert explode buf = %s\tpath = %s\r\n", buf, path);

					if(file_exists(buf)) rename(buf, path);
				}
				tclistdel(arr);
			}
			
			path[len] = '\0';
			

			//更新图片附件的 路径
			rc = sqlite3_prepare(db, "UPDATE article SET content = replace(content, '/files/', ?) WHERE art_id = ?;", -1, &stmt, NULL);
			
			sqlite3_bind_text(stmt,  1, path+1, -1, SQLITE_STATIC);
			sqlite3_bind_int64(stmt, 2, id);
			sqlite3_step(stmt); 
			sqlite3_finalize(stmt); 
			
		}
		

#ifdef _BUILD_HTML
		memset(path, 0, sizeof(path));
		snprintf(path, sizeof(path), "./html/list/%s", sortlevel);
		ftw(path, fn, 500);
#endif
		make_article_index(_SERVER);
		printf("Content-type: text/html\r\n\r\n<script>alert('添加成功!');window.location.href='/panel-article-list-%s.html';</script>", sortlevel);

		return ;
	}
	

	tpl = tpl_alloc();
	if (tpl_load(tpl, "./templets/panel/article_insert.html") != TPL_OK)
    {
		puts("Content-type: text/html\r\n\r\n./templets/panel/article_insert.html Error loading template file!");
        tpl_free(tpl);
		return ;
    }

	rc = sqlite3_prepare(db, "SELECT sortname, sortdir FROM category", -1, &stmt, NULL);
	
	tpl_select_section(tpl, "classic");
	while((rc = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		for(i=0; i<sqlite3_column_count(stmt); i++)
		{
			tpl_set_field(tpl, sqlite3_column_name(stmt,i) ,sqlite3_column_text(stmt,i), strlen(sqlite3_column_text(stmt,i)));
		}

		tpl_append_section(tpl);
	}

	tpl_deselect_section(tpl);
	sqlite3_finalize(stmt);


	html = mspace_malloc(_SERVER->mp, tpl_length(tpl) + 1);
    tpl_get_content(tpl, html);

	printf("Content-type: text/html\r\n\r\n%s", html);
	
	tpl_free(tpl);
	mspace_free(_SERVER->mp, html);
	
	tpl  = NULL;
}


void panel_article_update(struct env_t *_SERVER)
{
	tpl_t        *tpl;
	TCLIST       *arr;
	char         *html, path[128], buf[128];

	sqlite3_stmt *stmt;
	time_t       visit;
	int          rc, i, type, pos, len, n;
	const char   *id, *title, *sortlevel, *content, *recommend, *keyword, *filename, *file, *post_time;


	if(tcmapget2(_SERVER->_POST, "Article_Update"))
	{
		file      = tcmapget2(_SERVER->_COOKIE, "file");
		id        = tcmapget2(_SERVER->_POST, "id");
		title     = tcmapget2(_SERVER->_POST, "title");
		content   = tcmapget2(_SERVER->_POST, "content");
		keyword   = tcmapget2(_SERVER->_POST, "keyword");
		filename  = tcmapget2(_SERVER->_POST, "filename");
		sortlevel = tcmapget2(_SERVER->_POST, "sortlevel");
		recommend = tcmapget2(_SERVER->_POST, "recommend");
		post_time = tcmapget2(_SERVER->_POST, "post_time");

		pos = strpos(content, "<!-- idx -->");
		pos = (pos == -1) ? strlen(content) : pos;
		rc = sqlite3_prepare(db, "UPDATE article SET title = ?, content = ?, catid = ?, keyword = ?, filename = ?, recommend = ?, position = ? WHERE art_id = ?;", -1, &stmt, NULL); 
		
		sqlite3_bind_text(stmt, 1, title,     -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, content,   -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 3, sortlevel, -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 4, keyword,   -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 5, filename,  -1, SQLITE_STATIC);

		sqlite3_bind_int(stmt, 6, atoi(recommend));
		sqlite3_bind_int(stmt, 7, pos);
		sqlite3_bind_int(stmt, 8, atoi(id));
		

		sqlite3_step(stmt); 
		sqlite3_finalize(stmt); 

		visit = atoi(post_time);

		if(file)
		{
			memset(buf,  0, sizeof(buf));
			memset(path, 0, sizeof(path));
			
			strftime(buf, sizeof(buf), "%Y/%m/%d", localtime(&visit));

			//如果有上传的文件,就把上传的文件,从临时目录移动目标目录
			len = snprintf(path, sizeof(path), "./attachment/%s/%llu/", buf, id);
			
			if(!is_dir(path)) mkpath(_SERVER->mp, path, 0777);
			if(!strchr(file, '|'))
			{
				memset(buf,  0, sizeof(buf));
				snprintf(buf, sizeof(buf), "./files/%s", file);
				strcat(path, file);
				
			//	log_debug("panel_article_insert buf = %s\tpath = %s\r\n", buf, path);
				if(file_exists(buf)) rename(buf, path);
			}
			else
			{
				arr = explode("|", (char *)file);
				n   = tclistnum(arr);
				for(i=0; i<n; i++)
				{
					memset(buf,  0, sizeof(buf));
					filename = tclistval2(arr, i);
					snprintf(buf, sizeof(buf), "./files/%s", filename);

					path[len] = '\0';
					strcat(path, filename);

		//			log_debug("panel_article_insert explode buf = %s\tpath = %s\r\n", buf, path);

					if(file_exists(buf)) rename(buf, path);
				}
				tclistdel(arr);
			}
			
			path[len] = '\0';
			

			//更新图片附件的 路径
			rc = sqlite3_prepare(db, "UPDATE article SET content = replace(content, '/files/', ?) WHERE art_id = ?;", -1, &stmt, NULL);
			
			sqlite3_bind_text(stmt, 1, path+1,   -1, SQLITE_STATIC);
			sqlite3_bind_int(stmt, 2, atoi(id));
			sqlite3_step(stmt); 
			sqlite3_finalize(stmt); 
			
		}

#ifdef _BUILD_HTML
		memset(path, 0, sizeof(path));
		snprintf(path, sizeof(path), "./html/article/%s/%s.htm", sortlevel, id);
		if(file_exists(path))
			remove(path);

		memset(path, 0, sizeof(path));
		snprintf(path, sizeof(path), "./html/list/%s", sortlevel);
		ftw(path, fn, 500);
#endif
		printf("Content-type: text/html\r\n\r\n<script>alert('编辑成功!');window.location.href='/panel-article-list-%s.html';</script>", sortlevel);

		return ;
	}
	
	id = tclistval2(_SERVER->_GET, 3);
	if(id == NULL || !is_digit(id))
	{
		puts("Content-type: text/html\r\n\r\n<script>alert('请您选择要更新的文章');window.location.href='/panel-article-list-all.html';</script>");
		return ;
	}

	tpl = tpl_alloc();
	if (tpl_load(tpl, "./templets/panel/article_update.html") != TPL_OK)
    {
		printf("Content-type: text/html\r\n\r\n./templets/panel/article_update.html Error loading template file!");
        tpl_free(tpl);
		return ;
    }
	
	//加载分类的数据
	rc = sqlite3_prepare(db, "SELECT sortname, sortdir FROM category", -1, &stmt, NULL);
	
	tpl_select_section(tpl, "classic");
	while((rc = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		for(i=0; i<sqlite3_column_count(stmt); i++)
		{
			tpl_set_field(tpl, sqlite3_column_name(stmt,i) ,sqlite3_column_text(stmt,i), strlen(sqlite3_column_text(stmt,i)));
		}
		tpl_append_section(tpl);
	}

	tpl_deselect_section(tpl);
	sqlite3_finalize(stmt);
	
	//加载需要编辑的数据
	rc = sqlite3_prepare(db, "SELECT * FROM article WHERE art_id = ?", -1, &stmt, NULL);
	sqlite3_bind_int(stmt, 1, atoi(id));

	while((rc = sqlite3_step(stmt)) == SQLITE_ROW)
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


	html = mspace_malloc(_SERVER->mp, tpl_length(tpl) + 1);
    tpl_get_content(tpl, html);

	printf("Content-type: text/html\r\n\r\n%s", html);
	
	tpl_free(tpl);
	mspace_free(_SERVER->mp, html);
	
	tpl  = NULL;
}

void panel_article_delete(struct env_t *_SERVER)
{
	int        rc;
	char       *addr;
	const char *id;
	
	sqlite3_stmt *stmt;

	id = tclistval2(_SERVER->_GET, 3);
	if(id == NULL || !is_digit(id))
	{
		puts("Content-type: text/html\r\n\r\n<script>alert('请您选择要删除的文章');window.location.href='/panel-article-list-all.html';</script>");
		return ;
	}
	
	//tcmapout2(maps, id);
	
	rc = sqlite3_prepare(db, "DELETE FROM article WHERE art_id = ?;", -1, &stmt, NULL); 

	sqlite3_bind_int(stmt, 1, atoi(id));
	
	sqlite3_step(stmt); 
	sqlite3_finalize(stmt); 
	
	addr = getenv("HTTP_REFERER");
	if(addr)
		printf("Content-type: text/html\r\n\r\n<script>alert('删除已完成');window.location.href='%s';</script>", addr);
	else
		puts("Content-type: text/html\r\n\r\n<script>alert('删除已完成');window.location.href='/panel-article-list-all.html';</script>");

}

void panel_article_upload(struct env_t *_SERVER)
{
	const char *path, *file;

	if(tclistnum(_SERVER->_FILES) == 0)
	{
		puts("Content-type: text/html\r\n\r\n{\"error\":1, \"message\":\"文件上传失败\"}");
		return ;
	}
	
	file = tclistval2(_SERVER->_FILES, 0);
	path = tcmapget2(_SERVER->_COOKIE, "file");
	if(path == NULL)
	{
		printf("Set-Cookie: file=%s;\r\nContent-type: text/html\r\n\r\n{\"error\":0, \"url\":\"/files/%s\"}", file, file);
		return ;
	}

	printf("Set-Cookie: file=%s|%s;\r\nContent-type: text/html\r\n\r\n{\"error\":0, \"url\":\"/files/%s\"}", path, file, file);
}


