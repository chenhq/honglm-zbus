#include "zbus/include/prelude.h"
#include "zbus/include/zbusapi.h"
#include "zbus/include/hash.h"   

typedef struct _proxy_cfg_t{ 
	char*	proxy_name;
	char*	proxy_host;
	int		proxy_port; 
	int		proxy_reconnect;

	char*	auth_user;
	char*	auth_pwd;

	int		verbose;
	int		debug;
	char*	broker; 
	char*	service_name;
	char*	service_regtoken;
	char*	service_acctoken;
	int		worker_threads;

	int		probe_interval;

	char*   log_path;

} proxy_cfg_t;

typedef struct _reply_t{
	zframe_t* sock_id;
	zframe_t* msg_id;
	zmsg_t* reply;
	int64_t ts;
}reply_t;

//////////////////////////////////////////////////////////////////////////
//global 
zmutex_t*		g_mutex_hash; 
proxy_cfg_t*	g_proxy_cfg;   
void*			g_zmq_context;  
hash_t*			g_msgid2reply;
hash_ctrl_t		g_hctrl_msgid2reply = {
	hash_func_string,			/* hash function */
	hash_dup_string,			/* key dup */
	NULL,				        /* val dup */
	hash_cmp_string,			/* key compare */
	hash_destroy_string,		/* key destructor */
	NULL,			            /* val destructor */
};

//////////////////////////////////////////////////////////////////////////

reply_t*
reply_new(zframe_t* sock_id, zframe_t* msg_id, zmsg_t* reply){
	reply_t* self = (reply_t*)zmalloc(sizeof(reply_t));
	self->sock_id = sock_id;
	self->msg_id = msg_id;
	self->reply = reply;
	self->ts = zclock_time();

	return self;
}
void
reply_destroy(reply_t** self_p){
	assert(self_p);
	reply_t* self = *self_p;
	if(self){
		zfree(self);
		*self_p = NULL;
	}
}

proxy_cfg_t*
proxy_cfg_new(int argc, char* argv[]){
	proxy_cfg_t* self = (proxy_cfg_t*)zmalloc(sizeof(proxy_cfg_t));
	assert(self);
	memset(self, 0, sizeof(proxy_cfg_t));
	self->proxy_name = zstrdup(option(argc,argv, "-x", "zbus_kcxp")); 
	self->proxy_host = zstrdup(option(argc,argv, "-h", "172.24.180.216"));
	self->proxy_port = atoi(option(argc,argv,"-p","21000"));
	self->proxy_reconnect = atoi(option(argc,argv,"-r","1000")); 
	self->auth_user = zstrdup(option(argc,argv, "-u", "KCXP00"));
	self->auth_pwd = zstrdup(option(argc,argv, "-P", "888888"));

	self->verbose = atoi(option(argc,argv, "-v", "1"));
	self->debug = atoi(option(argc,argv, "-dbg", "0"));
	self->worker_threads = atoi(option(argc,argv, "-c", "2"));
	self->broker = zstrdup(option(argc,argv, "-b", "localhost:15555"));
	self->service_name = zstrdup(option(argc,argv, "-s", "KCXP"));
	self->service_regtoken = zstrdup(option(argc,argv, "-kreg", ""));
	self->service_acctoken = zstrdup(option(argc,argv, "-kacc", ""));
	self->probe_interval = atoi(option(argc,argv, "-t", "6000"));

	self->log_path = zstrdup(option(argc,argv, "-log", NULL));
	return self;
}

void
proxy_cfg_destroy(proxy_cfg_t** self_p){
	assert(self_p);
	proxy_cfg_t* self = *self_p;
	if(self){ 
		if(self->proxy_name)
			zfree(self->proxy_name);
		if(self->proxy_host)
			zfree(self->proxy_host); 
		if(self->auth_user)
			zfree(self->auth_user);
		if(self->auth_pwd)
			zfree(self->auth_pwd);
		if(self->service_name)
			zfree(self->service_name);
		if(self->broker)
			zfree(self->broker);
		if(self->log_path)
			zfree(self->log_path);
		zfree(self);
	}
}
 

static void
s_log_request(zmsg_t* msg, char* funcid){
}

static void
s_log_reply(zmsg_t* msg){

}

static int
s_client_reply(zbusconn_t* conn, zframe_t* sock_id, zframe_t* msg_id, zmsg_t* reply){
	zmsg_push_front(reply, msg_id);
	zmsg_push_front(reply, zframe_newstr(MDPC)); 
	int rc = zbusconn_route(conn, sock_id, reply);
	return rc;
}

void*
thread_reply(void* args){  
	assert(args);
	char* broker = (char*)args;

	char temp[256];
	sprintf(temp, "%s", broker);
	char* host = strtok(temp, ":");
	char* portstr = strtok(NULL, ":");
	assert(host);
	assert(portstr);
	int port = atoi(portstr);   

	zbusconn_cfg_t conn_cfg; 
	memset(&conn_cfg, 0, sizeof(conn_cfg));
	conn_cfg.ctx = g_zmq_context;
	conn_cfg.verbose = g_proxy_cfg->debug; 
	strcpy(conn_cfg.host, host);
	conn_cfg.port = port; 
	 
	zbusconn_t* zbus_conn = zbusconn_new(&conn_cfg);  
 

	char error_code[1024] = {0};
	char field_value[1024] = {0};
	char column_name[1024] = {0}; 
	char debug_info[1024] = {0};
	char error_msg[1024] = {0};
	char msg_info[1024] = {0};
 
	int rc;

	void* proxy = NULL;
	while(1){
		while(!proxy){ //assume proxy ok, if not reconnect
			zlog("Proxy(%s:%d) down, reconnecting...\n", g_proxy_cfg->proxy_host,g_proxy_cfg->proxy_port);
			zclock_sleep(g_proxy_cfg->proxy_reconnect);
			//reconnect
		} 
	}
	zfree(broker);
	zbusconn_destroy(&zbus_conn);
	return NULL;
}


