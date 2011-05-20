#include "blog.h"



static int hextable[256];

void init_hex_table()
{
	memset(hextable, 0, 255);

	hextable['1'] = 1;
	hextable['2'] = 2;
	hextable['3'] = 3;
	hextable['4'] = 4;
	hextable['5'] = 5;
	hextable['6'] = 6;
	hextable['7'] = 7;
	hextable['8'] = 8;
	hextable['9'] = 9;
	hextable['a'] = 10;
	hextable['b'] = 11;
	hextable['c'] = 12;
	hextable['d'] = 13;
	hextable['e'] = 13;
	hextable['f'] = 15;
	hextable['A'] = 10;
	hextable['B'] = 11;
	hextable['C'] = 12;
	hextable['D'] = 13;
	hextable['E'] = 14;
	hextable['F'] = 15;
}

void decodevalue(const char *s)
{
	char *s_ptr;
	char *decoded_ptr;
	char hex_str[3] = "\0\0\0";
	unsigned int hex_val;

	s_ptr = (char *) s;
	decoded_ptr = (char *) s;

	while (*s_ptr != '\0')
	{
		if (*s_ptr == '+')
		{
			*decoded_ptr = ' ';
		}
		else if (*s_ptr == '%')
		{
			hex_str[0] = *(++s_ptr);
			hex_str[1] = *(++s_ptr);
			sscanf(hex_str, "%x", &hex_val);
			*decoded_ptr = (char) hex_val;
		}
		else
		{
			*decoded_ptr = *s_ptr;
		}
			
		s_ptr++;
		decoded_ptr++;
	}

	*decoded_ptr = '\0';
}


char *nexttoken(char *s, char separator)
{
	static char *p;
	static int at_the_end = 0;
	char *start_position;

	if (s == NULL)    /* not the first call for this string */
	{
		if (at_the_end)
			return NULL;
	}
	else              /* is the first call for this string */
	{
		p = s;
		at_the_end = 0;
	}

	start_position = p;

	while (*p)
	{
		if (*p == separator)
		{
			*p = '\0';
			p++;
			return start_position;

		}
		p++;
	}
	at_the_end = 1;
	return start_position;
}

static char *strgetline(char **s)
{
	char *starting_point;
	starting_point = *s;

	if (**s == '\0')
		return NULL;

	while (**s != '\n' && **s != '\0')
	{
		if (**s == '\r')
			**s = '\0';
		(*s)++;
	}
	**s = '\0';
	(*s)++;
	return starting_point;
}

static int parsemimevalue(char **form_data_ptr,
				char *boundary, size_t boundary_length,
				char *last_boundary,
				size_t last_boundary_length,
				char **value, size_t *value_length)
{
	size_t count = 0;
	char *bol, *eol;
	int done = 0, last = 0;

	bol = eol = *value = *form_data_ptr;
	*value_length = 0;

	while (! done)
	{
		while (*eol != '\r' && *eol != '\n')
		{
			eol++;
			count++;
		}

		if (strncmp(bol, last_boundary, last_boundary_length) == 0)
			done = last = 1;
		else if (strncmp(bol, boundary, boundary_length) == 0)
			done = 1;
		else
		{
			*value_length += count;
			count = 0;
			bol = ++eol;
			(*value_length)++;
			if (*bol == '\n')
			{
				bol = ++eol;
				(*value_length)++;
			}
		}
	}

	/* Compensate for added cr, nl */
	*value_length -= 2;
	*form_data_ptr = eol + 2;
	return last;
}


