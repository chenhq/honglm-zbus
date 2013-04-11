#include "zbus/include/prelude.h"
#include "zbus/include/zbusapi.h"
#include "zbus/include/hash.h"  

#include "msmqcli.h" 

const char* g_sn_delimit = "~_*&#^#&*_~";

typedef struct _msmq_cfg_t{ 
	char* msmq_server;
	char* msmq_client; 
	int	  msmq_timeout;

	int   worker_threads;
	int   verbose;
	int	  debug;
	char* service_name; 
	char* service_regtoken;
	char* service_acctoken;
	char* broker;

	char* log_path;
} msmq_cfg_t;
 
typedef struct _reply_t{
	zframe_t* sock_id;
	zframe_t* msg_id;
	zmsg_t* reply;
	int64_t ts;
}reply_t;

//////////////////////////////////////////////////////////////////////////
//global 
msmq_cfg_t*		g_msmq_cfg;
zmutex_t*		g_mutex_msgid;
zmutex_t*		g_mutex_msmq; 

zmutex_t*		g_mutex_hash;   
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

 

msmq_cfg_t*
msmq_cfg_new(int argc, char* argv[]){
	msmq_cfg_t* self = (msmq_cfg_t*)zmalloc(sizeof(msmq_cfg_t));
	assert(self);
	memset(self, 0, sizeof(msmq_cfg_t));
	
	self->verbose = atoi(option(argc, argv, "-v", "1"));
	self->debug = atoi(option(argc, argv, "-dbg", "0"));
	self->msmq_server = zstrdup(option(argc, argv, "-msmq_s", "172.24.180.180"));
	self->msmq_client = zstrdup(option(argc, argv, "-msmq_c", "172.24.20.21"));
	self->msmq_timeout = atoi(option(argc, argv, "-msmq_t", "10000")); 
	self->worker_threads = atoi(option(argc, argv, "-c", "1"));
	self->service_name = zstrdup(option(argc, argv, "-s", "Trade"));
	self->service_regtoken = zstrdup(option(argc,argv, "-kreg", ""));
	self->service_acctoken = zstrdup(option(argc,argv, "-kacc", ""));
	self->broker = zstrdup(option(argc,argv, "-b", "localhost:15555"));
	self->log_path = zstrdup(option(argc,argv, "-log", NULL));

	return self;
}

