#include "blog.h"

void panel_category_move(struct env_t *_SERVER)
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

	rc  = sqlite3_prepare(db, "UPDATE category SET pos = ? WHERE sid = ?;", -1, &stmt, NULL); 
	
	sqlite3_bind_int(stmt, 1, atoi(pos));
	sqlite3_bind_int(stmt, 2, atoi(id));

	
	sqlite3_step(stmt); 
	sqlite3_finalize(stmt); 

	puts("Content-type: text/html\r\n\r\nok");
}

void panel_category_list(struct env_t *_SERVER)
{
	tpl_t        *tpl;
	char         *html;
	sqlite3_stmt *stmt;
	int          i, rc, type;


	tpl = tpl_alloc();
	if (tpl_load(tpl, "./templets/panel/category_list.html") != TPL_OK)
    {
		printf("Content-type: text/html\r\n\r\n./templets/panel/category_list.html Error loading template file!");
		tpl_free(tpl);
		return ;
    }
	
	rc = sqlite3_prepare(db, "SELECT * FROM category ORDER BY pos ASC;", -1, &stmt, NULL);
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

void panel_category_insert(struct env_t *_SERVER)
{
	char         path[128];
	char         *html;
	tpl_t        *tpl;
	sqlite3_stmt *stmt;
	int          rc, i;
	const char   *sortname, *sortdir, *pos;


	if(tcmapget2(_SERVER->_POST, "Category_Insert"))
	{
		pos       = tcmapget2(_SERVER->_POST, "pos");
		sortdir   = tcmapget2(_SERVER->_POST, "sortdir");
		sortname  = tcmapget2(_SERVER->_POST, "sortname");
		
		rc  = sqlite3_prepare(db, "INSERT INTO category(sortdir, sortname, pos) VALUES(?, ?, ?);", -1, &stmt, NULL); 
		
		sqlite3_bind_text(stmt, 1, sortdir,  -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, sortname, -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 3, pos,      -1, SQLITE_STATIC);

		
		sqlite3_step(stmt); 
		sqlite3_finalize(stmt); 
		
		memset(&path, 0, sizeof(path));
		sprintf(path, "./html/list/%s/", sortdir);
		mkdir(path, 0777); 

		memset(&path, 0, sizeof(path));
		sprintf(path, "./html/article/%s/", sortdir);
		mkdir(path, 0777); 

		puts("Content-type: text/html\r\n\r\n<script>alert('添加成功!');window.location.href='/panel-category-list.html';</script>");

		return ;
	}

	tpl = tpl_alloc();
	if (tpl_load(tpl, "./templets/panel/category_insert.html") != TPL_OK)
    {
		printf("Content-type: text/html\r\n\r\n./templets/panel/category_update.html Error loading template file!");
        tpl_free(tpl);
		return ;
    }

	//获取分类数据
	assign_tpl_category(tpl);

	html = mspace_malloc(_SERVER->mp, tpl_length(tpl) + 1);
    tpl_get_content(tpl, html);
	
	printf("Content-type: text/html\r\n\r\n%s", html);

	tpl_free(tpl);
	mspace_free(_SERVER->mp, html);
	
	tpl  = NULL;
}

void panel_category_delete(struct env_t *_SERVER)
{
	int        rc;
	char       *addr;
	const char *id;
	
	sqlite3_stmt *stmt;

	id = tclistval2(_SERVER->_GET, 3);
	if(id == NULL || !is_digit(id))
	{
		puts("Content-type: text/html\r\n\r\n<script>alert('请您选择要删除的分类');window.location.href='/panel-category-list.html';</script>");
		return ;
	}
		
	rc = sqlite3_prepare(db, "DELETE FROM category WHERE sid = ?;", -1, &stmt, NULL); 

	sqlite3_bind_int(stmt, 1, atoi(id));
	
	sqlite3_step(stmt); 
	sqlite3_finalize(stmt); 
	
	addr = getenv("HTTP_REFERER");
	if(addr)
		printf("Content-type: text/html\r\n\r\n<script>alert('删除已完成');window.location.href='%s';</script>", addr);
	else
		puts("Content-type: text/html\r\n\r\n<script>alert('删除已完成');window.location.href='/panel-category-list.html';</script>");
}

void panel_category_update(struct env_t *_SERVER)
{
	int          i, rc, type;
	tpl_t        *tpl;
	char         *html;
	char         oldpath[128], newpath[128];
	sqlite3_stmt *stmt;

	const char   *id, *sortname, *sortdir, *old_dir, *pos;

	if(tcmapget2(_SERVER->_POST, "Category_Update"))
	{
		id       = tcmapget2(_SERVER->_POST, "id");
		pos      = tcmapget2(_SERVER->_POST, "pos");
		old_dir  = tcmapget2(_SERVER->_POST, "old_dir");
		sortname = tcmapget2(_SERVER->_POST, "sortname");
		sortdir  = tcmapget2(_SERVER->_POST, "sortdir");
		
		if(id == NULL || !is_digit(id))
		{
			puts("Content-type: text/html\r\n\r\n<script>alert('请您选择要编辑的分类ID');window.location.href='/panel-category-list.html';</script>");
			return ;
		}

		if(old_dir && (strcmp(old_dir, sortdir) != 0))
		{
			memset(oldpath, 0, sizeof(oldpath));
			memset(newpath, 0, sizeof(newpath));
			
			//修改cache 列表页 目录名字
			snprintf(oldpath, sizeof(oldpath), "./html/list/%s/", old_dir);
			snprintf(newpath, sizeof(newpath), "./html/list/%s/", sortdir);
			
			if(is_dir(oldpath))
			{
				rename(oldpath, newpath);
			}

			memset(oldpath, 0, sizeof(oldpath));
			memset(newpath, 0, sizeof(newpath));
			
			//修改cache 文章页 目录名字
			snprintf(oldpath, sizeof(oldpath), "./html/article/%s/", old_dir);
			snprintf(newpath, sizeof(newpath), "./html/article/%s/", sortdir);
			
			if(is_dir(oldpath))
			{
				rename(oldpath, newpath);
			}
		}

		rc  = sqlite3_prepare(db, "UPDATE category SET sortname = ?, sortdir = ?, pos = ? WHERE sid = ?;", -1, &stmt, NULL); 
		
		sqlite3_bind_text(stmt, 1, sortname,  -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, sortdir,   -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 3, atoi(pos));
		sqlite3_bind_int(stmt, 4, atoi(id));

		
		sqlite3_step(stmt); 
		sqlite3_finalize(stmt); 

		puts("Content-type: text/html\r\n\r\n<script>alert('编辑成功!');window.location.href='/panel-category-list.html';</script>");
		return ;
	}

	id = tclistval2(_SERVER->_GET, 3);
	if(id == NULL || !is_digit(id))
	{
		puts("Content-type: text/html\r\n\r\n<script>alert('请您选择要编辑的分类');window.location.href='/panel-category-list.html';</script>");
		return ;
	}
	tpl = tpl_alloc();
	if (tpl_load(tpl, "./templets/panel/category_update.html") != TPL_OK)
    {
		printf("Content-type: text/html\r\n\r\n./templets/panel/category_update.html Error loading template file!");
        tpl_free(tpl);
		return ;
    }

	//加载需要编辑的数据
	rc = sqlite3_prepare(db, "SELECT * FROM category WHERE sid = ?", -1, &stmt, NULL);
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

