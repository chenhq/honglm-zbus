#include "zbus/include/prelude.h"
#include "zbus/include/zbusapi.h" 
#include "zbus/include/list.h"

#include "crypt/KDEncodeCli.h"  //金证交易加密
#include "crypt/encrypt.h"      //金证加密2
#include "crypt/CryptDLL2.h"    //网安解密 

typedef struct _trade_cfg_t{   
	int   worker_threads;
	int   verbose;
	int	  debug;
	char* service_name; 
	char* service_regtoken;
	char* service_acctoken;
	char* brokers; 

	char* log_path;
} utils_cfg_t;

void*		 g_zmq_context;
utils_cfg_t* g_utils_cfg;

utils_cfg_t*
trade_cfg_new(int argc, char* argv[]){
	utils_cfg_t* self = (utils_cfg_t*)zmalloc(sizeof(utils_cfg_t));
	assert(self);
	memset(self, 0, sizeof(utils_cfg_t));
	
	self->verbose = atoi(option(argc, argv, "-v", "1"));
	self->debug = atoi(option(argc, argv, "-dbg", "0"));
	self->worker_threads = atoi(option(argc, argv, "-c", "1"));
	self->service_name = zstrdup(option(argc, argv, "-s", "Utils"));
	self->service_regtoken = zstrdup(option(argc,argv, "-kreg", ""));
	self->service_acctoken = zstrdup(option(argc,argv, "-kacc", ""));
	self->brokers = zstrdup(option(argc,argv, "-b", "localhost:15555"));
	self->log_path = zstrdup(option(argc,argv, "-log", NULL));

	return self;
}

void
trade_cfg_destroy(utils_cfg_t** self_p){
	assert(self_p);
	utils_cfg_t* self = *self_p;
	if(self){ 
		if(self->service_name)
			zfree(self->service_name);
		if(self->brokers)
			zfree(self->brokers);
		if(self->service_regtoken)
			zfree(self->service_regtoken);
		if(self->service_acctoken)
			zfree(self->service_acctoken);
		if(self->log_path)
			zfree(self->log_path);
		zfree(self);
		*self_p = NULL;
	}
}

zmsg_t* 
encrypt(zmsg_t* params){
	assert(params);
	zmsg_t* reply = zmsg_new();
	if(zmsg_frame_size(params) != 3){
		zmsg_push_back(reply, zframe_newstr("400"));
		zmsg_push_back(reply, zframe_newstr("encrypt need <algorithm> <raw> <key> (3 frames)"));
		return reply;
	} 
	zframe_t* algor_frame = zmsg_pop_front(params);
	zframe_t* raw_pwd_frame = zmsg_pop_front(params);
	zframe_t* key_frame = zmsg_pop_front(params);

	char* raw_pwd = zframe_strdup(raw_pwd_frame);
	char* key = zframe_strdup(key_frame);

	char res[1024] = {0};

	if(zframe_streq(algor_frame, "KDE")){
		KDEncode(KDCOMPLEX_ENCODE,(unsigned char*)raw_pwd, strlen(raw_pwd),
			(unsigned char*)res, 64, key, strlen(key));
		zmsg_push_back(reply, zframe_newstr("200"));
		zmsg_push_back(reply, zframe_newstr(res));
	} else if(zframe_streq(algor_frame, "AES")){
		long long llkey = _atoi64(key);
		AES_Encrypt1(res, sizeof(res), llkey, raw_pwd);
		zmsg_push_back(reply, zframe_newstr("200"));
		zmsg_push_back(reply, zframe_newstr(res)); 
	} else {
		zmsg_push_back(reply, zframe_newstr("404"));
		zmsg_push_back(reply, zframe_newstr("encrypt method not support"));
	} 

	zfree(raw_pwd);
	zfree(key);
	zframe_destroy(&algor_frame);
	zframe_destroy(&raw_pwd_frame);
	zframe_destroy(&key_frame); 

	zmsg_destroy(&params);

	return reply;
}

zmsg_t* 
decrypt(zmsg_t* params){
	assert(params);
	zmsg_t* reply = zmsg_new();
	if(zmsg_frame_size(params) != 4){
		zmsg_push_back(reply, zframe_newstr("400"));
		zmsg_push_back(reply, zframe_newstr("decrypt need <algorithm> <public> <private> <pwd> (4 frames)"));
		return reply;
	} 
	zframe_t* algor_frame = zmsg_pop_front(params);
	zframe_t* public_frame = zmsg_pop_front(params);
	zframe_t* private_frame = zmsg_pop_front(params);
	zframe_t* password_frame = zmsg_pop_front(params);

	char* public_str = zframe_strdup(public_frame);
	char* private_str = zframe_strdup(private_frame);
	char* password_str = zframe_strdup(password_frame);

	char res[1024] = {0};

	if(zframe_streq(algor_frame, "WANGAN")){
		int len = DecryptStr(public_str, private_str, password_str, res, sizeof(res));
		if(len == -1){ 
			zmsg_push_back(reply, zframe_newstr("500"));
			zmsg_push_back(reply, zframe_newstr("decrpyt error"));
		} else {
			res[len] = '\0';
			zmsg_push_back(reply, zframe_newstr("200"));
			zmsg_push_back(reply, zframe_newstr(res)); 
		}

	} else {
		zmsg_push_back(reply, zframe_newstr("404"));
		zmsg_push_back(reply, zframe_newstr("decrypt method not support"));
	} 

	zfree(public_str);
	zfree(private_str);
	zfree(password_str);

	zframe_destroy(&public_frame);
	zframe_destroy(&private_frame); 
	zframe_destroy(&password_frame); 
	zframe_destroy(&algor_frame);

	zmsg_destroy(&params);

	return reply;
}


