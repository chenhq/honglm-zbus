#include "zbus/include/prelude.h"
#include "zbus/include/zbusapi.h"
#include "zbus/include/hash.h"  
#include "kcxp/include/KCBPCli.h"

typedef struct _kcxp_cfg_t{ 
	char*	kcxp_name;
	char*	kcxp_host;
	int		kcxp_port;
	char*	sendq_name;
	char*	recvq_name; 
	int		kcxp_reconnect;

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

} kcxp_cfg_t;

typedef struct _reply_t{
	zframe_t* sock_id;
	zframe_t* msg_id;
	zmsg_t* reply;
	int64_t ts;
}reply_t;

//////////////////////////////////////////////////////////////////////////
//global 
zmutex_t*		g_mutex_hash; 
kcxp_cfg_t*		g_kcbp_cfg;   
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

kcxp_cfg_t*
kcxp_cfg_new(int argc, char* argv[]){
	kcxp_cfg_t* self = (kcxp_cfg_t*)zmalloc(sizeof(kcxp_cfg_t));
	assert(self);
	memset(self, 0, sizeof(kcxp_cfg_t));
	self->kcxp_name = zstrdup(option(argc,argv, "-x", "zbus_kcxp")); 
	self->kcxp_host = zstrdup(option(argc,argv, "-h", "172.24.180.216"));
	self->kcxp_port = atoi(option(argc,argv,"-p","21000"));
	self->kcxp_reconnect = atoi(option(argc,argv,"-r","1000"));
	self->sendq_name = zstrdup(option(argc,argv, "-qs", "req_zb"));
	self->recvq_name = zstrdup(option(argc,argv, "-qr", "ans_zb"));
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
kcxp_cfg_destroy(kcxp_cfg_t** self_p){
	assert(self_p);
	kcxp_cfg_t* self = *self_p;
	if(self){ 
		if(self->kcxp_name)
			zfree(self->kcxp_name);
		if(self->kcxp_host)
			zfree(self->kcxp_host);
		if(self->sendq_name)
			zfree(self->sendq_name);
		if(self->recvq_name)
			zfree(self->recvq_name);
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

void
kcxpcli_destroy(void** self_p){
	assert(self_p);
	void* self = *self_p;
	if(self){
		KCBPCLI_DisConnectForce(self);	
		KCBPCLI_Exit(self);
		*self_p = NULL;
	}
}


void* 
kcxpcli_new(kcxp_cfg_t* cfg){
	KCBPCLIHANDLE self; 
	tagKCBPConnectOption conn; 

	int rc;
	rc = KCBPCLI_Init(&self); 
	assert(rc == 0);

	memset(&conn, 0, sizeof(conn)); 
	strcpy(conn.szServerName, cfg->kcxp_name); 
	conn.nProtocal = 0; 
	strcpy(conn.szAddress, cfg->kcxp_host); 
	conn.nPort = cfg->kcxp_port; 
	strcpy(conn.szSendQName, cfg->sendq_name); 
	strcpy(conn.szReceiveQName, cfg->recvq_name); 

	KCBPCLI_SetOptions(self, KCBP_OPTION_CONNECT, &conn, sizeof(conn));

	int auth = 0; //for asynchronise mode
	KCBPCLI_SetOptions(self, KCBP_OPTION_AUTHENTICATION, &auth, sizeof(auth));

	zlog("KCXP Connecting ...\n");
	rc = KCBPCLI_ConnectServer(self, cfg->kcxp_name, cfg->auth_user, cfg->auth_pwd); 

	if(rc != 0){
		zlog("KCXP Connect failed: %d\n", rc);
		kcxpcli_destroy(&self);
	} else {
		zlog("KCXP Connected\n", rc);
	}
	return self;
}


void
kcxpcli_clear_data(void* kcbp){
	assert(kcbp);
	do {
		while( KCBPCLI_SQLFetch(kcbp) == 0 );
	}while( KCBPCLI_SQLMoreResults( kcbp ) == 0 );

	KCBPCLI_SQLCloseCursor(kcbp);
}


static void
s_log_kcxp_reply(zmsg_t* msg){
	assert(msg);
	msg = zmsg_dup(msg);
	if(zmsg_frame_size(msg) < 1) return;
	zframe_t* frame = zmsg_pop_front(msg);
	char* frame_str = zframe_strdup(frame);
	int rsCount = atoi(frame_str);
	zfree(frame_str);
	zframe_destroy(&frame);

	FILE* file = zlog_get_log_file();

	zlog("KCXP 应答(结果集: %d): \n", rsCount);
	for(int rs=0; rs<rsCount; rs++){
		fprintf(file, "第%d结果集: \n", rs+1);
		
		frame = zmsg_pop_front(msg);
		frame_str = zframe_strdup(frame);
		int rows, cols;
		sscanf(frame_str,"%d,%d", &rows, &cols);
		zfree(frame_str);
		zframe_destroy(&frame);
		for(int r=0; r<rows; r++){
			fprintf(file, "记录%d: ", r+1);
			for(int c=0; c<cols; c++){
				zframe_t* key_frame = zmsg_pop_front(msg);
				zframe_t* val_frame = zmsg_pop_front(msg);
				char* key = zframe_strdup(key_frame);
				char* val = "";
				if(zframe_size(val_frame)>1024){
					val = "{大数据块}";
				} else {
					val = zframe_strdup(val_frame);
				} 

				fprintf(file, "[%s=%s] ",key, val);
				zframe_destroy(&key_frame);
				zframe_destroy(&val_frame);
				zfree(key);
				zfree(val);
			}
			fprintf(file, "\n");
		}
	} 
	zmsg_destroy(&msg);
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
	conn_cfg.verbose = g_kcbp_cfg->debug; 
	strcpy(conn_cfg.host, host);
	conn_cfg.port = port; 
	 
	zbusconn_t* zbus_conn = zbusconn_new(&conn_cfg);  
 

	char error_code[1024] = {0};
	char field_value[1024] = {0};
	char column_name[1024] = {0}; 
	char debug_info[1024] = {0};
	char error_msg[1024] = {0};
	char msg_info[1024] = {0};

	tagCallCtrl bpctrl;
	int rc;

	void* kcbp = kcxpcli_new(g_kcbp_cfg);
	while(1){
		
		while(!kcbp){ //assume kcbp ok, if not reconnect
			zlog("KCXP(%s:%d) down, reconnecting...\n", g_kcbp_cfg->kcxp_host,g_kcbp_cfg->kcxp_port);
			zclock_sleep(g_kcbp_cfg->kcxp_reconnect);
			kcbp = kcxpcli_new(g_kcbp_cfg);
		}
		memset(&bpctrl, 0, sizeof(bpctrl));

		int expiry = g_kcbp_cfg->probe_interval/1000;
		if(expiry == 0) expiry = 10;

		bpctrl.nExpiry = expiry;
		strcpy(bpctrl.szMsgId,"0");
		strcpy(bpctrl.szCorrId, g_kcbp_cfg->kcxp_name);
		KCBPCLI_BeginWrite(kcbp);
		rc = KCBPCLI_GetReply(kcbp, &bpctrl);

		if( rc != 0){
			if(rc == 2003 || rc == 2004 || rc == 2054 || rc == 2055){ 
				KCBPCLI_GetErrorMsg(kcbp, error_msg);
				zlog("WARN: KCXP failed(code=%d,msg=%s)\n", rc, error_msg);
				//require reconnect
				kcxpcli_destroy(&kcbp); 
			}
			//send probe to keep reply zbusconn_t alive
			int probe_rc = zbusconn_probe(zbus_conn, g_kcbp_cfg->probe_interval);
			if(probe_rc != 0){
				zbusconn_reconnect(zbus_conn);
			}
			continue;
		} 
		
		zmsg_t* reply = zmsg_new();
		int ret_code = 200;  
		int cols = 0;
		rc = KCBPCLI_SQLNumResultCols( kcbp, &cols );
		if(cols < 3){
			ret_code = 900;
			kcxpcli_clear_data(kcbp); 
			sprintf(msg_info, "error code: %d", rc);
			zmsg_push_back(reply, zframe_newstr(msg_info));  
		} else {
			KCBPCLI_SQLFetch( kcbp );  
			KCBPCLI_RsGetCol( kcbp, 1, field_value);
			KCBPCLI_RsGetCol( kcbp, 2, error_code);
			KCBPCLI_RsGetCol( kcbp, 3, field_value); 
			if(cols == 4)
				KCBPCLI_RsGetCol( kcbp, 4, debug_info); 

			if(atoi(error_code) != 0){
				ret_code = 900; //90x: business error code
				zmsg_push_back(reply, zframe_newstr(error_code));
				zmsg_push_back(reply, zframe_newstr(field_value));  
				if(g_kcbp_cfg->verbose){ 
					zlog("KCXP 应答 KCXP业务错误: %s,%s\n",error_code, field_value);
				}
				sprintf(msg_info, "%d", ret_code); 
				zmsg_push_front(reply, zframe_newstr(msg_info));
			} else { 
				int rs_count = 0;
				while(KCBPCLI_SQLMoreResults(kcbp) == 0){
					rs_count++; 
					int cols;
					int rc = KCBPCLI_SQLNumResultCols(kcbp, &cols);
					if(rc != 0) continue; 
					zmsg_t* rs = zmsg_new();
					int rows = 0;
					while(KCBPCLI_SQLFetch(kcbp) == 0){
						rows++;
						for( int i=1; i<=cols; i++){
							unsigned char* col_val;
							long  col_len;
							KCBPCLI_RsGetColName(kcbp, i, column_name, 1024);
							KCBPCLI_RsGetVal(kcbp, i, &col_val, &col_len);
							zmsg_push_back(rs, zframe_newstr(column_name));
							zmsg_push_back(rs, zframe_new(col_val, col_len));
						}
					}
					sprintf(msg_info, "%d,%d", rows, cols); 
					zmsg_push_front(rs, zframe_newstr(msg_info));
					zframe_t* frame = zmsg_pop_front(rs);
					while(frame){
						zmsg_push_back(reply, frame);
						frame = zmsg_pop_front(rs);
					}
					zmsg_destroy(&rs);
				} 
				sprintf(msg_info, "%d", rs_count); 
				zmsg_push_front(reply, zframe_newstr(msg_info));

				KCBPCLI_SQLCloseCursor(kcbp);
				if(g_kcbp_cfg->verbose){ 
					s_log_kcxp_reply(reply);
				}
				sprintf(msg_info, "%d", ret_code); 
				zmsg_push_front(reply, zframe_newstr(msg_info));
			}	 
		}
		
		
		zframe_t* sock_id = NULL;
		zframe_t* msg_id = NULL;
		zmutex_lock(g_mutex_hash);
		reply_t* sock_reply = (reply_t*)hash_get(g_msgid2reply, bpctrl.szMsgId);
		if(sock_reply){ 
			assert(sock_reply->sock_id);
			assert(sock_reply->msg_id);

			assert(!sock_reply->reply);  
			
			sock_id = sock_reply->sock_id;
			msg_id = sock_reply->msg_id;
			hash_rem(g_msgid2reply, bpctrl.szMsgId);//remove mapping 
			reply_destroy(&sock_reply);
		} else {
			sock_reply = reply_new(NULL, NULL, reply); 
			hash_put(g_msgid2reply, bpctrl.szMsgId, sock_reply);
		}
		zmutex_unlock(g_mutex_hash);

		if(sock_id){ 
			assert(msg_id);
			if(g_kcbp_cfg->debug){
				char* sock_id_str = zframe_strhex(sock_id);
				zlog("[REP] KCXP(%s)=>ZBUS(%s#%s)\n",bpctrl.szMsgId, broker, sock_id_str);
				zfree(sock_id_str);
			} 
			int rc = s_client_reply(zbus_conn, sock_id, msg_id, reply);
			if(rc != 0){
				zlog("ERROR: reply back error encountered");
			}   
		} 
	}
	zfree(broker);
	zbusconn_destroy(&zbus_conn);
	return NULL;
}

static void
s_log_kcxp_request(zmsg_t* msg, char* funcid){
	assert(msg);
	FILE* file = zlog_get_log_file();
	zmsg_t* request = zmsg_dup(msg);
	zlog("KCXP 请求(功能号: %s)\n", funcid);
	while(1){
		zframe_t* key_frame = zmsg_pop_front(request);
		if(!key_frame) break;
		zframe_t* val_frame = zmsg_pop_front(request);
		assert(val_frame); //should exist

		char* key = zframe_strdup(key_frame); 

		//KCBP limit!!!, special for binary key-value
		if(strchr(key,'^') == key && strrchr(key,'^') == (key+strlen(key)-1)){
			char bin_key[1024]; 
			strncpy(bin_key, key+1, strlen(key)-2);
			bin_key[strlen(key)-2] = '\0';
			fprintf(file,"[%s={二进制块}] ", bin_key);    
		} else {
			char* val = zframe_strdup(val_frame); 
			fprintf(file,"[%s=%s] ", key, val);  
			zfree(val);
		}   

		zframe_destroy(&key_frame);
		zframe_destroy(&val_frame);
		zfree(key); 
	}
	fprintf(file, "\n\n");
	zmsg_destroy(&request);
}

void* 
thread_recv(void* args){
	assert(args);
	assert(g_kcbp_cfg);   
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
	conn_cfg.verbose = g_kcbp_cfg->debug; 
	strcpy(conn_cfg.host, host);
	conn_cfg.port = port; 
	
	zbusconn_t* zbus_conn = zbusconn_new(&conn_cfg);  


	zbuswrk_cfg_t wrk_cfg;
	memset(&wrk_cfg, 0, sizeof(wrk_cfg));
	strcpy(wrk_cfg.service, g_kcbp_cfg->service_name);
	strcpy(wrk_cfg.reg_token, g_kcbp_cfg->service_regtoken);
	strcpy(wrk_cfg.acc_token, g_kcbp_cfg->service_acctoken);
	strcpy(wrk_cfg.mode, MODE_LB);

	zbuswrk_t* worker = zbuswrk_new(&wrk_cfg);
	assert(worker);


	void* kcbp = kcxpcli_new(g_kcbp_cfg); 
	char error_msg[1024] = {0}; 
	tagCallCtrl bpctrl; 
	int rc; 

	while(1){  
		while(!kcbp){ //assume kcbp ok, if not reconnect
			zlog("KCXP(%s:%d) down, reconnecting...\n", g_kcbp_cfg->kcxp_host,g_kcbp_cfg->kcxp_port);
			zclock_sleep(g_kcbp_cfg->kcxp_reconnect);
			kcbp = kcxpcli_new(g_kcbp_cfg);
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

		if(g_kcbp_cfg->verbose){ 
			s_log_kcxp_request(request, funcid);
		}

		KCBPCLI_BeginWrite(kcbp);
		while(1){
			zframe_t* key_frame = zmsg_pop_front(request);
			if(!key_frame) break;
			zframe_t* val_frame = zmsg_pop_front(request);
			assert(val_frame); //should exist

			char* key = zframe_strdup(key_frame); 

			//KCBP limit!!!, special for binary key-value
			if(strchr(key,'^') == key && strrchr(key,'^') == (key+strlen(key)-1)){
				char bin_key[1024]; 
				strncpy(bin_key, key+1, strlen(key)-2);
				bin_key[strlen(key)-2] = '\0';

				unsigned char* val = (unsigned char*)zframe_data(val_frame);   
				KCBPCLI_SetVal(kcbp, bin_key, val, zframe_size(val_frame));  
			} else {
				char* val = zframe_strdup(val_frame);
				KCBPCLI_SetValue(kcbp, key, val); // 0 for non-binary, 
				zfree(val);
			}   

			zframe_destroy(&key_frame);
			zframe_destroy(&val_frame);
			zfree(key); 
		}
		
		memset(&bpctrl, 0, sizeof(bpctrl));
		bpctrl.nExpiry = 10;

		if(g_kcbp_cfg->debug)
			zlog("KCXP AsynCall BEGIN....\n");

		rc = KCBPCLI_ACallProgramAndCommit(kcbp, funcid, &bpctrl);  
		
		if(g_kcbp_cfg->debug)
			zlog("KCXP AsynCall END (%d)\n", rc);


		if(rc == 0){  
			zmsg_t* reply = NULL;
			
			if(g_kcbp_cfg->debug){
				char* sock_id_str = zframe_strhex(sock_id);
				zlog("[REQ] ZBUS(%s#%s)=>KCXP(%s)\n", broker, sock_id_str, bpctrl.szMsgId);
				zfree(sock_id_str);
			}

			zmutex_lock(g_mutex_hash); 
			reply_t* sock_reply = (reply_t*)hash_get(g_msgid2reply, bpctrl.szMsgId);
			if(sock_reply){ //reply may already returned
				assert(sock_reply->sock_id == NULL);
				assert(sock_reply->msg_id == NULL);
				assert(sock_reply->reply); 

				reply = sock_reply->reply;
				hash_rem(g_msgid2reply, bpctrl.szMsgId);//remove mapping 
				reply_destroy(&sock_reply);
				
			} else {
                sock_reply = reply_new(sock_id, msg_id, NULL);
				hash_put(g_msgid2reply,bpctrl.szMsgId, sock_reply);
			} 
			zmutex_unlock(g_mutex_hash);

			if(reply){
				int rc = s_client_reply(zbus_conn, sock_id, msg_id, reply);
				if(rc != 0){
					zlog("ERROR: reply back error encountered");
				}   
			}
		} else {
			KCBPCLI_GetErrorMsg(kcbp, error_msg);
			zlog("WARN: KCXP failed(code=%d,msg=%s)\n", rc, error_msg);
			if(rc == 2003 || rc == 2004 || rc == 2054 || rc == 2055){ 
				//require reconnect
				kcxpcli_destroy(&kcbp); 
			} 

			zframe_destroy(&sock_id);
			zframe_destroy(&msg_id);
		} 

		zmsg_destroy(&request);  
		zframe_destroy(&funcid_frame); 
		zfree(funcid); 
	}   

	zfree(broker);
	zbusconn_destroy(&zbus_conn); 
	kcxpcli_destroy(&kcbp);

	return NULL;
} 

 
int main(int argc, char* argv[]){ 
	if(argc>1 && ( strcmp(argv[1],"help")==0 
		|| strcmp(argv[1],"--help")==0 )){
			printf("-v\tverbose mode to show message, e.g. -v0, -v1\n");
			printf("-x\tkcxp server name, e.g. -xzbus_kcxp\n");
			printf("-h\tkcxp host e.g. -h172.24.180.134 \n");
			printf("-p\tkcxp port e.g. -p21000 \n");
			printf("-qs\tkcxp sendQ name e.g. -qsreq_zb \n");
			printf("-qr\tkcxp recvQ name e.g. -qrans_zb \n");
			printf("-u\tkcxp auth user e.g. -uKCXP00 \n");
			printf("-P\tkcxp auth user e.g. -P888888 \n");
			printf("-c\tzbus-kcxp worker count, concurrency e.g. -c2 \n");
			printf("-s\tservice name, concurrency e.g. -skcxp \n");
			printf("-kreg\tservice register token, e.g. -kregzbus \n");
			printf("-kacc\tservice access token, e.g. -kacczbus \n");
			printf("-b\tzbus broker, e.g. -b172.24.180.27:15555 \n");
			printf("-log\tlog path,  e.g. -log. \n");
			return 0;
	}   

	g_kcbp_cfg = kcxp_cfg_new(argc,argv); 
	assert(g_kcbp_cfg);

	if(g_kcbp_cfg->log_path){
		zlog_use_file(g_kcbp_cfg->log_path); 
	} else {
		zlog_use_stdout();
	}  
	g_zmq_context = zctx_new(1);
	g_mutex_hash = zmutex_new(); 
	g_msgid2reply = hash_new(&g_hctrl_msgid2reply, NULL);
	

	char* broker = zstrdup(g_kcbp_cfg->broker);
	
	int thread_count = g_kcbp_cfg->worker_threads;
	zthread_t* reply_threads = (zthread_t*)zmalloc(thread_count*sizeof(zthread_t));
	zthread_t* recv_threads = (zthread_t*)zmalloc(thread_count*sizeof(zthread_t));

	for(int i=0; i<g_kcbp_cfg->worker_threads; i++){
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
	kcxp_cfg_destroy(&g_kcbp_cfg); 
	zctx_destroy(&g_zmq_context);

	return 0;
}