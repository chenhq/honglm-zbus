#include <prelude.h>
#include <zbusapi.h>
 

int main(int argc, char* argv[]){  
	zbusconn_cfg_t cfg;
	memset(&cfg, 0, sizeof(cfg));
	strcpy(cfg.host, "127.0.0.1");
	cfg.port = 15555;
	strcpy(cfg.id, "local_mq");

	zbusconn_t* conn = zbusconn_new(&cfg); 
	
	while(1){
		zmsg_t* msg = zbuscli_recv(conn, 4000);;
		if(!msg) break;
		zmsg_dump(msg);
	}
	

	zbusconn_destroy(&conn);

	getchar();
	return 0;
}
