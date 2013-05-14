#include "include/prelude.h"
#include "include/zbusapi.h" 

int main(int argc, char* argv[]){  
	zbusconn_cfg_t cfg;
	memset(&cfg, 0, sizeof(cfg));
	strcpy(cfg.host, "127.0.0.1");
	cfg.port = 15555;

	zbusconn_t* conn = zbusconn_new(&cfg);
	asyn_ctrl_t ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	strcpy(ctrl.service, "helloworld");
	ctrl.timeout = 2500;

	zmsg_t* req = zmsg_new();
	zmsg_push_back(req, zframe_newstr("hello"));
	zmsg_t* res = zbuscli_send(conn, &ctrl, req);
	if(res){
		zmsg_log(res, NULL);
	}

	getchar();
	return 0;
}
