#include "zbus/include/prelude.h"
#include "zbus/include/zbusapi.h"
#include "zbus/include/hash.h"  
#include "apex/FixApi.h"
#include "apex/fiddef.h"
#include "apex/blsdef_3rd.h"
#include "json/cJSON.h"

#define VALUE_MAX_LEN 10240

typedef struct _apex_cfg_t{ 
	char*	apex_address; 
	char*	apex_user;
	char*	apex_pwd; 
	int		apex_timeout; 
	int		apex_reconnect;

	int		verbose;
	int		debug;
	char*	broker; 
	char*	service_name;
	char*	service_regtoken;
	char*	service_acctoken;
	int		worker_threads;

	int		probe_interval;

	char*   log_path;

} apex_cfg_t;

apex_cfg_t*
apex_cfg_new(int argc, char* argv[]){
	apex_cfg_t* self = (apex_cfg_t*)zmalloc(sizeof(apex_cfg_t));
	assert(self);
	memset(self, 0, sizeof(apex_cfg_t)); 
	self->apex_address = zstrdup(option(argc,argv, "-a", "10.27.49.13@9101/tcp"));
	//self->apex_address = zstrdup(option(argc,argv, "-a", "127.0.0.1@9101/tcp"));
	self->apex_user = zstrdup(option(argc,argv, "-u", ""));
	self->apex_pwd = zstrdup(option(argc,argv, "-p", ""));
	self->apex_timeout = atoi(option(argc,argv,"-t","3"));
	self->apex_reconnect = atoi(option(argc,argv,"-r","1000"));

	self->verbose = atoi(option(argc,argv, "-v", "1"));
	self->debug = atoi(option(argc,argv, "-dbg", "0"));
	self->worker_threads = atoi(option(argc,argv, "-c", "2"));
	self->broker = zstrdup(option(argc,argv, "-b", "localhost:15555"));
	self->service_name = zstrdup(option(argc,argv, "-s", "APEX"));
	self->service_regtoken = zstrdup(option(argc,argv, "-kreg", ""));
	self->service_acctoken = zstrdup(option(argc,argv, "-kacc", ""));
	self->probe_interval = atoi(option(argc,argv, "-t", "6000"));

	self->log_path = zstrdup(option(argc,argv, "-log", NULL));
	return self;
}
void
apex_cfg_destroy(apex_cfg_t** self_p){
	if(!self_p) return;
	apex_cfg_t* self = *self_p;
	if(self){
		if(self->apex_address) zfree(self->apex_address);
		if(self->apex_user) zfree(self->apex_user);
		if(self->apex_pwd) zfree(self->apex_pwd);
		if(self->service_name) zfree(self->service_name);
		if(self->broker) zfree(self->broker);
		//if(self->service_name) zfree(self->service_name);
		if(self->service_regtoken) zfree(self->service_regtoken);
		if(self->service_acctoken) zfree(self->service_acctoken);
		if(self->log_path) zfree(self->log_path);

		zfree(self);
		*self_p = NULL;
	}
}


typedef struct _reply_t{
	zframe_t* sock_id;
	zframe_t* msg_id;
	zmsg_t* reply;
	int64_t ts;
}reply_t;

//////////////////////////////////////////////////////////////////////////
//global 
zmutex_t*		g_mutex_hash; 
apex_cfg_t*		g_apex_cfg;   
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

static int
s_client_reply(zbusconn_t* conn, zframe_t* sock_id, zframe_t* msg_id, zmsg_t* reply){
	zmsg_push_front(reply, msg_id);
	zmsg_push_front(reply, zframe_newstr(MDPC)); 
	int rc = zbusconn_route(conn, sock_id, reply);
	return rc;
}
 

