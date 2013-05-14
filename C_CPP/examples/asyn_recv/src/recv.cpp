#include "include/prelude.h"
#include "include/zbusapi.h"
 

int main(int argc, char* argv[]){  
	//1) 创建配置链接
	zbusconn_cfg_t cfg;
	memset(&cfg, 0, sizeof(cfg));
	strcpy(cfg.host, "127.0.0.1");
	cfg.port = 15555;
	strcpy(cfg.id, "local_mq"); //与消息发送方协商队列名称, 全局唯一

	zbusconn_t* conn = zbusconn_new(&cfg); 
	
	while(1){
		//2) 从队列取消息（链接id已经指定消息队列名称）
		zmsg_t* msg = zbuscli_recv(conn, 2500);;
		if(!msg) break;
		zmsg_log(msg, "");
	}
	

	zbusconn_destroy(&conn);

	getchar();
	return 0;
}
