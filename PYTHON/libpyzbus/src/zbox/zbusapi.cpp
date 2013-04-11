#include "include/prelude.h" 
#include "include/zbusapi.h"
#include "include/hash.h"

#include "../include/zmq.h"

hash_ctrl_t hash_ctrl_string_topic = {
	hash_func_string,            /* hash function */
	hash_dup_string,             /* key dup */
	hash_dup_string,             /* val dup */
	hash_cmp_string,             /* key compare */
	hash_destroy_string,         /* key destructor */
	hash_destroy_string,         /* val destructor */
};

struct _zbusconn_t{
	void*	ctx;
	int		own_ctx;
	void*	socket;
	char*	host;
	int		port;
	int		verbose;
	char*	id;
};

struct _zbuswrk_t{
	char*		service;	
	char*		mode;		
	char*		reg_token;	
	char*		acc_token;

	zframe_t*	recv_sock_id;
	zframe_t*	recv_msg_id;

	int			_registered;
	int			_liveness;
	int64_t		_heartbeat_at;

	hash_t*		topics;
};

 
int
zbusconn_reconnect(zbusconn_t* self){
	assert(self);
	assert(self->ctx);
	int rc;
	if(self->socket){ 
		rc = zmq_close(self->socket);
		if(self->verbose){
			if(rc == 0)
				zlog("[INFO]: ZBUS(%s:%d) Disconnected \n", self->host, self->port);
			else
				zlog("[WARN]: ZBUS(%s:%d) Disconnect failed\n", self->host, self->port);
		} 
	}
	self->socket = zmq_socket(self->ctx, ZMQ_DEALER);
	assert(self->socket);
	int value = 0; //
	rc = zmq_setsockopt(self->socket, ZMQ_LINGER, &value, sizeof(value));
	assert( rc == 0);

	if(strcmp(self->id, "") != 0){ 
		rc = zmq_setsockopt(self->socket, ZMQ_IDENTITY, self->id, strlen(self->id));
		assert( rc == 0);
	}
	char broker[128];
	sprintf(broker, "tcp://%s:%d", self->host, self->port);
	if(self->verbose){
		zlog("[INFO]: ZBUS(%s:%d) Connecting... \n", self->host, self->port);
	}
	rc = zmq_connect(self->socket, broker);
	if(self->verbose){
		if(rc == 0)
			zlog("[INFO]: ZBUS(%s:%d) Connected \n", self->host, self->port);
		else
			zlog("[WARN]: ZBUS(%s:%d) Connect failed\n", self->host, self->port);
	} 
	return rc;
}

zbusconn_t*
zbusconn_new(zbusconn_cfg_t* cfg){
	assert(cfg);
	zbusconn_t* self = (zbusconn_t*)zmalloc(sizeof(zbusconn_t));
	assert(self);
	memset(self, 0, sizeof(zbusconn_t));
	if(cfg->ctx){
		self->ctx = cfg->ctx;
		self->own_ctx = 0;
	} else {
		self->ctx = zctx_new(1);
		self->own_ctx = 1;
	}
	self->host = zstrdup(cfg->host);
	self->id = zstrdup(cfg->id);
	self->port = cfg->port;
	self->verbose = cfg->verbose;
	
	int rc = zbusconn_reconnect(self);
	if(rc != 0) //destroy if not connected
		zbusconn_destroy(&self);
	
	return self;
} 
void
zbusconn_destroy(zbusconn_t** self_p){
	if(!self_p) return;
	zbusconn_t* self = *self_p;
	if(self){
		if(self->socket){
			int rc = zmq_close(self->socket);
			assert(rc == 0);
		}
		if(self->id){
			zfree(self->id);
		}
		if(self->host){
			zfree(self->host);
		}
		if(self->own_ctx && self->ctx){
			zctx_destroy(&self->ctx);
		}
		zfree(self);
		*self_p = NULL;
	}
}

int	
zbusconn_send(zbusconn_t* self, zmsg_t* msg){
	assert(self);
	assert(msg);   
	zmsg_push_front(msg, zframe_new(NULL, 0));
	if(self->verbose){ 
		zmsg_log(msg, "[INFO]: send out");
	} 
	int rc = zmsg_send(&msg, self->socket);
	return rc; 
}

