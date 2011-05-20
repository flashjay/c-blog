#include "blog.h"


FILE *debug_log;

void log_init() 
{
	debug_log = fopen("../log/debug.log", "w");

	if(debug_log == NULL) exit;
}

void log_finish()
{
	fclose(debug_log);
}

void log_debug(const char *format,...)
{
	va_list  va;
	va_start(va, format);
	
	vfprintf(debug_log, format, va);

	va_end(va);
}

/*
void log_debug(const char *format,...)
{
	char timestr[128];
	struct tm *tm = gmtime(&(current_time.tv_sec));
	if(tm == NULL)
		return NULL;
	
	strftime(timestr, 127, "%Y-%m-%d %H:%M:%S", tm);
	
	va_list  va;
	va_start(va, format);
	
	snprintf(new_format, BUF_SIZE - 1 ,  "[%s] | %s", smart_time(), format);

	if(vfprintf(file, new_format, va) < 0)
		fprintf(stderr, "Warning. [%s %d] %s\n", __FILE__, __LINE__, strerror(errno));

	va_end(va);
}
*/