void
msmq_cfg_destroy(msmq_cfg_t** self_p){
	assert(self_p);
	msmq_cfg_t* self = *self_p;
	if(self){
		if(self->msmq_client)
			zfree(self->msmq_client);
		if(self->msmq_server)
			zfree(self->msmq_server);
		if(self->service_name)
			zfree(self->service_name);
		if(self->broker)
			zfree(self->broker);
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

static int
generate_msg_id(){
	zmutex_lock(g_mutex_msgid);
	static int id = 0;
	if(id == 99999999) id = 0;
	id++;
	zmutex_unlock(g_mutex_msgid);
	return id;
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
	conn_cfg.verbose = g_msmq_cfg->debug; 
	strcpy(conn_cfg.host, host);
	conn_cfg.port = port; 

	zbusconn_t* zbus_conn = zbusconn_new(&conn_cfg);   

	while(1){
		try{ 
			zmutex_lock(g_mutex_msmq);
			mqcli_t* mqcli = mqcli_new(g_msmq_cfg->msmq_server, g_msmq_cfg->msmq_client, g_msmq_cfg->msmq_timeout);
			zmutex_unlock(g_mutex_msmq);

			IMSMQMessagePtr	pmsg("MSMQ.MSMQMessage");
			_variant_t	timeout((long)mqcli->timeout); 
			_variant_t  want_body((bool)true);
			assert(mqcli);
			while(1){   
				pmsg = mqcli->qrecv->Receive(&vtMissing, &vtMissing, &want_body, &timeout);
				if(pmsg == NULL){
					int rc = zbusconn_probe(zbus_conn, mqcli->timeout);
					if(rc != 0){//reconnect
						zbusconn_reconnect(zbus_conn);
					}
					continue;   
				}

				_bstr_t body = pmsg->Body;  

				char* msmq_msg = (char*)body;  
				if(g_msmq_cfg->verbose){
					zlog("[MSMQ] REP: %s\n", msmq_msg);
				}
				char* split = strstr(msmq_msg, g_sn_delimit);
				if(split == NULL){
					zlog("[ERROR]: MSMQ invalid message\n");
					continue;
				}
				 
				*split = '\0';
				char* msg_id_str = msmq_msg;
				char* msg_body = split+strlen(g_sn_delimit); 
				
				zmsg_t* reply = zmsg_new(); 
				zmsg_push_back(reply, zframe_newstr("200"));//200 indicate ok
				zmsg_push_back(reply, zframe_new(msg_body, strlen(msg_body)));

				zframe_t* sock_id = NULL;
				zframe_t* msg_id = NULL;

				zmutex_lock(g_mutex_hash);
				reply_t* sock_reply = (reply_t*)hash_get(g_msgid2reply, msg_id_str);
				if(sock_reply){ 
					assert(sock_reply->sock_id);
					assert(sock_reply->msg_id);

					assert(!sock_reply->reply);  

					sock_id = sock_reply->sock_id;
					msg_id = sock_reply->msg_id;

					hash_rem(g_msgid2reply, msg_id_str);//remove mapping 
					reply_destroy(&sock_reply);
				} else {
					sock_reply = reply_new(NULL, NULL, reply); 
					hash_put(g_msgid2reply, msg_id_str, sock_reply);
				}
				zmutex_unlock(g_mutex_hash);

				if(sock_id){  

					if(g_msmq_cfg->debug){
						char* sock_id_str = zframe_strhex(sock_id);
						zlog("[REP] MSMQ(%s)=>ZBUS(%s#%s)\n",msg_id_str, broker, sock_id_str);
						zfree(sock_id_str);
					} 

					int rc = s_client_reply(zbus_conn, sock_id, msg_id, reply);
					if(rc != 0){
						zlog("ERROR: reply back error encountered");
					}   
				}
			}
			mqcli_destroy(&mqcli);
		}catch(_com_error& e){ 
			wprintf(L"Error Code = 0x%X\nError Description = %s\n", e.Error(), (wchar_t *)e.Description());
			zclock_sleep(100); //
			zlog("Going to retry recv from MSMQ...\n");
		} 
	}
	zfree(broker);
	zbusconn_destroy(&zbus_conn);
	return NULL;
}

void*
thread_recv(void* args){
	assert(args);
	assert(g_msmq_cfg);   
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
	conn_cfg.verbose = g_msmq_cfg->debug; 
	strcpy(conn_cfg.host, host);
	conn_cfg.port = port; 

	zbusconn_t* zbus_conn = zbusconn_new(&conn_cfg);  


	zbuswrk_cfg_t wrk_cfg;
	memset(&wrk_cfg, 0, sizeof(wrk_cfg));
	strcpy(wrk_cfg.service, g_msmq_cfg->service_name);
	strcpy(wrk_cfg.reg_token, g_msmq_cfg->service_regtoken);
	strcpy(wrk_cfg.acc_token, g_msmq_cfg->service_acctoken);
	strcpy(wrk_cfg.mode, MODE_LB);

	zbuswrk_t* worker = zbuswrk_new(&wrk_cfg);
	assert(worker);


	zmutex_lock(g_mutex_msmq);
	mqcli_t* mqcli = mqcli_new(g_msmq_cfg->msmq_server, g_msmq_cfg->msmq_client, g_msmq_cfg->msmq_timeout);
	zmutex_unlock(g_mutex_msmq);
	assert(mqcli);

	while(1){ 
		zmsg_t* request = zbuswrk_recv(zbus_conn, worker);
		if(!request) break; 

		zframe_t* sock_id, *msg_id;
		zbuswrk_get_address(worker, &sock_id, &msg_id);

		if(zmsg_frame_size(request) < 1){
			zmsg_t* reply = zmsg_new();
			zmsg_push_back(reply, zframe_newstr("400"));
			zmsg_push_back(reply, zframe_newstr("format error, require <frame>..."));
			int rc = s_client_reply(zbus_conn, sock_id, msg_id, reply);
			if(rc != 0){
				zlog("ERROR: reply back error encountered");
			} 
			continue;
		}

		//call trade
		char msg_id_str[12]; 
		sprintf(msg_id_str,"%010d", generate_msg_id());
		 
		zframe_t* body = zmsg_pop_front(request); //other frame ignored
		char* body_str = zframe_strdup(body);

		char sn_prefix[256];
		sprintf(sn_prefix, "%s%s", msg_id_str, g_sn_delimit); 

		size_t size = strlen(sn_prefix)+strlen(body_str)+1;
		char* msmq_msg = (char*)zmalloc(size);
		memset(msmq_msg, 0, size);
		strcpy(msmq_msg, sn_prefix); 
		strcat(msmq_msg, body_str); 
		if(g_msmq_cfg->verbose){
			zlog("[MSMQ] REQ: %s\n", msmq_msg);
		}

		int rc = mqcli_send(mqcli, msmq_msg);
		if(rc == 0){
			
			if(g_msmq_cfg->debug){
				char* sock_id_str = zframe_strhex(sock_id);
				zlog("[REQ] ZBUS(%s#%s)=>MSMQ(%s)\n", broker, sock_id_str, msg_id_str);
				zfree(sock_id_str);
			}

			zmsg_t* reply = NULL;
			zmutex_lock(g_mutex_hash); 
			reply_t* sock_reply = (reply_t*)hash_get(g_msgid2reply, msg_id_str);
			if(sock_reply){ //reply may already returned
				assert(sock_reply->sock_id == NULL);
				assert(sock_reply->msg_id == NULL);
				assert(sock_reply->reply); 

				reply = sock_reply->reply;

				hash_rem(g_msgid2reply, msg_id_str);//remove mapping 
				reply_destroy(&sock_reply);

			} else {
				sock_reply = reply_new(sock_id, msg_id, NULL);
				hash_put(g_msgid2reply, msg_id_str, sock_reply);
			} 
			zmutex_unlock(g_mutex_hash); 
			
			if(reply){ 
				int rc = s_client_reply(zbus_conn, sock_id, msg_id, reply);
				if(rc != 0){
					zlog("ERROR: reply back error encountered");
				}   
			}
		} else {
			zframe_destroy(&sock_id);
			zframe_destroy(&msg_id);
		}

		zfree(msmq_msg); 
		zfree(body_str);
		zframe_destroy(&body); 
		zmsg_destroy(&request);
	} 
	zfree(broker);
	return NULL;
}


 
int main(int argc, char* argv[]){   
	if(argc>1 && ( strcmp(argv[1],"help")==0 
		|| strcmp(argv[1],"--help")==0 )){
			printf("-v\tverbose mode to show message, e.g. -v0, -v1\n");
			printf("-msmq_s\t msmq server e.g. -msmq_s127.0.0.1\n");
			printf("-msmq_c\t msmq client e.g. -msmq_c127.0.0.1 \n");
			printf("-msmq_t\t msmq recv timeout e.g. -msmq_t1000 \n");
			printf("-c\tmsmq proxy thread count e.g. -c1 \n"); 
			printf("-s\t zbus service name e.g. -sTrade \n"); 
			printf("-b\tzbus broker,  e.g. -b172.24.180.27:15555 \n");
			printf("-kreg\tzbus service registration token,  e.g. -kregxyz \n");
			printf("-kacc\tzbus service access token e.g. -kaccxyz \n"); 
			printf("-log\tlog file path,  e.g. -loglogs \n");
			return 0;
	}


	g_zmq_context = zctx_new(1);
	assert(g_zmq_context);

	g_mutex_msgid = zmutex_new(); 
	g_mutex_msmq = zmutex_new();
	g_mutex_hash = zmutex_new(); 

	g_zmq_context = zctx_new(1);
	g_msgid2reply = hash_new(&g_hctrl_msgid2reply, NULL);
	g_msmq_cfg = msmq_cfg_new(argc, argv); 
	
	if(g_msmq_cfg->log_path){
		zlog_use_file(g_msmq_cfg->log_path); 
	} else {
		zlog_use_stdout();
	}
	 
	char* broker = zstrdup(g_msmq_cfg->broker);

	int thread_count = g_msmq_cfg->worker_threads;
	zthread_t* reply_threads = (zthread_t*)zmalloc(thread_count*sizeof(zthread_t));
	zthread_t* recv_threads = (zthread_t*)zmalloc(thread_count*sizeof(zthread_t));

	for(int i=0; i<thread_count; i++){
		reply_threads[i] = zthread_new(thread_reply, zstrdup(broker));
		recv_threads[i] = zthread_new(thread_recv, zstrdup(broker));
	} 
	zfree(broker);

	for(int i=0; i<thread_count; i++){
		zthread_join(reply_threads[i]);
		zthread_join(recv_threads[i]);
	}  

	zctx_destroy(&g_zmq_context);
	zmutex_destroy(&g_mutex_msgid);
	zmutex_destroy(&g_mutex_hash);
	zmutex_destroy(&g_mutex_msmq); 
	hash_destroy(&g_msgid2reply);

	msmq_cfg_destroy(&g_msmq_cfg);
	return 0;
}