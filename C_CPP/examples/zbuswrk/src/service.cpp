#include "include/prelude.h"
#include "include/zbusapi.h"

int main(int argc, char* argv[]){  
	zbusconn_cfg_t conn_cfg;
	memset(&conn_cfg, 0, sizeof(conn_cfg));
	strcpy(conn_cfg.host, "127.0.0.1");
	conn_cfg.port = 15555; 

	zbuswrk_cfg_t wrk_cfg;
	memset(&wrk_cfg, 0, sizeof(wrk_cfg)); 
	strcpy(wrk_cfg.service, "pubsub");
	strcpy(wrk_cfg.mode, MODE_PUBSUB);

	zbusconn_t* conn = zbusconn_new(&conn_cfg);
	zbuswrk_t* worker = zbuswrk_new(&wrk_cfg);
	
	zbuswrk_subscribe(conn, worker, "topic1");
	zbuswrk_subscribe(conn, worker, "topic2");
	zbuswrk_unsubscribe(conn, worker, "topic1");
	zbuswrk_unsubscribe(conn, worker, "topic3");
	zmsg_t* msg;

	while(1){
		msg = zbuswrk_recv(conn, worker);
		if(!msg) break; 
		zmsg_log(msg,"");
		 
		zbuswrk_send(conn, worker, msg);
	}

	zbusconn_destroy(&conn);
	zbuswrk_destroy(&worker);

	return 0;
}
