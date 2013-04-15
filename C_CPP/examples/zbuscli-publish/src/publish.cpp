#include "include/prelude.h"
#include "include/zbusapi.h"

void publish(zbusconn_t* conn, char* topic, zmsg_t* msg){
	char* pubsub_service_name = "pubsub";

	asyn_ctrl_t ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	strcpy(ctrl.service, pubsub_service_name);
	ctrl.timeout = 2500; //ms

	//增加topic头部帧
	zmsg_push_front(msg, zframe_newstr(topic));
	zmsg_t* res = zbuscli_send(conn, &ctrl, msg); //异步模式完成
	if(res){
		zmsg_log(res, "");
	}
}
int main(int argc, char* argv[]){  
	//链接配置
	zbusconn_cfg_t conn_cfg;
	memset(&conn_cfg, 0, sizeof(conn_cfg));
	strcpy(conn_cfg.host, "127.0.0.1");
	conn_cfg.port = 15555; 
 
	zbusconn_t* conn = zbusconn_new(&conn_cfg); 

	//消息体
	zmsg_t* msg = zmsg_new();
	zmsg_push_back(msg, zframe_newstr("pubsub msg body"));
	publish(conn, "topic1", msg);

	zbusconn_destroy(&conn); 

	return 0;
}
