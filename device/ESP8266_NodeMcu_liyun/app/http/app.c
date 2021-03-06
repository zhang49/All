/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Israel Lot <me@israellot.com> and Jeroen Domburg <jeroen@spritesmods.com> 
 * wrote this file. As long as you retain this notice you can do whatever you 
 * want with this stuff. If we meet some day, and you think this stuff is 
 * worth it, you can buy us a beer in return. 
 * ----------------------------------------------------------------------------
 */



#include "c_types.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "osapi.h"
#include "cgi.h"
#include "user_config.h"
#include "http_server.h"

#include "comm/comm.h"

#include "ws_app.h"

#define HTTP_PORT 80


static http_server_url api_urls[]={
	
//------URL-------------------------CGI----------------------ARGUMENT--------METHOD-------------FLAGS------	
	{"/command",				http_operator_api,				NULL,		HTTP_POST,			NEED_BODY},
	{NULL,						NULL,							NULL,		HTTP_ANY_METHOD,	NO_FLAG}
		
};

static url_rewrite rewrites[]={
//----PATH---------REWRITE-------
	//{"/"			,	"/index.html"},
	{"/"		,	"/index.html"},
	{"/speed-test"	,	"/speed_test.html"},
	{"/cats"		,	"/cats.html"},
	{NULL,NULL}

};

void ICACHE_FLASH_ATTR init_http_server(){

	//general max tcp conns
	espconn_tcp_set_max_con(20);

	http_server_init();
	http_server_bind_domain(INTERFACE_DOMAIN);
	http_server_enable_captive_portal();
	http_server_enable_cors();

	http_server_rewrite(&rewrites);
	http_server_bind_urls((http_server_url *)&api_urls);

	http_server_start();

	//ws
	//init_ws_server();

}
