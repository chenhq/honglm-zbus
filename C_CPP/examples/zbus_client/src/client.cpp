#include "include/prelude.h"
#include "include/zbusapi.h"
 

int main(int argc, char* argv[]){  

	zbusconn_cfg_t cfg;
	memset(&cfg, 0, sizeof(cfg));
	strcpy(cfg.host, "127.0.0.1");
	cfg.port = 15555;
	
	zbusconn_t* conn = zbusconn_new(&cfg); 
	zmsg_t* req = zmsg_new();
	zmsg_push_back(req, zframe_newstr("request from C/C++"));

	zmsg_t* res = zbuscli_request(conn, "MyService", "", req, 2500);
	if(res){
		zmsg_log(res, NULL);
	}

	zbusconn_destroy(&conn);

	getchar();
	return 0;
}
