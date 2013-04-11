#include <prelude.h>
#include <zbusapi.h> 
#include "../include/zmq.h"

void* g_ctx;
zmsg_t* random_zmsg(){
	zmsg_t* msg = zmsg_new();
	srand(time(NULL));
	int size = rand()%100; 
	for(int i=0;i<size;i++){
		int frame_size = rand()%1024; 
		unsigned char* data = (unsigned char*)zmalloc(frame_size);
		for(int j=0;j<frame_size;j++)
			data[j] = randof(256);
		zmsg_push_back(msg, zframe_new(data, frame_size));
		zfree(data);
	}
	return msg;
}
void register_svc(void* sock, char* svc){
	zmsg_t* msg = zmsg_new();
	zmsg_push_back(msg, zframe_new(NULL,0));
	zmsg_push_back(msg, zframe_newstr(MDPM));
	zmsg_push_back(msg, zframe_newstr(""));  
	zmsg_push_back(msg, zframe_newstr("reg"));  
	zmsg_push_back(msg, zframe_newstr(svc));  
	zmsg_push_back(msg, zframe_newstr(""));
	zmsg_push_back(msg, zframe_newstr("1")); 

	zmsg_send(&msg, sock);

	
}

void unregister_svc(void* sock, char* svc){
	zmsg_t* msg = zmsg_new();
	zmsg_push_back(msg, zframe_new(NULL,0));
	zmsg_push_back(msg, zframe_newstr(MDPM));
	zmsg_push_back(msg, zframe_newstr(""));  
	zmsg_push_back(msg, zframe_newstr("del"));  
	zmsg_push_back(msg, zframe_newstr(svc));   

	zmsg_send(&msg, sock);

	zmsg_t* rep = zmsg_recv(sock);
	if(rep){
		zmsg_log(rep, NULL);
	}
}

void register_worker(char* svc){
	for(int i=0;i<1;i++){
	void* sock = zmq_socket(g_ctx, ZMQ_DEALER);
	zmq_connect(sock, "tcp://localhost:15555");
	
	zmsg_t* msg = zmsg_new();
	zmsg_push_back(msg, zframe_new(NULL,0));

	zmsg_send(&msg, sock);

	msg = zmsg_new();
	zmsg_push_back(msg, zframe_new(NULL,0));
	zmsg_push_back(msg, zframe_newstr(MDPW));
	zmsg_push_back(msg, zframe_newstr(MDPW_DISC));   

	zmsg_send(&msg, sock);
	
	zclock_sleep(10);
	zmq_close(sock);
	}
}
int main(int argc, char* argv[]){
	g_ctx = zctx_new(1);
	
	for(int i=0;i<1;i++){
		char svc[64];
		sprintf(svc, "svcx%04d", i);
		register_worker(svc);
	} 

	getchar(); 
	zctx_destroy(&g_ctx);
	
	return 0;
}
 