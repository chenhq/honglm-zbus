#include "include/prelude.h"
#include "include/zbusapi.h"

void*
thread_func(void* args){
	zbusconn_cfg_t conn_cfg;
	memset(&conn_cfg, 0, sizeof(conn_cfg));
	strcpy(conn_cfg.host, "127.0.0.1");
	conn_cfg.port = 15555; 
	zbusconn_t* conn = zbusconn_new(&conn_cfg);

	zbuswrk_cfg_t wrk_cfg;
	memset(&wrk_cfg, 0, sizeof(wrk_cfg)); 
	strcpy(wrk_cfg.service, "MyService");
	strcpy(wrk_cfg.mode, MODE_LB); 

	zbuswrk_t* worker = zbuswrk_new(&wrk_cfg);
	zlog("%s started \n", "MyService");
	zmsg_t* msg;

	while(1){
		msg = zbuswrk_recv(conn, worker);
		
		if(!msg) break;  
		zmsg_log(msg, "");
		printf("\n");
		zmsg_destroy(&msg);

		msg = zmsg_new();
		zmsg_push_back(msg, zframe_newstr("response from C/C++ worker"));
		zbuswrk_send(conn, worker, msg);
	}

	zbusconn_destroy(&conn);
	zbuswrk_destroy(&worker);

	return NULL;
}

int main(int argc, char* argv[]){  
	
	const int worker_count = 2;
	int i;
	zthread_t threads[worker_count]; 
	for(i=0; i<worker_count; i++){
		threads[i] = zthread_new(thread_func,NULL);
	}
	for(i=0; i<worker_count; i++){
		zthread_join(threads[i]);
	}

	return 0;
}