void* 
thread_recv(void* args){
	assert(args);
	assert(g_proxy_cfg);   
	assert(g_zmq_context);
	
	char* broker = (char*)args; 

	char temp[256];
	sprintf(temp, "%s", broker);
	char* host = strtok(temp, ":");
	char* portstr = strtok(NULL, ":");
	assert(host);
	assert(portstr);
	int port = atoi(portstr);   

	zbusconn_cfg_t conn_cfg; 
	memset(&conn_cfg, 0, sizeof(conn_cfg));
	conn_cfg.ctx = g_zmq_context;
	conn_cfg.verbose = g_proxy_cfg->debug; 
	strcpy(conn_cfg.host, host);
	conn_cfg.port = port; 
	
	zbusconn_t* zbus_conn = zbusconn_new(&conn_cfg);  


	zbuswrk_cfg_t wrk_cfg;
	memset(&wrk_cfg, 0, sizeof(wrk_cfg));
	strcpy(wrk_cfg.service, g_proxy_cfg->service_name);
	strcpy(wrk_cfg.reg_token, g_proxy_cfg->service_regtoken);
	strcpy(wrk_cfg.acc_token, g_proxy_cfg->service_acctoken);
	strcpy(wrk_cfg.mode, MODE_LB);

	zbuswrk_t* worker = zbuswrk_new(&wrk_cfg);
	assert(worker);

	//create proxy connection
	void* proxy =  NULL;
	char error_msg[1024] = {0};  
	int rc; 

	while(1){  
		while(!proxy){ //assume proxy ok, if not reconnect
			zlog("Proxy(%s:%d) down, reconnecting...\n", g_proxy_cfg->proxy_host,g_proxy_cfg->proxy_port);
			zclock_sleep(g_proxy_cfg->proxy_reconnect);
			//reconnect
		}

		zmsg_t* request = zbuswrk_recv(zbus_conn, worker);
		if(!request) break; 


		zframe_t* sock_id, *msg_id;
		zbuswrk_get_address(worker, &sock_id, &msg_id);

		if(zmsg_frame_size(request) % 2 == 0){
			zmsg_destroy(&request); 
			zmsg_t* reply = zmsg_new(); 
			zmsg_push_back(reply, zframe_newstr("400"));
			zmsg_push_back(reply, zframe_newstr("request format: <funcid> <key_i> <val_i> ... "));
			
			int rc = s_client_reply(zbus_conn, sock_id, msg_id, reply);

			if(rc != 0){
				zlog("ERROR: reply back error encountered");
			} 
			continue;
		}

		zframe_t* funcid_frame = zmsg_pop_front(request);
		char* funcid = zframe_strdup(funcid_frame);

		if(g_proxy_cfg->verbose){ 
			s_log_request(request, funcid);
		}
	}
	zfree(broker);
	zbusconn_destroy(&zbus_conn);  
	//destroy proxy

	return NULL;
} 

 
int main(int argc, char* argv[]){ 
	if(argc>1 && ( strcmp(argv[1],"help")==0 
		|| strcmp(argv[1],"--help")==0 )){
			printf("-v\tverbose mode to show message, e.g. -v0, -v1\n");
			printf("-x\tproxy server name, e.g. -xzbus_kcxp\n");
			printf("-h\tproxy host e.g. -h172.24.180.134 \n");
			printf("-p\tproxy port e.g. -p21000 \n");  
			printf("-u\tproxy auth user e.g. -uKCXP00 \n");
			printf("-P\tproxy auth password e.g. -P888888 \n");
			printf("-c\tzbus-proxy worker count, concurrency e.g. -c2 \n");
			printf("-s\tservice name, concurrency e.g. -skcxp \n");
			printf("-kreg\tservice register token, e.g. -kregzbus \n");
			printf("-kacc\tservice access token, e.g. -kacczbus \n");
			printf("-b\tzbus broker, e.g. -b172.24.180.27:15555 \n");
			printf("-log\tlog path,  e.g. -log. \n");
			return 0;
	}   

	g_proxy_cfg = proxy_cfg_new(argc,argv); 
	assert(g_proxy_cfg);

	if(g_proxy_cfg->log_path){
		zlog_use_file(g_proxy_cfg->log_path); 
	} else {
		zlog_use_stdout();
	}  
	g_zmq_context = zctx_new(1);
	g_mutex_hash = zmutex_new(); 
	g_msgid2reply = hash_new(&g_hctrl_msgid2reply, NULL);
	

	char* broker = zstrdup(g_proxy_cfg->broker);
	
	int thread_count = g_proxy_cfg->worker_threads;
	zthread_t* reply_threads = (zthread_t*)zmalloc(thread_count*sizeof(zthread_t));
	zthread_t* recv_threads = (zthread_t*)zmalloc(thread_count*sizeof(zthread_t));

	for(int i=0; i<g_proxy_cfg->worker_threads; i++){
		reply_threads[i] = zthread_new(thread_reply, zstrdup(broker));
		recv_threads[i] = zthread_new(thread_recv, zstrdup(broker));
	}  
	zfree(broker); 
   
	for(int i=0; i<thread_count; i++){
		zthread_join(reply_threads[i]);
		zthread_join(recv_threads[i]);
	} 
 
	zmutex_destroy(&g_mutex_hash);  
	hash_destroy(&g_msgid2reply);
	proxy_cfg_destroy(&g_proxy_cfg); 
	zctx_destroy(&g_zmq_context);

	return 0;
}