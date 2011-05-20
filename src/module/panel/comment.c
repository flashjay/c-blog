#include "blog.h"


void panel_comment_list(struct env_t *_SERVER)
{
	tpl_t  *tpl;
	char   url[128];
	char   *html, *sql;
	int    rc, i, type, total;
	

	uint limit[2], cur;
	sqlite3_stmt   *stmt;
	const char     *art_id, *page;

	art_id = tclistval2(_SERVER->_GET, 3);
	if(art_id == NULL || !is_digit(art_id))
	{
		puts("Content-type: text/html\r\n\r\n<script>alert('请您选择要查看的文章ID');</script>");
		return ;
	}

	tpl = tpl_alloc();
	if (tpl_load(tpl, "./templets/panel/comment_list.html") != TPL_OK)
    {
		printf("Content-type: text/html\r\n\r\n./templets/panel/comment_list.html Error loading template file!");
		tpl_free(tpl);
		return ;
    }

	rc = sqlite3_prepare(db, "SELECT id, author, replace(content, '<br />', ' ') AS c, datetime(post_time, 'unixepoch') AS dt, addr FROM comment WHERE article_id = ? ORDER BY id DESC", -1, &stmt, NULL);
	sqlite3_bind_int(stmt, 1, atoi(art_id));

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

void panel_comment_delete(struct env_t *_SERVER)
{
	int        rc;
	char       *addr;
	const char *id;
	
	sqlite3_stmt *stmt;

	id = tclistval2(_SERVER->_GET, 3);
	if(id == NULL || !is_digit(id))
	{
		puts("Content-type: text/html\r\n\r\n<script>alert('请您选择要删除的评论');</script>");
		return ;
	}
		
	rc = sqlite3_prepare(db, "DELETE FROM comment WHERE id = ?;", -1, &stmt, NULL); 

	sqlite3_bind_int(stmt, 1, atoi(id));
	
	sqlite3_step(stmt); 
	sqlite3_finalize(stmt); 
	
	addr = getenv("HTTP_REFERER");
	if(addr)
		printf("Content-type: text/html\r\n\r\n<script>alert('删除已完成');window.location.href='%s';</script>", addr);
	else
		puts("Content-type: text/html\r\n\r\n<script>alert('删除已完成');</script>");
}

