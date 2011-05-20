#include "blog.h"


bool my_rb_insert(struct rb_root *root, struct click_t *data)
{
	struct rb_node **new = &(root->rb_node), *parent = NULL;

	// Figure out where to put new node 
	while (*new) 
	{
		struct click_t *this = container_of(*new, struct click_t, node);

		parent = *new;

		if (this->id > data->id)
			new = &(*new)->rb_left;
	    else
			new = &(*new)->rb_right;
	}

	// Add new node and rebalance tree. 
	rb_link_node(&data->node, parent, new);
	rb_insert_color(&data->node, root);

	return true;
}

struct click_t *rb_uint_search(struct rb_root *root, uint value)
{
	struct rb_node *node = root->rb_node;  /* top of the tree */

	while (node)
	{
		struct click_t *stuff = rb_entry(node, struct click_t, node);

		if (stuff->id > value)
			node = node->rb_left;
		else if (stuff->id < value)
			node = node->rb_right;
		else
			return stuff;  /* Found it */
	}
	return NULL;
}

struct click_t *blog_click_init(uint id)
{

	int            rc, i;
	sqlite3_stmt   *stmt;
	struct click_t *recs;


	sqlite3_prepare(db, "SELECT hit, comment_num FROM article WHERE art_id = ?", -1, &stmt, NULL);
	sqlite3_bind_int(stmt, 1, id);
	
	rc = sqlite3_step(stmt);
	

	if(rc != SQLITE_ROW)
		return NULL;
	
	recs     = (struct click_t *)malloc(sizeof(struct click_t));
	if(recs == NULL)
		return NULL;
	
	recs->id = id;

	for(i=0; i<sqlite3_column_count(stmt); i++)
	{
		if(strcasecmp("hit", sqlite3_column_name(stmt,i)) == 0)
		{
			recs->hit     = sqlite3_column_int(stmt, i);
			recs->hit     = (recs->hit==0) ? 1 : recs->hit;
		}
		if(strcasecmp("comment_num", sqlite3_column_name(stmt,i)) == 0)
		{
			recs->comment = sqlite3_column_int(stmt, i);
			recs->comment = (recs->comment==0) ? 1 : recs->comment;
		}
	}
	sqlite3_finalize(stmt);

	return recs;
	
}


void blog_click_once(struct env_t *_SERVER)
{
	uint  value;
	const char *id;
	struct click_t *recs;
	

	id = tclistval2(_SERVER->_GET, 2);
	if(!id || !is_digit(id))
	{
		puts("Content-type: application/x-javascript\r\nCache-control: no-cache\r\nPragma: no-cache\r\nExpires: Tue, 17 12 2002 15:34:27 GMT\r\n\r\n");
		return ;
	}

	value  = atoi(id);
	recs   = rb_uint_search(&rr, value);
	if(recs == NULL)
	{
		recs = blog_click_init(value);
		if(recs)
		{
			my_rb_insert(&rr, recs);
		};
	}
	else
	{
		recs->hit++;
	}
	puts("Content-type: application/x-javascript\r\nCache-control: no-cache\r\nPragma: no-cache\r\nExpires: Tue, 17 12 2002 15:34:27 GMT\r\n\r\n");
	
}

void blog_click_count(struct env_t *_SERVER)
{

	TCLIST     *id;
	TCXSTR     *json;

	uint       value;
	int        num, i, sp;

	char  *uri;
	const char *str, *val;
	struct click_t *recs;

	str = tclistval2(_SERVER->_GET, 2);
	if(!str)
	{
		puts("Content-type: application/x-javascript\r\nCache-control: no-cache\r\nPragma: no-cache\r\nExpires: Tue, 17 12 2002 15:34:27 GMT\r\n\r\n");
		return ;
	}
	
	uri = mspace_strdup(_SERVER->mp, str);
	id  = explode(",", uri);
	mspace_free(_SERVER->mp, uri);

	num = tclistnum(id);
	if(num == 0)
	{
		puts("Content-type: application/x-javascript\r\nCache-control: no-cache\r\nPragma: no-cache\r\nExpires: Tue, 17 12 2002 15:34:27 GMT\r\n\r\n");
		return ;
	}
	
	json = tcxstrnew3(100);

	tcxstrcat(json, "{", 1);
	for(i=0; i<num; i++)
	{
		val = tclistval(id, i, &sp);
		if(sp == 0)
			continue;

		value  = atoi(val);
		recs   = rb_uint_search(&rr, value);
		if(recs)
			tcxstrprintf(json, "%u:[%u,%u],", recs->id, recs->hit, recs->comment);
		else
			tcxstrprintf(json, "%u:[0,0],", value);
	}

	printf("Content-type: application/x-javascript\r\nCache-control: no-cache\r\nPragma: no-cache\r\nExpires: Tue, 17 12 2002 15:34:27 GMT\r\n\r\nassign('%s', %d);", tcxstrptr(json), tcxstrsize(json));

	tclistdel(id);
	tcxstrdel(json);
	
}

void *blog_click_sync(void *arg)
{
	int    rc;
	sqlite3_stmt     *stmt;
	pthread_cond_t   cond;
	pthread_mutex_t  mutex;
	struct rb_node   *node;
	struct click_t   *recs;
	struct timespec  timeout;
	
	pthread_cond_init (&cond, 0);
	pthread_mutex_init(&mutex, 0);	
	while(1)
	{
		timeout.tv_sec  = time(NULL) + 3600;   
		timeout.tv_nsec = 0;  

		pthread_mutex_lock(&mutex);
		pthread_cond_timedwait(&cond,   &mutex,   &timeout);   
		pthread_mutex_unlock(&mutex);
		
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
	}
}
