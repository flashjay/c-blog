#include "blog.h"


void panel_friendlink_move(struct env_t *_SERVER)
{
	int        rc;
	const char *id, *pos;
	
	sqlite3_stmt *stmt;

	id  = tclistval2(_SERVER->_GET, 3);
	pos = tclistval2(_SERVER->_GET, 4);
	if(id == NULL || !is_digit(id))
	{
		puts("Content-type: text/html\r\n\r\nID不是数字");
		return ;
	}
	else if(pos == NULL || !is_digit(pos))
	{
		puts("Content-type: text/html\r\n\r\n序号不是数字");
		return ;
	}

	rc  = sqlite3_prepare(db, "UPDATE friendlink SET pos = ? WHERE id = ?;", -1, &stmt, NULL); 
	
	sqlite3_bind_int(stmt, 1, atoi(pos));
	sqlite3_bind_int(stmt, 2, atoi(id));

	
	sqlite3_step(stmt); 
	sqlite3_finalize(stmt); 

	puts("Content-type: text/html\r\n\r\nok");
}

void panel_friendlink_list(struct env_t *_SERVER)
{
	tpl_t        *tpl;
	char         *html;
	sqlite3_stmt *stmt;
	int          i, rc, type;


	tpl = tpl_alloc();
	if (tpl_load(tpl, "./templets/panel/friendlink_list.html") != TPL_OK)
    {
		printf("Content-type: text/html\r\n\r\n./templets/panel/friendlink_list.html Error loading template file!");
		tpl_free(tpl);
		return ;
    }
	
	rc = sqlite3_prepare(db, "SELECT id, site, url, pos, datetime(post_time, 'unixepoch') AS dt FROM friendlink ORDER BY pos ASC;", -1, &stmt, NULL);
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

void panel_friendlink_insert(struct env_t *_SERVER)
{
	char         *html;

	sqlite3_stmt *stmt;
	int          rc, i;
	const char   *site, *url, *pos;


	if(tcmapget2(_SERVER->_POST, "Friendlink_Insert"))
	{
		pos   = tcmapget2(_SERVER->_POST, "pos");
		url   = tcmapget2(_SERVER->_POST, "url");
		site  = tcmapget2(_SERVER->_POST, "site");
		
		rc  = sqlite3_prepare(db, "INSERT INTO friendlink(site, url, post_time, pos) VALUES(?, ?, ?, ?);", -1, &stmt, NULL); 
		
		sqlite3_bind_text(stmt, 1, site,  -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, url,   -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 3, time((time_t*)0));
		sqlite3_bind_int(stmt, 4, atoi(pos));

		
		sqlite3_step(stmt); 
		sqlite3_finalize(stmt); 

		puts("Content-type: text/html\r\n\r\n<script>alert('添加成功!');window.location.href='/panel-friendlink-list.html';</script>");

		return ;
	}

	puts("X-Accel-Redirect: /templets/panel/friendlink_insert.htm\r\n\r\n");
}

void panel_friendlink_delete(struct env_t *_SERVER)
{
	int        rc;
	char       *addr;
	const char *id;
	
	sqlite3_stmt *stmt;

	id = tclistval2(_SERVER->_GET, 3);
	if(id == NULL || !is_digit(id))
	{
		puts("Content-type: text/html\r\n\r\n<script>alert('请您选择要删除的友情链接');window.location.href='/panel-friendlink-list.html';</script>");
		return ;
	}
		
	rc = sqlite3_prepare(db, "DELETE FROM friendlink WHERE id = ?;", -1, &stmt, NULL); 

	sqlite3_bind_int(stmt, 1, atoi(id));
	
	sqlite3_step(stmt); 
	sqlite3_finalize(stmt); 
	
	addr = getenv("HTTP_REFERER");
	if(addr)
		printf("Content-type: text/html\r\n\r\n<script>alert('删除已完成');window.location.href='%s';</script>", addr);
	else
		puts("Content-type: text/html\r\n\r\n<script>alert('删除已完成');window.location.href='/panel-friendlink-list.html';</script>");
}

void panel_friendlink_update(struct env_t *_SERVER)
{
	int          i, rc, type;
	tpl_t        *tpl;
	char         *html;
	sqlite3_stmt *stmt;

	const char   *id, *site, *url, *pos;;

	if(tcmapget2(_SERVER->_POST, "Friendlink_Update"))
	{
		id    = tcmapget2(_SERVER->_POST, "id");
		pos   = tcmapget2(_SERVER->_POST, "pos");
		url   = tcmapget2(_SERVER->_POST, "url");
		site  = tcmapget2(_SERVER->_POST, "site");
		
		rc  = sqlite3_prepare(db, "UPDATE friendlink SET site = ?, url = ?, pos = ? WHERE id = ?;", -1, &stmt, NULL); 
		
		sqlite3_bind_text(stmt, 1, site,  -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, url,   -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 3, atoi(pos));
		sqlite3_bind_int(stmt, 4, atoi(id));

		
		sqlite3_step(stmt); 
		sqlite3_finalize(stmt); 

		puts("Content-type: text/html\r\n\r\n<script>alert('编辑成功!');window.location.href='/panel-friendlink-list.html';</script>");
		return ;
	}

	id = tclistval2(_SERVER->_GET, 3);
	if(id == NULL || !is_digit(id))
	{
		puts("Content-type: text/html\r\n\r\n<script>alert('请您选择要编辑的友情链接');window.location.href='/panel-friendlink-list.html';</script>");
		return ;
	}
	tpl = tpl_alloc();
	if (tpl_load(tpl, "./templets/panel/friendlink_update.html") != TPL_OK)
    {
		printf("Content-type: text/html\r\n\r\n./templets/panel/friendlink_update.html Error loading template file!");
        tpl_free(tpl);
		return ;
    }

	//加载需要编辑的数据
	rc = sqlite3_prepare(db, "SELECT * FROM friendlink WHERE id = ?", -1, &stmt, NULL);
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

