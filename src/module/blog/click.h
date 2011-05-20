#ifndef __BLOG_CLICK_H__
#define __BLOG_CLICK_H__


void *blog_click_sync(void *arg);

struct click_t *blog_click_init(uint id);

void blog_click_once(struct env_t *_SERVER);

void blog_click_count(struct env_t *_SERVER);

bool my_rb_insert(struct rb_root *root, struct click_t *data);

struct click_t *rb_uint_search(struct rb_root *root, uint value);

#endif
