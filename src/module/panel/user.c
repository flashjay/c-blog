#include "blog.h"


void build_id(char *key)
{
	uuid_t id;

	uuid_generate(id); 
	uuid_unparse(id, key); 
	uuid_clear(id);
}

void panel_user_login(struct env_t *_SERVER)
{
	time_t     expires;
	char       code[37];
	const char *user, *pass;
	
	if(tcmapget2(_SERVER->_POST, "User_Login"))
	{
		user = tcmapget2(_SERVER->_POST, "username");
		pass = tcmapget2(_SERVER->_POST, "password");
		
		if(!user || !is_username((u_char *)user))
		{
			puts("Content-type: text/html\r\n\r\n<script>alert('用户名格式错误');</script>");
			return ;
		}
		else if(!pass || strlen(pass) < 5)
		{
			puts("Content-type: text/html\r\n\r\n<script>alert('密码要大于5位');</script>");
			return ;
		}
		
		memset(code, 0, sizeof(code));
		tcmd5hash(pass, strlen(pass), code);
		
		if(strcasecmp(user, conf.username) != 0)
		{
			puts("Content-type: text/html\r\n\r\n<script>alert('用户名没找到');</script>");
			return ;
		}
		else if(strcasecmp(code, conf.password) != 0)
		{
			puts("Content-type: text/html\r\n\r\n<script>alert('密码错误');</script>");
			return ;
		}
		
		memset(session_id, 0, sizeof(session_id));

		build_id(session_id);
		expires = time(NULL) + 600;
		
		printf("Set-Cookie: session=%s;expires=%s;\r\nContent-type: text/html\r\n\r\n<script>alert('登陆成功!');top.location.href='/cpanel.htm';</script>", session_id, ctime(&expires));
		return ;
	}

	puts("X-Accel-Redirect: /templets/panel/user_login.htm\r\n\r\n");
}

//判断用户是否已登陆
bool panel_user_is_login(struct env_t *_SERVER)
{
	const char *val;

	val = tcmapget2(_SERVER->_COOKIE, "session");
	if(val == NULL)
		return false;
	
	return (strcmp(session_id, val) == 0) ? true : false;
}

void panel_user_quit(struct env_t *_SERVER)
{
	time_t     expires;

	memset(&session_id, 0, sizeof(session_id));

	expires = time(NULL);
	printf("Set-Cookie: session=none;expires=%s;\r\nContent-type: text/html\r\n\r\n<script>alert('退出成功!');top.location.href='/panel-user-login.html';</script>", session_id, ctime(&expires));

}