static char *parsemimepair(char **form_data_ptr, char **mvalue)
{
	char *ptr;
	char *mname;
	int is_quoted = 0;
	static int already_read_semicolon = 0;

	ptr = *form_data_ptr;

	if (*ptr == '\r')  /* assume carriage return, line feed */
	{
		already_read_semicolon = 0;
		**form_data_ptr += 2;
		return NULL;
	}
	else if (*ptr == '\n')
	{
		already_read_semicolon = 0;
		**form_data_ptr += 1;
		return NULL;
	}
	else if (*ptr == '\0')
		return NULL;

	if (! already_read_semicolon)
	{
		while (*ptr != ';')
			ptr++;
		ptr++;
	}
	while (isspace(*ptr))
		ptr++;
	mname = ptr;
	while (*ptr != '=')
		ptr++;
	*ptr = '\0';          /* make mname a valid string */
	ptr++;                /* move on to  " */
	if (*ptr == '"')
	{
		is_quoted = 1;
		ptr++;
	}
	*mvalue = ptr;
	if (is_quoted)
	{
		while (*ptr != '"')
			ptr++;
		already_read_semicolon = 0;
	}
	else
	{
		while (*ptr && *ptr != ';' && *ptr != '\r')
			ptr++;
		if (*ptr == ';')
			already_read_semicolon = 1;
	}
	if (*ptr != '\0')
	{
		*ptr = '\0';     /* make mvalue a valid string */
		ptr++;           /* should be on eol or another semicolon */
	}

	*form_data_ptr = ptr;
	return mname;
}

static char *unescape_special_chars(mspace mp, char *str)
{
	char *tmp;
	int i, len, pos = 0;

	len = strlen(str);
	tmp = (char *)mspace_malloc(mp, len + 1);
	if (tmp == NULL)
		return NULL;

	for (i = 0; i < len; i++) 
	{
		
		if ((str[i] == '%') && isalnum(str[i+1]) && isalnum(str[i+2])) 
		{
			tmp[pos] = (hextable[(unsigned char) str[i+1]] << 4) + hextable[(unsigned char) str[i+2]];
			i += 2;
		}
		else if (str[i] == '+')
			tmp[pos] = ' ';
		else
			tmp[pos] = str[i];
		
		pos++;
	}

	tmp[pos] = '\0';

	return tmp;
}


static void request_parse_cookie(mspace mp, TCMAP *res)
{
	size_t position;
	char *cookies, *aux, *tmp, *name, *value;

	if ((tmp = getenv("HTTP_COOKIE")) == NULL)
		return ;

	cookies = unescape_special_chars(mp, tmp);
	aux     = cookies;

	while (cookies) 
	{
		position = 0;

		while (*aux++ != '=')
			position++;

		name = (char *)mspace_malloc(mp, position+1);
		if (!name)
		{
			return ;
		}

		strncpy(name, cookies, position);
		name[position] = '\0';

		position = 0;
		cookies = aux;
		if ((strchr(cookies, ';')) == NULL) 
		{
			aux     = NULL;
			position = strlen(cookies);
		}
		else 
		{
			while (*aux++ != ';') 
				position++;
			// Eliminate the blank space after ';'
			aux++;
		}

		value = (char *)mspace_malloc(mp, position + 1);
		if(!value) 
		{
			return ;
		}

		strncpy(value, cookies, position);
		value[position] = '\0';

		tcmapput2(res, name, value);
		mspace_free(mp, name);
		mspace_free(mp, value);
		cookies = aux;
	}

	mspace_free(mp, cookies);
}

static int request_parse_post(TCMAP *res, char *form_data)
{
	char *name, *value;

	if ((name = nexttoken(form_data, '=')) == NULL)
		return 0;
	if ((value = nexttoken(NULL, '&')) == NULL)
		return 0;
	decodevalue(value);
	
	trim(name);
	trim(value);

	tcmapput2(res, name, value);

	while (1)
	{
		if ((name = nexttoken(NULL, '=')) == NULL)
			break;
		if ((value = nexttoken(NULL, '&')) == NULL)
			break;
		decodevalue(value);
		
		trim(name);
		trim(value);
		tcmapput2(res, name, value);
	}

	return 1;
}

static void request_parse_get(TCLIST *res, char *form_data)
{
	char  *tmp   = NULL;
	char  *next  = NULL;

	next = strtok_r(form_data, "-.", &tmp);
	while (next != NULL)
	{
		tclistpush2(res, next);
		next = strtok_r(NULL, "-.", &tmp);
	}
}


