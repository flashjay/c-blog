#ifndef __PANEL_ARTICLE_H__
#define __PANEL_ARTICLE_H__


void panel_article_list(struct env_t *_SERVER);

void panel_article_index(struct env_t *_SERVER);

void panel_article_insert(struct env_t *_SERVER);

void panel_article_delete(struct env_t *_SERVER);

void panel_article_update(struct env_t *_SERVER);

void panel_article_clean(struct env_t *_SERVER);

void panel_article_upload(struct env_t *_SERVER);

#endif

