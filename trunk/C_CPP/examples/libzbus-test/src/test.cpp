#include "include/prelude.h" 
#include "include/zbusapi.h"
#include "include/hash.h"
#include "include/list.h"

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

int test_request(){
	zbusconn_cfg_t cfg;
	memset(&cfg, 0, sizeof(cfg));
	strcpy(cfg.host, "127.0.0.1");
	cfg.port = 15555;
	cfg.verbose = 1;
	
	zbusconn_t* conn = zbusconn_new(&cfg);
	for(int i=0;i<100;i++){
		zmsg_t* req = random_zmsg();
		zmsg_t* rep = zbuscli_request(conn, "helloworld", "", req, 2500);
	}


	zbusconn_destroy(&conn);
	return 0;
}

int main(int argc, char* argv[]){   
	hash_t* hash = hash_new(NULL, NULL);
	for(int i=0;i<100000;i++){
		char key[12];
		char val[24];
		sprintf(key, "%d", i);
		sprintf(val, "%d%d", i, i);
		hash_put(hash, key, val);
	}

	zmalloc_report();
	hash_stats(hash);
	hash_destroy(&hash);
	zmalloc_report();
	getchar();
	return 0;
}
 