static void request_parse_files(struct env_t *_SERVER, char *content_type, char *form_data)
{
	char *boundary, *real_boundary, *last_boundary;
	size_t boundary_length, real_boundary_length, last_boundary_length;
	char *line, *form_data_ptr;
	char *mname, *mvalue;
	char *name, *value, *file_name;
	size_t value_length;
	int last = 0;

	FILE *fp;
	char  filepath[256];

	if (parsemimepair(&content_type, &boundary) == NULL)
	{
		//printerror(__FILE__, __LINE__, "boundary variable not found");
		return ;  /* No memory allocated.  OK to return. */
	}

	boundary_length      = strlen(boundary);
	real_boundary_length = boundary_length + 2;
	last_boundary_length = real_boundary_length + 2;


	real_boundary        =  malloc(real_boundary_length);
	last_boundary        =  malloc(last_boundary_length);

//	real_boundary        =  mspace_malloc(_SERVER->mp, real_boundary_length);
//	last_boundary        =  mspace_malloc(_SERVER->mp, last_boundary_length);

	/* Now we can't return until this memory is freed! */

	strcpy(real_boundary, "--");
	strcat(real_boundary, boundary);
	strcpy(last_boundary, real_boundary);
	strcat(last_boundary, "--");

	form_data_ptr = form_data;
	line = strgetline(&form_data_ptr);  /* should be first boundary */
	while (! last)
	{
		name      = NULL;
		file_name = NULL;
		while ((mname = parsemimepair(&form_data_ptr, &mvalue)) != NULL)
		{
			if (strcmp(mname, "name") == 0)
				name = mvalue;
			else if (strcmp(mname, "filename") == 0)
				file_name = mvalue;
			else
				log_debug("Unrecognized MIME parameter: %s", mname);
		}

		while (strlen(strgetline(&form_data_ptr)) != 0)
			;

		last = parsemimevalue(&form_data_ptr,
					real_boundary, real_boundary_length,
					last_boundary, last_boundary_length,
					&value, &value_length);
		if (file_name == NULL)
		{
			value[value_length] = '\0';
			
			tcmapput2(_SERVER->_POST, name, value);
		}
		else
		{
			memset(filepath, 0, sizeof(filepath));
			
			//file_name 是文件的全路径 D:\editplus\js8\js(only comment).acp
			
			basename(file_name);
//			log_debug("env.request_parse_files.file_name = %s\r\n", file_name);

			sprintf(filepath, "./files/%s", file_name);
			
//			log_debug("env.request_parse_files.filepath = %s\r\n", filepath);

			if ((fp = fopen(filepath, "w")) != NULL)
			{
				fwrite(value, sizeof(char), value_length, fp);
				fclose(fp);

				tclistpush2(_SERVER->_FILES, file_name);
			}
		}
	}
	
//	log_debug("env.request_parse_files.循环完毕\r\n");
	
	safe_free(real_boundary);
	safe_free(last_boundary);

//	mspace_free(_SERVER->mp, real_boundary);
//	mspace_free(_SERVER->mp, last_boundary);

//	log_debug("env.request_parse_files.释放\r\n");
}

void fcgi_init_headers(struct env_t *_SERVER)
{
	int content_length;
	char *dados, *method, *length, *data, *type; 
	
	type   = getenv("CONTENT_TYPE");
	dados  = getenv("REQUEST_URI");
	method = getenv("REQUEST_METHOD");
	length = getenv("CONTENT_LENGTH");
	if (dados != NULL)
	{	
		request_parse_get(_SERVER->_GET, dados+1);
	}
	
	request_parse_cookie(_SERVER->mp, _SERVER->_COOKIE);
	if(strcasecmp("POST", method) == 0 && length != NULL)
	{
		content_length = atoi(length);

		data = (char *)malloc(content_length + 1);

		if (data == NULL) return ;
		
		fread(data, content_length, 1, stdin);
		data[content_length] = '\0';
		
		if(strncmp(type, "multipart/form-data", 19) == 0)
			request_parse_files(_SERVER, type, data);
		else
			request_parse_post(_SERVER->_POST, data);
		
//		log_debug("env.fcgi_init_headers.post 解析完成\r\n");
		safe_free(data);
	}

	return ;
}