void* 
thread_recv(void* args){
	assert(args);
	assert(g_apex_cfg);   
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
	conn_cfg.verbose = g_apex_cfg->debug; 
	strcpy(conn_cfg.host, host);
	conn_cfg.port = port; 

	zbusconn_t* zbus_conn = zbusconn_new(&conn_cfg);   

	zbuswrk_cfg_t wrk_cfg;
	memset(&wrk_cfg, 0, sizeof(wrk_cfg));
	strcpy(wrk_cfg.service, g_apex_cfg->service_name);
	strcpy(wrk_cfg.reg_token, g_apex_cfg->service_regtoken);
	strcpy(wrk_cfg.acc_token, g_apex_cfg->service_acctoken);
	strcpy(wrk_cfg.mode, MODE_LB);

	zbuswrk_t* worker = zbuswrk_new(&wrk_cfg);
	assert(worker);

	int rc; 
	char value_data[VALUE_MAX_LEN];
	char msg_data[VALUE_MAX_LEN];

	
	HANDLE_CONN apex_conn = Fix_Connect(g_apex_cfg->apex_address,
		g_apex_cfg->apex_user,g_apex_cfg->apex_pwd, g_apex_cfg->apex_timeout);
	if(apex_conn){
		zlog("APEX Connected\n");
	}
	while(1){   
		while(!apex_conn){
			zlog("APEX(%s) down, reconnecting...\n", g_apex_cfg->apex_address);
			apex_conn = Fix_Connect(g_apex_cfg->apex_address,
				g_apex_cfg->apex_user,g_apex_cfg->apex_pwd,g_apex_cfg->apex_timeout);
			if(apex_conn){
				zlog("APEX Connected\n");
			}
			zclock_sleep(g_apex_cfg->apex_reconnect);
		}
		zmsg_t* request = zbuswrk_recv(zbus_conn, worker);
		if(!request) break;  

		zframe_t* sock_id, *msg_id;
		zbuswrk_get_address(worker, &sock_id, &msg_id);

		if(zmsg_frame_size(request) != 2){
			zmsg_destroy(&request); 
			zmsg_t* reply = zmsg_new(); 
			zmsg_push_back(reply, zframe_newstr("400"));
			zmsg_push_back(reply, zframe_newstr("request format: <funcid> <json_params>"));

			s_client_reply(zbus_conn, sock_id, msg_id, reply);
			continue;
		}

		zframe_t* funcid_frame = zmsg_pop_front(request);
		char* funcid_str = zframe_strdup(funcid_frame);
		long funcid = atol(funcid_str);

		zframe_t* params_frame = zmsg_pop_front(request); 
		char* params_json = zframe_strdup(params_frame);  
		
		HANDLE_SESSION sess = Fix_AllocateSession(apex_conn);
		Fix_SetGYDM(sess, "99999998"); 
		Fix_SetFBDM(sess, "9999");
		Fix_CreateReq(sess, funcid);
		
		cJSON* json = cJSON_Parse(params_json);  
		if(json){
			cJSON* param = json->child;
			while(param && param->string){
				long key = atol(param->string);
				char* val = param->valuestring; 
				if(!val) val = "";
				Fix_SetString(sess, key, val);
				/* 
				char* val_utf8 = utf8_to_gbk(val);
				Fix_SetString(sess, key, val_utf8);
				delete[] val_utf8;
				*/
				param = param->next;
			}
			cJSON_Delete(json);
		} else {
			zframe_destroy(&funcid_frame); 
			zfree(funcid_str); 
			zframe_destroy(&params_frame);
			zfree(params_json);
			zmsg_destroy(&request);
			Fix_ReleaseSession(sess);
			continue;
		}

		rc = Fix_Run(sess);     
		if(rc){  
			zmsg_t* reply = zmsg_new(); 
			zmsg_push_back(reply, zframe_newstr("200"));

			cJSON* root = cJSON_CreateArray();
			int rows = Fix_GetCount(sess);
			for(int i=0; i<rows; i++){
				int cols = Fix_GetColumnCount(sess, i);
				cJSON* row = cJSON_CreateObject();
				for(int j=0; j<cols; j++){
					long key = 0; 
					int size = VALUE_MAX_LEN; 
					Fix_GetValWithIdByIndex(sess, i, j, key, value_data, size);
					char key_str[32];
					sprintf(key_str, "%d", key);
					value_data[size] = '\0';
					cJSON_AddStringToObject(row, key_str, value_data); 
					/*
					char* value_data_utf8 = gbk_to_utf8(value_data);
					cJSON_AddStringToObject(row, key_str, value_data_utf8); 
					delete[] value_data_utf8;
					*/
				}
				cJSON_AddItemToArray(root, row);
			} 
			if(rows == 0){
				Fix_GetItem(sess, FID_MESSAGE, msg_data, sizeof(msg_data));
				root->valuestring = msg_data;
			}
			char* json_data = cJSON_Print(root); 
			zmsg_push_back(reply, zframe_newstr(json_data));
			free(json_data);
			s_client_reply(zbus_conn, sock_id, msg_id, reply);
		} else {
			long code = Fix_GetCode(sess);
			Fix_GetErrMsg(sess, msg_data, sizeof(msg_data));
			zlog("WARN: KCXP failed(code=%d,msg=%s)\n", code, msg_data);

			zmsg_t* reply = zmsg_new(); 
			zmsg_push_back(reply, zframe_newstr("500"));
			zmsg_push_back(reply, zframe_newstr(msg_data));

			s_client_reply(zbus_conn, sock_id, msg_id, reply);

			zframe_destroy(&sock_id);
			zframe_destroy(&msg_id);
		} 
		Fix_ReleaseSession(sess);


		zframe_destroy(&funcid_frame); 
		zfree(funcid_str); 
		zframe_destroy(&params_frame);
		zfree(params_json);
		zmsg_destroy(&request);  
		
	}   

	zfree(broker);
	zbusconn_destroy(&zbus_conn); 
	Fix_Close(apex_conn);

	return NULL;
} 

int main(int argc, char* argv[]){  
	Fix_Initialize(); 

	g_apex_cfg = apex_cfg_new(argc,argv); 
	assert(g_apex_cfg);

	if(g_apex_cfg->log_path){
		zlog_use_file(g_apex_cfg->log_path); 
	} else {
		zlog_use_stdout();
	}  
	g_zmq_context = zctx_new(1);
	g_mutex_hash = zmutex_new(); 
	g_msgid2reply = hash_new(&g_hctrl_msgid2reply, NULL);


	char* broker = zstrdup(g_apex_cfg->broker);

	int thread_count = g_apex_cfg->worker_threads;
	zthread_t* reply_threads = (zthread_t*)zmalloc(thread_count*sizeof(zthread_t));
	zthread_t* recv_threads = (zthread_t*)zmalloc(thread_count*sizeof(zthread_t));

	for(int i=0; i<g_apex_cfg->worker_threads; i++){
		//reply_threads[i] = zthread_new(thread_reply, zstrdup(broker));
		recv_threads[i] = zthread_new(thread_recv, zstrdup(broker));
	}  
	zfree(broker); 

	for(int i=0; i<thread_count; i++){
		//zthread_join(reply_threads[i]);
		zthread_join(recv_threads[i]);
	} 

	zmutex_destroy(&g_mutex_hash);  
	hash_destroy(&g_msgid2reply);
	apex_cfg_destroy(&g_apex_cfg); 
	zctx_destroy(&g_zmq_context);	
	
	
	Fix_Uninitialize();  
	return 0;
}