#include "blog.h"


void blog_comment_list(struct env_t *_SERVER)
{
	sqlite3_stmt *stmt;
	const char   *id, *page;

	tpl_t        *tpl;
	char         *html, sql[128];
	
	uint         total, limit[2], cur;
	int          rc, len, i, type;

	id   = tclistval2(_SERVER->_GET, 2);
	page = tclistval2(_SERVER->_GET, 3);
	if(!is_digit(id))
	{
		printf("Content-type: text/html\r\n\r\n<script>window.location.href='/';</script>");
		return ;
	}


	tpl = tpl_alloc();
	if (tpl_load(tpl, "./templets/blog/comment_list.html") != TPL_OK)
    {
		printf("Content-type: text/html\r\n\r\n./templets/panel/article_insert.html Error loading template file!");
        tpl_free(tpl);
		return ;
    }
	
	memset(sql, 0, sizeof(sql));
	len = snprintf(sql, sizeof(sql), "SELECT COUNT(*) AS c FROM comment WHERE article_id = '%s'", id);
	total = dataset_count(sql);
	
	//分页
	cur = (page) ? atoi(page) : 1;
	ajax_pager(tpl, total, conf.page.comment, cur, limit);
	
//	log_debug("blog_comment_list.id = %s\tlimit[0] = %d\tlimit[1] = %d\tcur = %d\r\n", id, limit[0], limit[1], cur);

	rc = sqlite3_prepare(db, "SELECT id, author, content, datetime(post_time, 'unixepoch') AS dt FROM comment WHERE article_id = ? ORDER BY id DESC LIMIT ?, ?;", -1, &stmt, NULL);
	sqlite3_bind_int(stmt, 1, atoi(id));
	sqlite3_bind_int(stmt, 2, limit[0]);
	sqlite3_bind_int(stmt, 3, limit[1]);

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
	
	tpl_set_field_uint_global(tpl, "total", total);

	html = mspace_malloc(_SERVER->mp, tpl_length(tpl) + 1);
    tpl_get_content(tpl, html);

	printf("Content-type: text/html; charset=gbk\r\n\r\n%s", html);
	
	tpl_free(tpl);
	mspace_free(_SERVER->mp, html);
	
	tpl  = NULL;
}

void blog_comment_insert(struct env_t *_SERVER)
{
	const char *id;
	const char *author;
	const char *content;
	

	int    rc;
	ulong  ip;
	TCXSTR *data;
	char   *addr, *user;
	
	struct click_t  *recs;
	sqlite3_int64   lastid;
	sqlite3_stmt    *stmt;


	if(tcmapget2(_SERVER->_POST, "Comment_Insert"))
	{
		id        = tcmapget2(_SERVER->_POST, "id");
		author    = tcmapget2(_SERVER->_POST, "author");
		content   = tcmapget2(_SERVER->_POST, "content");
		addr      = getenv("REMOTE_ADDR");

		if(!is_digit(id))
		{
			puts("Content-type: text/html\r\n\r\n<script>alert('ID参数错误');top.location.href='/';</script>");
			return ;
		}
		else if(content == NULL || strlen(content) < 10)
		{
			printf("Content-type: text/html\r\n\r\n<script>alert('评论的内容是10个字以上');parent.document.getElementById('content').focus();</script>");
			return ;
		}


		if(!panel_user_is_login(_SERVER))
		{
			if(!author || !is_username((u_char *)author))
			{
				puts("Content-type: text/html\r\n\r\n<script>alert('用户名格式错误');parent.document.getElementById('author').focus();</script>");
				return ;
			}
			else if(strlen(author) < 2 || strlen(author) > 15)
			{
				puts("Content-type: text/html\r\n\r\n<script>alert('用户名最小为2，最大为15了!');parent.document.getElementById('author').focus();</script>");
				return ;
			}
			if(strstr(author, conf.username) != NULL)
			{
				printf("Content-type: text/html\r\n\r\n<script>alert('用户名不能带有%s!');parent.document.getElementById('author').focus();</script>", conf.username);
				return ;
			}
		}
		
		ip   = addr ? ip2long(addr) : 0;
		data = str_filter(content);
		user = panel_user_is_login(_SERVER) ? conf.username :  (char *)author;
		rc   = sqlite3_prepare(db, "INSERT INTO comment(article_id, author, addr, post_time, content) VALUES(?, ?, ?, ?, ?);", -1, &stmt, NULL); 
		
		sqlite3_bind_text(stmt, 1, id,    -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, user, -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt,  3, ip);
		sqlite3_bind_int(stmt,  4, time((time_t*)0));
		sqlite3_bind_text(stmt, 5, tcxstrptr(data), -1, SQLITE_STATIC);

		sqlite3_step(stmt); 
		sqlite3_finalize(stmt); 	
		
		lastid = sqlite3_last_insert_rowid(db);
		
		recs   = rb_uint_search(&rr, lastid);
		if(recs == NULL)
		{
			recs = blog_click_init(lastid);
			if(recs)
				my_rb_insert(&rr, recs);
		}
		else
		{
			recs->comment++;
		}		
		tcxstrdel(data);
		data = NULL;
		
		puts("Content-type: text/html\r\n\r\n<script>alert('评论成功');parent.comment_clear();parent.loading(1);</script>");
		return ;
	}
	puts("Content-type: text/html\r\n\r\n<script>window.location.href='/';</script>");
}
