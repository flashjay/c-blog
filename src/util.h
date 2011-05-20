#ifndef __UTIL_H__
#define __UTIL_H__


void ltrim(char *str);

void rtrim(char *str);

void trim(char *str);

ulong ip2long(const char *ip);

bool file_exists(const char *filename);

bool is_dir(char *path);

bool is_alnum(const char *str);

bool is_digit(const char *str);

bool is_alpha(const char *str);

bool is_username(u_char *str);

char *dec2hex(ulong num);

ulong hex2dec(char *num);

int strpos(const char *haystack, char *needle);

char *str_replace(const char *string, const char *substr, const char *replacement);

void pager(tpl_t *tpl, double total, double slot, uint pos, char *url, uint limit[2]);

void ajax_pager(tpl_t *tpl, double total, double slot, uint pos, uint limit[2]);

TCXSTR *str_filter(const char *subject);

TCLIST *explode(const char *delimiter , char *str);

char *basename(char *path);

int mkpath(mspace mp, const char *s, mode_t mode);

#endif