zmsg_t* 
handle_request(zmsg_t* request){

	assert(request);
	zmsg_t* reply = NULL;
	if(zmsg_frame_size(request) < 1){
		reply = zmsg_new();
		zmsg_push_back(reply, zframe_newstr("400"));
		zmsg_push_back(reply, zframe_newstr("format error, missing function frame"));
		zmsg_destroy(&request);
		return reply;
	}

	zframe_t* func_frame = zmsg_pop_front(request);
	if(zframe_streq(func_frame, "encrypt")){//encrypt
		reply = encrypt(request);
	} else if(zframe_streq(func_frame, "decrypt")){//decrypt 
		reply = decrypt(request);
	} else {
		reply = zmsg_new();
		zmsg_push_back(reply, zframe_newstr("404"));
		zmsg_push_back(reply, zframe_newstr("util function not support"));
		zmsg_destroy(&request);
	}

	zframe_destroy(&func_frame);
	return reply;
}

void* 
thread_func(void* args){
	assert(args);
	assert(g_zmq_context);
	assert(g_utils_cfg);

	char* broker = (char*)args;

	char temp[256];
	sprintf(temp, "%s", broker);
	char* host = strtok(temp, ":");
	char* portstr = strtok(NULL, ":");
	assert(host);
	assert(portstr);
	int port = atoi(portstr);   

	zbusconn_cfg_t cfg; 
	cfg.ctx = g_zmq_context;
	cfg.verbose = g_utils_cfg->debug; 
	strcpy(cfg.host, host);
	cfg.port = port; 

	zbusconn_t* zbus_conn = zbusconn_new(&cfg);  


	zbuswrk_cfg_t wrk_cfg;
	memset(&wrk_cfg, 0, sizeof(wrk_cfg));
	strcpy(wrk_cfg.service, g_utils_cfg->service_name);
	strcpy(wrk_cfg.reg_token, g_utils_cfg->service_regtoken);
	strcpy(wrk_cfg.acc_token, g_utils_cfg->service_acctoken);
	strcpy(wrk_cfg.mode, MODE_LB);

	zbuswrk_t* worker = zbuswrk_new(&wrk_cfg);
	assert(worker);

	zmsg_t* msg;
	while(1){
		msg = zbuswrk_recv(zbus_conn, worker);
		if(!msg) break;
		msg = handle_request(msg);
		assert(msg);
		zbuswrk_send(zbus_conn, worker, msg);
	}

	zfree(broker);

	return NULL;
}
 
int main(int argc, char* argv[]){   
	if(argc>1 && ( strcmp(argv[1],"help")==0 
		|| strcmp(argv[1],"--help")==0 )){
			printf("-v\tverbose mode to show message, e.g. -v0, -v1\n"); 
			printf("-s\tzbus service name e.g. -sUtils \n"); 
			printf("-b\tzbus brokers, split by',',  e.g. -b172.24.180.27:15555 \n");
			printf("-c\tzbus utils thread count e.g. -c1 \n"); 
			printf("-kreg\tzbus service registration token,  e.g. -kregxyz \n");
			printf("-kacc\tzbus service access token e.g. -kaccxyz \n"); 
			printf("-log\tlog file path,  e.g. -loglogs \n");
			return 0;
	}


	g_zmq_context= zctx_new(1);
	assert(g_zmq_context);
	g_utils_cfg = trade_cfg_new(argc, argv);
	
	if(g_utils_cfg->log_path){
		zlog_use_file(g_utils_cfg->log_path); 
	} else {
		zlog_use_stdout();
	}

	list_t* broker_list = list_new(); 
	char* brokers = zstrdup(g_utils_cfg->brokers);
	char* broker = strtok(brokers,",");
	while (broker){  
		list_push_back(broker_list, zstrdup(broker));
		broker = strtok(NULL, ",");
	} 
	zfree(brokers); 

	int thread_count = list_size(broker_list)* g_utils_cfg->worker_threads;
	zthread_t* threads = (zthread_t*)zmalloc(thread_count*sizeof(zthread_t)); 
	int t = 0; 
	while(1){
		char* broker = (char*)list_pop_front(broker_list);
		if(!broker) break;
		for(int i=0; i<g_utils_cfg->worker_threads; i++){
			threads[i] = zthread_new(thread_func, zstrdup(broker)); 
		}  
		zfree(broker);
	} 

	list_destroy(&broker_list);

	for(int i=0; i<thread_count; i++){
		zthread_join(threads[i]); 
	}  

	zctx_destroy(&g_zmq_context); 
	trade_cfg_destroy(&g_utils_cfg);

	return 0;
}