zmsg_t*
zbusconn_recv(zbusconn_t* self, int timeout){
	int rc = zmq_setsockopt(self->socket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
	assert(rc == 0); 
	zmsg_t* msg = zmsg_recv(self->socket);
	if(msg){
		if(self->verbose)
			zmsg_log(msg, "[INFO]: recv in");

		zframe_t* empty = zmsg_pop_front(msg);
		assert(empty && zframe_size(empty)==0);
		zframe_destroy(&empty);
	} 
	return msg;
}

int	
zbusconn_route(zbusconn_t* self, zframe_t* sock_id, zmsg_t* msg){
	assert(self);
	assert(msg); 
	zmsg_push_front(msg, zframe_new(NULL, 0));
	zmsg_push_front(msg, sock_id);
	zmsg_push_front(msg, zframe_newstr(MDPX)); 
	return zbusconn_send(self, msg);  
}

int
zbusconn_probe(zbusconn_t* self, int timeout){
	assert(self);  
	zmsg_t* msg = zmsg_new();

	zmsg_push_front(msg, zframe_newstr(MDPT)); 

	int rc = zbusconn_send(self, msg);
	if(rc != 0){
		zlog("[WARN]: send MDPT message error\n");
		return -1;
	}
	msg = zbusconn_recv(self, timeout);
	if(!msg) return -2;

	zmsg_destroy(&msg); 
	return 0;//probe successful
}

zmsg_t*
zbuscli_request(zbusconn_t* self, char* service, char* token, zmsg_t* msg, int timeout){
	assert(self); 
	assert(service);
	assert(msg);

	if(!token) token = "";

	zmsg_push_front(msg, zframe_new(NULL, 0)); //msg_id<empty>, default to empty for synchronise mode
	zmsg_push_front(msg, zframe_newstr(token));
	zmsg_push_front(msg, zframe_newstr(service));
	zmsg_push_front(msg, zframe_newstr(MDPC)); //message queue mode


	int rc = zbusconn_send(self, msg);
	if(rc != 0){
		zlog("[WARN]: send MDPC message error\n");
		zbusconn_reconnect(self);  
		return NULL;
	}

	zmsg_t* msg_rep = zbusconn_recv(self, timeout);
	if(msg_rep){ 
		if(zmsg_frame_size(msg_rep)<2){
			zmsg_log(msg_rep, "[<header>, <msg_id>] frames required\n");
		}  
		zframe_t* header = zmsg_pop_front(msg_rep);
		zframe_destroy(&header);
		zframe_t* msg_id = zmsg_pop_front(msg_rep);
		zframe_destroy(&msg_id);

		return msg_rep;
	} else {
		if(self->verbose){
			zlog("zbuscli_request recv null, connection reset\n");
		}
		zbusconn_reconnect(self); 
	}    
	return NULL;
}


zmsg_t*
zbuscli_send(zbusconn_t* self, asyn_ctrl_t* ctrl, zmsg_t* msg){
	assert(self); 
	assert(ctrl);
	assert(msg);
	
	zmsg_push_front(msg, zframe_newstr(ctrl->msg_id));
	zmsg_push_front(msg, zframe_newstr(ctrl->peer_id));
	zmsg_push_front(msg, zframe_newstr(ctrl->token));
	zmsg_push_front(msg, zframe_newstr(ctrl->service));
	zmsg_push_front(msg, zframe_newstr(MDPQ)); //message queue mode

	int rc = zbusconn_send(self, msg);
	if(rc != 0){
		zlog("[WARN]: send MDPQ message error\n");
		return NULL;
	}
	
	msg = zbusconn_recv(self, ctrl->timeout);
	if(msg){ 
		if(zmsg_frame_size(msg)<2){
			zmsg_log(msg, "[<header>, <msg_id>] frames required\n");
		}  
		zframe_t* header = zmsg_pop_front(msg);
		zframe_destroy(&header);
		zframe_t* msg_id = zmsg_pop_front(msg);
		zframe_destroy(&msg_id);
	} else {
		if(self->verbose){
			zlog("zbuscli_send recv null, connection reset\n");
		}
		zbusconn_reconnect(self); 
	}
	return msg; 
}

zmsg_t*
zbuscli_recv(zbusconn_t* self, int probe_interval){
	assert(self); 
	zmsg_t* msg; 
	if(probe_interval < 1000){ //probe should not be too frequent
		probe_interval = 1000;
	}
	while(1){
		 msg = zbusconn_recv(self, probe_interval);
		 if(!msg){
			int probe_rc = zbusconn_probe(self, probe_interval);
			if(probe_rc != 0){
				zbusconn_reconnect(self);
			}
		 } else {
			 zframe_t* header = zmsg_pop_front(msg);
			 if(header && zframe_streq(header, MDPC)){
				 zframe_destroy(&header);
				 return msg;
			 } 
			 //received ping back message, discard
			 zframe_destroy(&header);
			 zmsg_destroy(&msg);
		 }
	}  
	return NULL; 
}

zmsg_t*
zbusmon_request(zbusconn_t* self, char* token, zmsg_t* msg, int timeout){
	assert(self); 
	assert(msg);
	if(!token) token = ""; 
	zmsg_push_front(msg, zframe_newstr(token)); 
	zmsg_push_front(msg, zframe_newstr(MDPM));
	int rc = zbusconn_send(self, msg);
	assert(rc == 0);

	zmsg_t* res = zbusconn_recv(self, timeout);

	if(res){
		zframe_t* header = zmsg_pop_front(res); 
		zframe_destroy(&header);
		zframe_t* msg_id = zmsg_pop_front(res);
		zframe_destroy(&msg_id);
	} 
	return res; 
}
 

static int
s_zbuswrk_command(zbusconn_t* self, char* command, zmsg_t* args){
	assert(self);
	if(args == NULL){
		args = zmsg_new();
	}
	zmsg_push_front(args, zframe_newstr(command));
	zmsg_push_front(args, zframe_newstr(MDPW));
	return zbusconn_send(self, args);
}

static int
s_zbuswrk_register(zbusconn_t* self, zbuswrk_t* worker){
	assert(self);
	assert(worker);
	worker->_registered = 1;
	zmsg_t* args = zmsg_new();  
	zmsg_push_back(args, zframe_newstr(worker->service));
	zmsg_push_back(args, zframe_newstr(worker->reg_token));
	zmsg_push_back(args, zframe_newstr(worker->acc_token));
	zmsg_push_back(args, zframe_newstr(worker->mode)); 

	int rc = s_zbuswrk_command(self, MDPW_REG, args); 
	
	if(worker->topics){ //subscribe topics
		zmsg_t* params = zmsg_new();
		hash_iter_t* it = hash_iter_new(worker->topics);
		hash_entry_t* he = hash_iter_next(it);
		while(he){
			char* topic = (char*)hash_entry_val(he);
			zmsg_push_back(params, zframe_newstr(topic));
			he = hash_iter_next(it);
		}
		hash_iter_destroy(&it); 
		rc = s_zbuswrk_command(self, MDPW_SUB, params);
	} 
	return rc;
}

int
zbuswrk_send(zbusconn_t* self, zbuswrk_t* worker, zmsg_t* msg){
	assert(self);
	assert(worker);
	assert(msg); 
	assert(worker->recv_msg_id);
	assert(worker->recv_sock_id);

	zframe_t* msg_id = zframe_dup(worker->recv_msg_id);
	zframe_t* sock_id = zframe_dup(worker->recv_sock_id); 

	zmsg_push_front(msg, msg_id);
	zmsg_push_front(msg, zframe_newstr(MDPC));  
	return zbusconn_route(self, sock_id, msg);
}

int
zbuswrk_sendto(zbusconn_t* self, zframe_t* client_sock_id, zframe_t* msg_id, zmsg_t* msg){
	assert(self); 
	assert(client_sock_id); 
	assert(msg_id);
	assert(msg);  
	
	zmsg_push_front(msg, msg_id);
	zmsg_push_front(msg, zframe_newstr(MDPC));  
	return zbusconn_route(self, client_sock_id, msg);
}

int
zbuswrk_subscribe(zbusconn_t* self, zbuswrk_t* worker, char* topic){
	assert(self);
	assert(worker);
	if(!worker->_registered){
		s_zbuswrk_register(self, worker);
	}
	if(!worker->topics){
		worker->topics = hash_new(&hash_ctrl_string_topic, NULL);
	}
	hash_put(worker->topics, topic, topic);
	zmsg_t* params = zmsg_new();
	zmsg_push_back(params, zframe_newstr(topic));
	int rc = s_zbuswrk_command(self, MDPW_SUB, params);
	return rc;
}

int
zbuswrk_unsubscribe(zbusconn_t* self, zbuswrk_t* worker, char* topic){
	assert(self);
	assert(worker);
	if(!worker->_registered){
		s_zbuswrk_register(self, worker);
	}
	if(worker->topics){
		hash_rem(worker->topics, topic);
	}
	zmsg_t* params = zmsg_new();
	zmsg_push_back(params, zframe_newstr(topic));
	int rc = s_zbuswrk_command(self, MDPW_UNSUB, params);
	return rc;
}


zmsg_t*
zbuswrk_recv(zbusconn_t* self, zbuswrk_t* worker){
	assert(self);
	assert(worker);
	if(!worker->_registered){
		s_zbuswrk_register(self, worker);
	}
	while(1){
		if(worker->_liveness >= 0){ 
			if( zclock_time() > worker->_heartbeat_at){ //send heartbeat
				s_zbuswrk_command(self, MDPW_HBT, NULL);
				worker->_heartbeat_at = zclock_time() + HEARTBEAT_INTERVAL;
			} 
		}

		zmsg_t* msg = zbusconn_recv(self, HEARTBEAT_INTERVAL); 
		
		if(!msg){ 
			worker->_liveness--;
			if(worker->_liveness <= 0){
				zlog("[WARN]: zbus is down, trying register again...\n"); 
				zbusconn_reconnect(self);
				s_zbuswrk_register(self, worker);//register
				zclock_sleep(HEARTBEAT_INTERVAL);
			}
			continue;
		} 
		//got message from zbus, zbus is alive
		if(worker->_liveness<=0){//first time change from dead to alive
			zlog("[INFO]: zbus returned\n");
		} 
		worker->_liveness = HEARTBEAT_LIVENESS;
		
		if(zmsg_frame_size(msg)<2){ 
			zlog("[ERROR]: [<header>, <cmd>] frames required\n");
			zmsg_destroy(&msg); 
			return NULL;
		}

		zframe_t* header = zmsg_pop_front(msg);
		assert(zframe_streq(header, MDPW));
		zframe_destroy(&header);

		zframe_t* cmd = zmsg_pop_front(msg); 
		if(zframe_streq(cmd, MDPW_REQ)){ 
			if(worker->recv_sock_id){ //destroy old one
				zframe_destroy(&worker->recv_sock_id);
			} 
			if(worker->recv_msg_id){ //destroy old one
				zframe_destroy(&worker->recv_msg_id);
			} 
			worker->recv_sock_id = zmsg_unwrap(msg);   //new one
			worker->recv_msg_id = zmsg_pop_front(msg); //new one

			s_zbuswrk_command(self, MDPW_IDLE, NULL);
			zframe_destroy(&cmd); 
			return msg;
		} else if(zframe_streq(cmd, MDPW_HBT)){ 
			//just do nothing for heartbeat
		} else if(zframe_streq(cmd, MDPW_DISC)){
			zlog("[WARN]: received disconnection message from zbus");
			zframe_destroy(&cmd);
			zmsg_destroy(&msg);
			break;
		} else if(zframe_streq(cmd, MDPW_SYNC)){
			zlog("[INFO]: synchronized by zbus\n"); 
			s_zbuswrk_register(self, worker);
		} else { 
			zlog("[ERROR]: invalid command");
		}

		zframe_destroy(&cmd);
		zmsg_destroy(&msg); 
	}
	return NULL;
}
  
 

zbuswrk_t*
zbuswrk_new(zbuswrk_cfg_t* cfg){
	assert(cfg);
	zbuswrk_t* self = (zbuswrk_t*)zmalloc(sizeof(zbuswrk_t));
	memset(self, 0, sizeof(zbuswrk_t));
	self->service = zstrdup(cfg->service); 
	self->mode = zstrdup(cfg->mode);
	self->reg_token = zstrdup(cfg->reg_token);
	self->acc_token = zstrdup(cfg->acc_token);
	
	self->_liveness = HEARTBEAT_LIVENESS; 
	self->_registered = 0;
	self->_heartbeat_at = 0; 
	return self;
}

void
zbuswrk_destroy(zbuswrk_t** self_p){
	assert(self_p);
	zbuswrk_t* self = *self_p;
	if(self){
		if(self->service) 
			zfree(self->service);
		if(self->mode)
			zfree(self->mode);
		if(self->reg_token)
			zfree(self->reg_token);
		if(self->acc_token)
			zfree(self->acc_token);
		if(self->recv_sock_id)
			zframe_destroy(&self->recv_sock_id);
		if(self->recv_msg_id)
			zframe_destroy(&self->recv_msg_id);
		if(self->topics)
			hash_destroy(&self->topics);

		zfree(self);
		*self_p = NULL;
	}
}

void
zbuswrk_get_address(zbuswrk_t* worker, zframe_t** sock_id, zframe_t** msg_id){
	assert(worker);
	assert(sock_id);
	assert(msg_id);
	
	*sock_id = zframe_dup(worker->recv_sock_id);
	*msg_id = zframe_dup(worker->recv_msg_id); 
}

void
zbuswrk_set_address(zbuswrk_t* worker, zframe_t* sock_id, zframe_t* msg_id){
	assert(worker); 
	if(worker->recv_sock_id){
		zframe_destroy(&worker->recv_sock_id);
	}
	if(worker->recv_msg_id){
		zframe_destroy(&worker->recv_msg_id);
	}
	worker->recv_sock_id = sock_id;
	worker->recv_msg_id = msg_id;
}


zbusconn_cfg_t*
zbusconn_cfg_new(char* host, int port, int verbose, void* ctx, char* id){
	zbusconn_cfg_t* self = (zbusconn_cfg_t*)zmalloc(sizeof(zbusconn_cfg_t));
	memset(self, 0, sizeof(zbusconn_cfg_t));
	if(host)
		strcpy(self->host, host);
	self->port = port;
	self->verbose = verbose;
	self->ctx = ctx;
	if(id)
		strcpy(self->id, id);
	return self;
}
zbuswrk_cfg_t*
zbuswrk_cfg_new(char* service, char* mode, char* reg_token, char* acc_token){
	zbuswrk_cfg_t* self = (zbuswrk_cfg_t*)zmalloc(sizeof(zbuswrk_cfg_t));
	memset(self, 0, sizeof(zbuswrk_cfg_t));
	if(service)
		strcpy(self->service, service);
	if(mode)
		strcpy(self->mode, mode);
	if(reg_token)
		strcpy(self->reg_token, reg_token);
	if(acc_token)
		strcpy(self->acc_token, acc_token);
	return self;
}
asyn_ctrl_t*
asyn_ctrl_new(char* service, char* token, int timeout, char* peer_id, char* msg_id){
	asyn_ctrl_t* self = (asyn_ctrl_t*)zmalloc(sizeof(asyn_ctrl_t));
	memset(self, 0, sizeof(asyn_ctrl_t));
	if(service)
		strcpy(self->service, service);
	if(token)
		strcpy(self->token, token);

	self->timeout = timeout;

	if(peer_id)
		strcpy(self->peer_id, peer_id);
	if(msg_id)
		strcpy(self->msg_id, msg_id);
	return self;
}
void
zbusconn_cfg_destroy(zbusconn_cfg_t** self_p){
	assert(self_p);
	zbusconn_cfg_t* self = *self_p;
	if(self){
		zfree(self);
		*self_p = NULL;
	}
}
void
zbuswrk_cfg_destroy(zbuswrk_cfg_t** self_p){
	assert(self_p);
	zbuswrk_cfg_t* self = *self_p;
	if(self){
		zfree(self);
		*self_p = NULL;
	}
}
void
asyn_ctrl_destroy(asyn_ctrl_t** self_p){
	assert(self_p);
	asyn_ctrl_t* self = *self_p;
	if(self){
		zfree(self);
		*self_p = NULL;
	}
}

