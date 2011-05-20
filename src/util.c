#include "blog.h"

ulong ip2long(const char *ip)
{
	int a, b, c, d;
	ulong rv = 0;

	sscanf(ip, "%d.%d.%d.%d", &a, &b, &c, &d);
	rv = rv | (a << 24);
	rv = rv | (b << 16);
	rv = rv | (c << 8);
	rv = rv | (d);

	return rv;
}

//dec2hex - convert a decimal number into hex 
char *dec2hex(ulong num)
{
	ulong id;
	char  *ptr, tmp[56];
	
	memset(&tmp, 0, 56);
	snprintf(tmp, 55, "%lu", num);

	id  = strtoul(tmp, NULL, 10);
	asprintf(&ptr, "%lx", id);
	
	return ptr;
}


//hex2dec - convert a hex number into decimal 
ulong hex2dec(char *num)
{
	return strtoul(num, NULL, 16);
}

//./str_replace_all "(uid=%u/%u)" "%u" chantra
char *str_replace(const char *string, const char *substr, const char *replacement )
{
	char *tok    = NULL;
	char *newstr = NULL;
	char *oldstr = NULL;
	
	/* if either substr or replacement is NULL, duplicate string a let caller handle it */
	if ( substr == NULL || replacement == NULL ) 
		return strdup (string);
	
	newstr = strdup (string);
	while ( (tok = strstr( newstr, substr)))
	{
		oldstr = newstr;
		newstr = malloc (strlen ( oldstr ) - strlen ( substr ) + strlen ( replacement ) + 1 );
		/*failed to alloc mem, free old string and return NULL */
		if (newstr == NULL)
		{
			free (oldstr);
			return NULL;
		}
		memcpy ( newstr, oldstr, tok - oldstr );
		memcpy ( newstr + (tok - oldstr), replacement, strlen ( replacement ) );
		memcpy ( newstr + (tok - oldstr) + strlen( replacement ), tok + strlen ( substr ), strlen ( oldstr ) - strlen ( substr ) - ( tok - oldstr ) );
		memset ( newstr + strlen ( oldstr ) - strlen ( substr ) + strlen ( replacement ) , 0, 1 );
		
		free(oldstr);
	}

	return newstr;
}


TCXSTR *str_filter(const char *subject)
{
	int i, j;
	uint16_t cur;
	TCXSTR  *out   = NULL;

	out  = tcxstrnew3(100);
	for(i=0; subject[i] != 0; i++)
	{
		cur  = 0;
		for(j = 0; filter[j].search; j++)
		{
			if(strncmp(subject+i, filter[j].search, filter[j].len) == 0)
			{
				cur = filter[j].len;
				i  += filter[j].len - 1;
				tcxstrcat2(out, filter[j].replace);
				break;
			}
		}

		if(cur == 0)
			tcxstrcat(out, subject+i, 1);
	}

	return out;
}


bool is_digit(const char *str)
{
	int i;

	for(i=0; str[i]!=0; i++)
	{
		if(!isdigit(str[i]))
			return false;
	}

	return true;
}

bool is_username(u_char *str)
{
	int i;

	for(i=0; str[i]!=0; i++)
	{
		if(!isalnum(str[i]) && str[i] < 0x80)
			return false;
	}

	return true;
}

bool is_alpha(const char *str)
{
	int i;

	for(i=0; str[i]!=0; i++)
	{
		if(!isalpha(str[i]))
			return false;
	}

	return true;
}

bool is_alnum(const char *str)
{
	int i;

	for(i=0; str[i]!=0; i++)
	{
		if(!isalnum(str[i]))
			return false;
	}

	return true;
}

void ltrim(char *str)
{
	char *s = str;

	while (isspace(*s))
		s++;

	while ((*str++ = *s++));
}

void rtrim(char *str)
{
	char *s = str;
	register int i;

	for (i = strlen(s)-1; isspace(*(s+i)); i--)
		*(s+i) = '\0';

	while ((*str++ = *s++));
}


void trim(char *str)
{
	if(str == NULL) return ;

	ltrim(str);
	rtrim(str);
}

