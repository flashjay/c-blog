#include "blog.h"

void assign_tpl_recommend(tpl_t *tpl)
{
	int          i, rc, type;
	sqlite3_stmt *stmt;

	rc = sqlite3_prepare(db, "SELECT filename, title, art_id, catid FROM article WHERE recommend = 1 ORDER BY art_id DESC LIMIT 7;", -1, &stmt, NULL);
	rc = sqlite3_step(stmt);
	
	tpl_select_section(tpl, "recommend");
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


void assign_tpl_category(tpl_t *tpl)
{
	int          i, rc;
	sqlite3_stmt *stmt;

	rc = sqlite3_prepare(db, "SELECT sortname, sortdir FROM category;", -1, &stmt, NULL);
	rc = sqlite3_step(stmt);
	
	tpl_select_section(tpl, "classic");
	while(rc == SQLITE_ROW)
	{
		for(i=0; i<sqlite3_column_count(stmt); i++)
		{
			tpl_set_field(tpl, sqlite3_column_name(stmt,i) ,sqlite3_column_text(stmt,i), strlen(sqlite3_column_text(stmt,i)));
		}

		tpl_append_section(tpl);
		rc = sqlite3_step(stmt);
	}

	tpl_deselect_section(tpl);
	sqlite3_finalize(stmt);
}

void assign_tpl_friendlink(tpl_t *tpl)
{
	int          i, rc;
	sqlite3_stmt *stmt;

	rc = sqlite3_prepare(db, "SELECT site, url FROM friendlink ORDER BY pos DESC;", -1, &stmt, NULL);
	rc = sqlite3_step(stmt);
	
	tpl_select_section(tpl, "friendlink");
	while(rc == SQLITE_ROW)
	{
		for(i=0; i<sqlite3_column_count(stmt); i++)
		{
			tpl_set_field(tpl, sqlite3_column_name(stmt,i) ,sqlite3_column_text(stmt,i), strlen(sqlite3_column_text(stmt,i)));
		}

		tpl_append_section(tpl);
		rc = sqlite3_step(stmt);
	}

	tpl_deselect_section(tpl);
	sqlite3_finalize(stmt);
}


uint dataset_count(char *sql)
{
	int          rc;
	uint         c;
	sqlite3_stmt *stmt;

	c  = 0;
	rc = sqlite3_prepare(db, sql, -1, &stmt, NULL);
	rc = sqlite3_step(stmt);
	
	if(rc == SQLITE_ROW)
	{
		c = sqlite3_column_int(stmt, 0);
	}
	sqlite3_finalize(stmt);

	return c;
}

