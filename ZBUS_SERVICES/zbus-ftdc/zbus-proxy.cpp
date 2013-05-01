#include "zbus/include/prelude.h"
#include "zbus/include/zbusapi.h"
#include "zbus/include/hash.h"  
#include "json/cJSON.h"
#include "ThostTraderApi/ThostFtdcTraderApi.h"
#include "ZBusSpi.h"

typedef struct _proxy_cfg_t{  
	char*	proxy_host;
	int		proxy_port; 
	int		proxy_reconnect; 

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
zmutex_t*		g_mutex_msgid;
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
	self->proxy_host = zstrdup(option(argc,argv, "-h", "asp-sim2-front1.financial-trading-platform.com"));
	self->proxy_port = atoi(option(argc,argv,"-p","26205"));
	self->proxy_reconnect = atoi(option(argc,argv,"-r","1000"));  

	self->verbose = atoi(option(argc,argv, "-v", "1"));
	self->debug = atoi(option(argc,argv, "-dbg", "0"));
	self->worker_threads = atoi(option(argc,argv, "-c", "2"));
	self->broker = zstrdup(option(argc,argv, "-b", "localhost:15555"));
	self->service_name = zstrdup(option(argc,argv, "-s", "FTDC"));
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
		if(self->proxy_host)
			zfree(self->proxy_host); 
		if(self->service_name)
			zfree(self->service_name);
		if(self->broker)
			zfree(self->broker);
		if(self->log_path)
			zfree(self->log_path);
		zfree(self);
	}
}

static int
s_next_msg_id(){
	zmutex_lock(g_mutex_msgid);
	static int id = 0;
	if(id == 99999999) id = 0;
	id++;
	zmutex_unlock(g_mutex_msgid);
	return id;
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
 
static int
ReqUserLogin(CThostFtdcTraderApi* api, cJSON* params){
	return 0;
}


typedef struct _api_broker_t{
	CThostFtdcTraderApi* api;
	char* broker;
}api_broker_t;

void* 
zbus_recv_thread(void* args){
	assert(args);
	assert(g_proxy_cfg);   
	assert(g_zmq_context);
	api_broker_t* api_broker = (api_broker_t*) args;

	char* broker = api_broker->broker;
	CThostFtdcTraderApi* api = api_broker->api;

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

	char error_msg[1024] = {0};  
	int rc; 

	while(1){   

		zmsg_t* request = zbuswrk_recv(zbus_conn, worker);
		if(!request) break; 

		zmsg_log(request, "FTDC REQ");

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
	zbusconn_destroy(&zbus_conn);   
	return NULL;
} 
typedef struct _zbusconn_mutex_t{
	zbusconn_t* conn;
	zmutex_t* mutex;
}zbusconn_mutex_t;

void*
reply_zbus_heartbeat(void* args){
	zbusconn_mutex_t* conn_mutex = (zbusconn_mutex_t*) args;
	zbusconn_t* conn = conn_mutex->conn;
	zmutex_t* mutex = conn_mutex->mutex;
	while(1){
		zclock_sleep(g_proxy_cfg->probe_interval);
		
		zmutex_lock(mutex);
		
		//send probe to keep reply zbusconn_t alive
		int probe_rc = zbusconn_probe(conn, g_proxy_cfg->probe_interval);
		if(probe_rc != 0){
			zbusconn_reconnect(conn);
		}

		zmutex_unlock(mutex);
	}
	return NULL;
}

void* 
ftdc_thread(void* args){
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

	zmutex_t* mutex = zmutex_new();

	char front_addr[256];
	sprintf(front_addr, "tcp://%s:%d", g_proxy_cfg->proxy_host, g_proxy_cfg->proxy_port);
	CThostFtdcTraderApi* api = CThostFtdcTraderApi::CreateFtdcTraderApi();
	
	ZBusSpi* spi = new ZBusSpi(zbus_conn, mutex, front_addr);
	api->RegisterSpi(spi);
	api->SubscribePublicTopic(TERT_RESTART);
	api->SubscribePrivateTopic(TERT_RESTART);
	api->RegisterFront(front_addr);
	api->Init();

	//1)start thread: heartbeating of reply connection
	zbusconn_mutex_t conn_mutex;
	conn_mutex.conn = zbus_conn;
	conn_mutex.mutex = mutex;
	zthread_t thread_hb = zthread_new(reply_zbus_heartbeat, &conn_mutex);
	//2)start thread: zbus recv
	api_broker_t api_broker;
	api_broker.api = api;
	api_broker.broker = broker;
	zthread_t thread_recv = zthread_new(zbus_recv_thread, &api_broker);

	api->Join();
	api->Release();
	
	zthread_join(thread_hb);
	zthread_join(thread_recv);

	zfree(broker);
	zbusconn_destroy(&zbus_conn);  
	zmutex_destroy(&mutex);
	return NULL;
}
int main(int argc, char* argv[]){ 
	if(argc>1 && ( strcmp(argv[1],"help")==0 
		|| strcmp(argv[1],"--help")==0 )){
			printf("-v\tverbose mode to show message, e.g. -v0, -v1\n"); 
			printf("-h\tproxy host e.g. -h172.24.180.134 \n");
			printf("-p\tproxy port e.g. -p21000 \n");   
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
	g_mutex_msgid = zmutex_new();
	g_mutex_hash = zmutex_new(); 
	g_msgid2reply = hash_new(&g_hctrl_msgid2reply, NULL);
	

	char* broker = zstrdup(g_proxy_cfg->broker);
	
	int thread_count = g_proxy_cfg->worker_threads;
	zthread_t* threads = (zthread_t*)zmalloc(thread_count*sizeof(zthread_t)); 

	for(int i=0; i<g_proxy_cfg->worker_threads; i++){ 
		threads[i] = zthread_new(ftdc_thread, zstrdup(broker));
	}  
	zfree(broker); 
   
	for(int i=0; i<thread_count; i++){ 
		zthread_join(threads[i]);
	} 
 
	zmutex_destroy(&g_mutex_hash); 
	zmutex_destroy(&g_mutex_msgid); 
	hash_destroy(&g_msgid2reply);
	proxy_cfg_destroy(&g_proxy_cfg); 
	zctx_destroy(&g_zmq_context);

	return 0;
}