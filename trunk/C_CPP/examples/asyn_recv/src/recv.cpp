#include "include/prelude.h"
#include "include/zbusapi.h"
 

int main(int argc, char* argv[]){  
	//1) ������������
	zbusconn_cfg_t cfg;
	memset(&cfg, 0, sizeof(cfg));
	strcpy(cfg.host, "127.0.0.1");
	cfg.port = 15555;
	strcpy(cfg.id, "local_mq"); //����Ϣ���ͷ�Э�̶�������, ȫ��Ψһ

	zbusconn_t* conn = zbusconn_new(&cfg); 
	
	while(1){
		//2) �Ӷ���ȡ��Ϣ������id�Ѿ�ָ����Ϣ�������ƣ�
		zmsg_t* msg = zbuscli_recv(conn, 2500);;
		if(!msg) break;
		zmsg_log(msg, "");
	}
	

	zbusconn_destroy(&conn);

	getchar();
	return 0;
}