int strpos(const char *haystack, char *needle)
{
	char *p = strstr(haystack, needle);

	if (p) return p - haystack;
	  
	return -1;   // Not found = -1.
}

TCLIST *explode(const char *delimiter , char *str)
{
	TCLIST *data  = NULL;
	char   *tmp   = NULL;
	char   *next  = NULL;
	
	data = tclistnew2(30);
	next = strtok_r(str, delimiter, &tmp);
	while (next != NULL)
	{
		tclistpush2(data, next);
		next = strtok_r(NULL, delimiter, &tmp);
	}

	return data;
}



//total 总数
//slot 每页的记录数
//pos  当前第几页

void pager(tpl_t *tpl, double total, double slot, uint pos, char *url, uint limit[2])
{
	TCXSTR *str;
	uint   n, i, cur;
	
	
	str  = tcxstrnew();
	n    = (uint)ceil(total / slot);
	if(pos < 1)
		pos = 1;
	if(pos > n)
		pos = n;
	

	limit[1] = slot;
	limit[0] = (pos == 1) ? 0 : (pos-1) * limit[1];
	
	for(i=0; i<n; i++)
	{
		cur = (i+1);
		if(cur == pos)
			tcxstrprintf(str, "<a class='page_link' href='javascript:void(0);'>%d</a>&nbsp;", cur);
		else
			tcxstrprintf(str, "<a href=\"%s-%d.html\">%d</a>&nbsp;", url, cur, cur);
	}
	
	tpl_set_field_global(tpl, "page", tcxstrptr(str), tcxstrsize(str));
	
	tcxstrdel(str);
}

//total 总数
//slot 每页的记录数
//pos  当前第几页

void ajax_pager(tpl_t *tpl, double total, double slot, uint pos, uint limit[2])
{
	TCXSTR *str;
	uint   n, i, cur;
	
	
	str  = tcxstrnew();
	n    = (uint)ceil(total / slot);
	if(pos < 1)
		pos = 1;
	if(pos > n)
		pos = n;
	

	limit[1] = slot;
	limit[0] = (pos == 1) ? 0 : (pos-1) * limit[1];
	
	for(i=0; i<n; i++)
	{
		cur = (i+1);
		if(cur == pos)
			tcxstrprintf(str, "<a class='page_link' href='javascript:void(0);'>%d</a>&nbsp;", cur);
		else
			tcxstrprintf(str, "<a href='#' onclick='return loading(%d)'>%d</a>&nbsp;", cur, cur);
	}
	
	tpl_set_field_global(tpl, "page", tcxstrptr(str), tcxstrsize(str));
	
	tcxstrdel(str);
}


bool file_exists(const char *filename)
{
	FILE *fp;

    if (fp = fopen(filename, "r")) //I'm sure, you meant for READING =)
    {
        fclose(fp);
        return true;
    }
    return false;
}


bool is_dir(char *path)
{
	DIR *d;

	d = opendir(path);
	if(d)
	{
		closedir(d);
		return true;
	}
	return false;
}

char *basename(char *path)
{
	char *p;
	
	p = strrchr(path, '\\');
	if(p) return ++p;

	p = strrchr(path, '/');
	if(p) return ++p;

	return path;
}

int mkpath(mspace mp, const char *s, mode_t mode)
{
	char *q, *r = NULL, *path = NULL, *up = NULL;
	int rv;

	rv = -1;
	if (strcmp(s, ".") == 0 || strcmp(s, "/") == 0)
			return (0);

	if ((path = mspace_strdup(mp, s)) == NULL)
			return rv;

	if ((q = mspace_strdup(mp, s)) == NULL)
			return rv;

	if ((r = dirname(q)) == NULL)
			goto out;

	if ((up = mspace_strdup(mp, r)) == NULL)
			return rv;

	if ((mkpath(mp, up, mode) == -1) && (errno != EEXIST))
			goto out;

	if ((mkdir(path, mode) == -1) && (errno != EEXIST))
			rv = -1;
	else
			rv = 0;
 
out:
	if (up != NULL)
			mspace_free(mp, up);
	mspace_free(mp, q);
	mspace_free(mp, path);

	return rv;
}

