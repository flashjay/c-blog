#ifndef __BLOG_ENV_H__
#define __BLOG_ENV_H__

//环境变量
struct env_t
{
	mspace mp;

	TCLIST *_GET;
	TCLIST *_FILES;

	TCMAP  *_POST;
	TCMAP  *_COOKIE;
};


void init_hex_table();

void fcgi_init_headers(struct env_t *_SERVER);

#endif

