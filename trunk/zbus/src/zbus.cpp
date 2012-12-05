#include "include/list.h"
#include "include/hash.h"

#include "zbus.h"

struct _zbus_t{
	void* 	ctx;
	void* 	socket;
	int 	verbose;
	char* 	endpoint;
	hash_t* services;
	hash_t* workers;

	uint64_t 	heartbeat_at;      	//  when to send HEARTBEAT
	int 		heartbeat_liveness;
	int 		heartbeat_interval;	//	msecs
	int			heartbeat_expiry;

	uint64_t 	msgclean_at;      	//  when to clean message
	uint64_t    workerclean_at;
    int			worker_timeout;
    int			msg_timeout;

	uint64_t	max_tmq_size; //total MQ size
	uint64_t 	max_smq_size; //single service MQ size
	uint64_t	mq_size;

	int64_t	    hwm;		  //high watermark of socket

	char* 		admin_token; 		// 	token authorised to admin this bus
	char* 		register_token;		//  token authorised to register new service

	char*		log;
};

struct _service_t{
	char *		name;              	//  Service name
	list_t *	requests;          	//  List of client requests
	list_t *	workers;           	//  List of waiting workers

	uint64_t    serve_at;
	uint64_t	mq_size;
	size_t		worker_cnt;         //  Registered worker count
	char* 		token;				//  Token authorised to call this service
	int			type;				//  Service type
};

//  This defines one worker, idle or active
struct _worker_t{
	char*		identity;         	//  Identity of worker
	zframe_t*	address;          	//  Address frame to route to
	service_t*	service;         	//  Owning service, if known
	int64_t 	expiry;             //  Expires at unless heartbeat
	zmsg_t*		msg_flying;			//  Message already sent via this worker
};

zbus_t *zbus = NULL;

static char*
option(int argc, char* argv[], char* opt, char* default_value){
	int i,len;
	char* value = default_value;
	for(i=1; i<argc; i++){
		len = strlen(opt);
		if(len> strlen(argv[i])) len = strlen(argv[i]);
		if(strncmp(argv[i],opt,len)==0){
			value = &argv[i][len];
		}
	}
	return value;
}

static uint64_t
parse_size(char* str){
	str = strdup(str);
	int len = strlen(str);
	uint64_t res = 0;
	if(toupper(str[len-1])=='G'){
		str[len-1] = '\0';
		res = (uint64_t)atoi(str)*1024*1024*1024;
	}else if(toupper(str[len-1])=='M'){
		str[len-1] = '\0';
		res = atoi(str)*1024*1024L;
	}else if(toupper(str[len-1])=='K'){
		str[len-1] = '\0';
		res = atoi(str)*1024L;
	}else{
		res = atol(str);
	}
	free(str);
	return res;
}

static void
_service_destroy(void *privdata, void* ptr){
	service_t* service = (service_t*)ptr;
	if(service){
		service_destroy(&service);
	}
}

static void
_worker_destroy(void *privdata, void* ptr){
	worker_t * worker = (worker_t*)ptr;
	if(worker){
		if(worker->service)
			worker->service->worker_cnt--;
		worker_destroy(&worker);
	}
}

hash_ctrl_t hash_ctrl_string_service = {
	hash_func_string,            /* hash function */
	hash_dup_string,             /* key dup */
	NULL,                	     /* val dup */
	hash_cmp_string,             /* key compare */
	hash_destroy_string,         /* key destructor */
	_service_destroy,   	     /* val destructor */
};

hash_ctrl_t hash_ctrl_string_worker = {
	hash_func_string,            /* hash function */
	hash_dup_string,             /* key dup */
	NULL,                	     /* val dup */
	hash_cmp_string,             /* key compare */
	hash_destroy_string,         /* key destructor */
	_worker_destroy,             /* val destructor */
};

int main (int argc, char *argv []){
	if(argc>1 && ( strcmp(argv[1],"help")==0
			|| strcmp(argv[1],"-help")==0
			|| strcmp(argv[1],"--help")==0
			|| strcmp(argv[1],"-h")==0)){

		printf("===============================================================================\n");
		printf("                              zbus(%s) manual\n",ZBUS_VERSION);
		printf("-v\tverbose mode to show message, e.g. -v0, -v1 -v3\n");
		printf("-p\tzbus listen port, e.g. -p15555 \n");
		printf("-log\tlog file path, e.g. -log.\n");
		printf("-reg\tzbus worker register service token, e.g. -regxyz (xyz is token)\n");
		printf("-adm\tzbus admin token, e.g. -admxyz (xyz is token)\n");
		printf("-io\tzbus background io threads e.g. -io2 \n");
		printf("-tmq\tzbus total message queue size e.g. -tmq2G \n");
		printf("-smq\tzbus single service message queue size e.g. -smq200M \n");
		printf("-liv\tzbus heartbeat liveness(try times) e.g. -liv3 \n");
		printf("-int\tzbus heartbeat interval e.g. -int2500 \n");
		printf("-wto\tzbus worker timeout e.g. -wto7500 \n");
		printf("-mto\tzbus message timeout e.g. -mto60000 \n");
		printf("-hwm\tzbus socket high watermark e.g. -hwm1000 \n");
		printf("==============================================================================\n");
		return 0;
	}

	zbus = zbus_new(argc, argv);
	assert(zbus);

	while(1){
		zmq_pollitem_t items [] = { { zbus->socket,  0, ZMQ_POLLIN, 0 } };

		int rc = zmq_poll (items, 1, zbus->heartbeat_interval * ZMQ_POLL_MSEC);
		if (rc == -1)
			break; //  Interrupted

		if (items [0].revents & ZMQ_POLLIN) {
			zmsg_t* msg = zmsg_recv(zbus->socket);
			if(!msg) break; //interrupted


			if(zbus->verbose){ //ignore heartbeat message
				zframe_t* header = NULL, *command = NULL;
				int idx = 0;
				list_node_t* node = (list_node_t*)list_head(zmsg_frames(msg));
				while(node){
					if(idx == 2){
						header = (zframe_t*)list_value(node);
					} else if(idx == 3){
						command = (zframe_t*)list_value(node);
					} else if (idx > 3){
						break;
					}
					idx++;
					node = list_next(node);
				}

				int heartbeat = header && command && zframe_streq(header,MDPW_WORKER)
						&& zframe_streq(command, MDPW_HEARTBEAT);

				if(!heartbeat){
					zmsg_log(msg);
				}
			}


			if(zmsg_frame_size(msg)<3){
				zlog("[ERROR]: invalid message: should be at least 3 frames\n");
				zmsg_destroy(&msg);
			}else{
				zframe_t* sender = zmsg_pop_front(msg);
				zframe_t* empty  = zmsg_pop_front(msg);
				zframe_t* header = zmsg_pop_front(msg);

				if(!zframe_streq(empty, "")){
					zlog("[ERROR]: invalid, missing empty frame");
					zmsg_destroy(&msg);
				}else if(zframe_streq(header, MDPW_WORKER)){
					//worker
					worker_process(sender, msg);
				}else if(zframe_streq(header, MDPC_CLIENT)){
					//client
					client_process(sender, msg);
				}else if(zframe_streq(header, MDPM_MONITOR)){
					//monitor
					monitor_process(sender, msg);
				}else{
					zlog("[ERROR]: invalid, wrong  header frame ");
					zmsg_destroy(&msg);
				}
				zframe_destroy(&sender);
				zframe_destroy(&empty);
				zframe_destroy(&header);
			}
		}

		//heartbeating, every default of 2.5s
		if(zclock_time() > zbus->heartbeat_at){
			zbus_heartbeat();
		}

		//clean timeout workers if any, every default of 25s
		if(zclock_time() > zbus->workerclean_at){
			zbus_clean_worker();
		}

	}

	zbus_destroy(&zbus);
	return 0;
}

void
zbus_heartbeat(){
	zbus->heartbeat_at = zclock_time() + zbus->heartbeat_interval;

	if(zbus->verbose == VERBOSE_CONSOLE){
		time_t curtime = time (NULL);
		struct tm *loctime = localtime (&curtime);
		char formatted [32];
		strftime (formatted, 32, "%Y-%m-%d %H:%M:%S ", loctime);
		fprintf(stdout, "%s heap:[%010ld] | services: [%02ld] | workers:[%04ld]\n",
			formatted, zmalloc_used_memory(),hash_size(zbus->services), hash_size(zbus->workers));
	}

	hash_iter_t* svc_iter = hash_iter_new(zbus->services);
	hash_entry_t* he = hash_iter_next(svc_iter);
	while(he){//iterate over services
		service_t* service = (service_t*)hash_entry_val(he);

		if(zclock_time() > service->serve_at + zbus->msg_timeout){
			//service->serve_at = zclock_time();
			if(0){//list_size(service->requests)){ //disable
				//clear timeout messages
				zlog("service[%s] clear timeout messages\n", service->name);
				zmsg_t* msg = service_deque_request(service);
				while(msg){
					zmsg_destroy(&msg);
					msg = service_deque_request(service);
				}
			}
		}

		list_node_t* node = list_head(service->workers);
		while(node){//for all waiting workers
			worker_t* worker = (worker_t*)list_value(node);
			list_node_t* next_node = list_next(node);

			if(zclock_time() > worker->expiry){//expired workers
				zlog("(-) unregister expired worker(%s)\n", worker->identity);
				list_remove_node(service->workers, node);
				hash_rem(zbus->workers, worker->identity);
			} else {
				worker_command(worker->address, MDPW_HEARTBEAT, NULL);
			}
			node = next_node;
		}
		he = hash_iter_next(svc_iter);
	}
	hash_iter_destroy(&svc_iter);
}

void
zbus_clean_worker(){
	zbus->workerclean_at = zclock_time() + zbus->worker_timeout;
	hash_iter_t* worker_iter = hash_iter_new(zbus->workers);
	hash_entry_t* he = hash_iter_next(worker_iter);
	while(he){
		worker_t* worker = (worker_t*)hash_entry_val(he);
		if(zclock_time() > worker->expiry + zbus->worker_timeout){
			zlog("(-) unregister timeout worker(%s)\n", worker->identity);
			list_remove(worker->service->workers, worker);
			hash_rem(zbus->workers, worker->identity);
		}
		he = hash_iter_next(worker_iter);
	}
	hash_iter_destroy(&worker_iter);
}

void
worker_process (zframe_t *sender, zmsg_t *msg){
	if(zmsg_frame_size(msg) < 1){ //invalid message
		zmsg_destroy(&msg);
		return;
	}

	char* worker_id = zframe_strhex(sender);
	worker_t* worker = (worker_t*)hash_get(zbus->workers, worker_id);
	zframe_t* command = zmsg_pop_front(msg);


	if(zframe_streq(command, MDPW_READY)){
		if(!worker){//register
			worker = worker_new(sender);
			worker_register(worker, msg);
		} else { //exist worker, synchronise worker
			worker_command(worker->address, MDPW_SYNC, NULL);
			worker_unregister(worker);
		}
		goto destroy;
	}

	if(!worker) { //wrong
		zlog("synchronize peer(%s)\n", worker_id);
		worker_command(sender, MDPW_SYNC, NULL);
		goto destroy;
	}

	//worker, right status
	if(zframe_streq(command, MDPW_REPLY)){
		if(worker->service->type == MODE_REQREP){
			zframe_t* client = zmsg_unwrap(msg);
			char* svc_name = worker->service->name;
			zmsg_push_front(msg, zframe_newstr(svc_name));
			zmsg_push_front(msg, zframe_newstr(MDPC_CLIENT));
			zmsg_wrap(msg, client);

			zmsg_send(&msg, zbus->socket);
			//worker idle again, enqueue to waiting list
			worker_waiting(worker);
		} else if(worker->service->type == MODE_MQ){
			worker_waiting(worker);
		}
	} else if(zframe_streq(command, MDPW_HEARTBEAT)){
		worker->expiry = zclock_time() + zbus->heartbeat_expiry;
	} else if(zframe_streq(command, MDPW_DISCONNECT)){
		worker_unregister(worker);
	} else {
		//ignore message
		zlog("invalid worker message\n");
	}

destroy:
	zfree(worker_id);
	zframe_destroy(&command);
	zmsg_destroy(&msg);
}

void
client_process (zframe_t *sender, zmsg_t *msg){
	if(zmsg_frame_size(msg)<2){ //fixed: changed from 3 to 2, to allow sending empty message,
		//invalid request just discard
		zmsg_destroy(&msg);
		return;
	}
	zframe_t* service_frame = zmsg_pop_front(msg);
	zframe_t* token_frame	= zmsg_pop_front(msg);

	//service lookup
	char* service_name = zframe_strdup(service_frame);
	service_t* service = (service_t*)hash_get(zbus->services,service_name);
	zfree(service_name);
	if(!service){
		client_command(sender, service_frame, "404","service not found");
		zmsg_destroy(&msg);
		goto destroy;
	}

	if(service->token && !zframe_streq(token_frame, service->token)){
		client_command(sender, service_frame, "403","forbidden, wrong token");
		zmsg_destroy(&msg);
		goto destroy;
	}

	zmsg_wrap(msg, zframe_dup(sender));
	service_dispatch(service, msg);

destroy:
	zframe_destroy(&service_frame);
	zframe_destroy(&token_frame);
}


worker_t*
worker_new(zframe_t* address){
	worker_t* self = (worker_t *) zmalloc (sizeof (worker_t));
	memset(self, 0, sizeof(worker_t));
	self->identity = zframe_strhex(address);
	self->address = zframe_dup(address);
	return self;
}

void
worker_destroy(worker_t** self_p){
	assert(self_p);
	worker_t* self = *self_p;
	if(self){
		if(self->identity)
			zfree(self->identity);
		if(self->address)
			zframe_destroy(&self->address);
		zfree(self);
		*self_p = NULL;
	}
}

void
worker_register (worker_t *worker, zmsg_t *msg){
	assert(worker);
	assert(msg);
	if(zmsg_frame_size(msg) < 4){
		worker_invalid(worker, "svc_name, reg_token, acc_token, type all required");
		zlog("invalid worker, need service name, register, access token, type\n");
		return;
	}
	zframe_t *service_frame  = zmsg_pop_front (msg);
	zframe_t *register_token = zmsg_pop_front (msg);
	zframe_t *access_token   = zmsg_pop_front (msg);

	//service type
	zframe_t *type_frame	 = zmsg_pop_front (msg);
	char* type_str = zframe_strdup(type_frame);
	int type = atoi(type_str);
	zfree(type_str);
	zframe_destroy (&type_frame);

	char* service_name = zframe_strdup(service_frame);
	service_t* service = (service_t*) hash_get(zbus->services, service_name);

	if(zbus->register_token && !zframe_streq(register_token, zbus->register_token)){
		worker_invalid(worker, "unauthorised, register token not matched");
		goto destroy;
	}

	if(type != MODE_REQREP && type != MODE_MQ && type != MODE_PUBSUB){
		worker_invalid(worker, "type frame wrong");
		goto destroy;
	}

	if(service && service->token){
		if(!zframe_streq(access_token, service->token)){
			worker_invalid(worker, "unauthorised, access token not matched");
			goto destroy;
		}
	}

	if(service && type != service->type){
		worker_invalid(worker, "service type not matched");
		goto destroy;
	}

	if(!service){
		service = service_new(service_frame, access_token, type);
		hash_put(zbus->services, service_name, service);
		zlog ("(+) register service(%s)\n", service_name);
	}

	zlog("(+) register worker(%s)\n", worker->identity);
	worker->service = service;
	worker->service->worker_cnt++;
	hash_put(zbus->workers, worker->identity, worker);

	worker_waiting(worker);

destroy:
	zfree(service_name);
	zframe_destroy (&service_frame);
	zframe_destroy (&register_token);
	zframe_destroy (&access_token);
}

service_t*
service_new(zframe_t* service_name, zframe_t* access_token, int type){
	service_t* self = (service_t*)zmalloc(sizeof(service_t));
	memset(self, 0, sizeof(service_t));
	self->name = zframe_strdup(service_name);
	self->mq_size = 0;
	self->serve_at = zclock_time();
	self->requests = list_new();

	self->workers  = list_new();
	self->token = NULL;
	self->type = type;

	if(!zframe_streq(access_token, "")){
		self->token = zframe_strdup(access_token);
	}
	return self;
}

void
service_destroy(service_t** self_p){
	assert(self_p);
	service_t* self = *self_p;
	if(self){
		if(self->name)
			zfree(self->name);
		if(self->token)
			zfree(self->token);

		if(self->requests){
			//destroy queued zmsgs
			zmsg_t* msg = service_deque_request(self);
			while(msg){
				zmsg_destroy(&msg);
				msg = service_deque_request(self);
			}
			list_destroy(&self->requests);
		}
		if(self->workers)
			list_destroy(&self->workers);
		zfree(self);
		*self_p = NULL;
	}
}


void
worker_unregister (worker_t* worker){
	zlog("(-) unregister worker(%s)\n", worker->identity);

	if(worker->service){ //if service attached
		list_remove(worker->service->workers, worker); //remove reference
	}
	hash_rem(zbus->workers, worker->identity); //implicit destroy worker
}

void
worker_waiting (worker_t *worker){
	assert(worker);

	list_push_back(worker->service->workers, worker);

	worker->expiry = zclock_time() + zbus->heartbeat_expiry;
	service_dispatch(worker->service, NULL);
}

void
worker_invalid (worker_t* worker, char* reason){
	zmsg_t* msg = zmsg_new();
	zmsg_push_back(msg, zframe_newstr(reason));
	worker_command (worker->address, MDPW_DISCONNECT, msg);
	worker_destroy(&worker);
}


void
service_dispatch (service_t *service, zmsg_t *msg){
	if(msg){
		size_t msg_size = zmsg_content_size(msg);
		if(zbus->mq_size + msg_size > zbus->max_tmq_size){
			zframe_t* sender = zmsg_unwrap(msg);
			zframe_t* svc = zframe_newstr(service->name);
			client_command(sender, svc , "500", "Total MQ full");
			zframe_destroy(&sender);
			zframe_destroy(&svc);
			zmsg_destroy(&msg);
		}else if ((service->mq_size + msg_size) > zbus->max_smq_size){
    		//service message queue full
    		zframe_t* sender = zmsg_unwrap(msg);
			zframe_t* svc = zframe_newstr(service->name);
			client_command(sender, svc ,"500", "Single MQ full");
			zframe_destroy(&sender);
			zframe_destroy(&svc);
			zmsg_destroy(&msg);
		}else{
			if(service->type == MODE_MQ || service->type == MODE_PUBSUB){
				zframe_t* sender = zmsg_unwrap(msg);
				zframe_t* svc = zframe_newstr(service->name);
				client_command(sender, svc ,"200", "OK");
				zframe_destroy(&svc);
				zmsg_wrap(msg, sender);
			}
			service_enque_request(service, msg);
		}
	}

	if(service->type == MODE_REQREP){
		while(list_size(service->workers) && list_size(service->requests)){
			worker_t* worker = (worker_t*)list_pop_front(service->workers);
			zmsg_t* msg = service_deque_request(service);
			worker_command(worker->address, MDPW_REQUEST, msg);
		}
	} else if(service->type == MODE_MQ){ // send 200,OK directly
		while(list_size(service->workers) && list_size(service->requests)){
			worker_t* worker = (worker_t*)list_pop_front(service->workers);
			zmsg_t* msg = service_deque_request(service);
			worker_command(worker->address, MDPW_REQUEST, msg);
		}
	} else if(service->type == MODE_PUBSUB){
		while(list_size(service->workers) && list_size(service->requests)){
			zmsg_t* msg = service_deque_request(service);
			list_node_t* node = list_head(service->workers);
			while(node){
				zmsg_t* msg_copy = zmsg_dup(msg);
				worker_t* worker = (worker_t*)list_value(node);
				worker_command(worker->address, MDPW_REQUEST, msg_copy);
				node = list_next(node);
			}
			zmsg_destroy(&msg);
		}
	} else {
		zlog("service type not support");
	}

}



inline void
service_enque_request(service_t *service, zmsg_t *msg){
	size_t msg_size = zmsg_content_size(msg);
	list_push_back(service->requests, msg);
	service->mq_size += msg_size;
	zbus->mq_size += msg_size;
}

inline zmsg_t*
service_deque_request(service_t *service){
	zmsg_t* msg = (zmsg_t*)list_pop_front(service->requests);
	if(msg){
		service->serve_at = zclock_time(); //mark last serve time
		size_t msg_size = zmsg_content_size(msg);
		service->mq_size -= msg_size;
		zbus->mq_size -= msg_size;
	}
	return msg;
}


//~
zbus_t*
zbus_new(int argc, char* argv[]){
	int rc;
	char endpoint[64];
	zbus_t *self = (zbus_t *) zmalloc (sizeof (zbus_t));
	memset(self, 0, sizeof(zbus_t));

	int io_threads = atoi(option(argc, argv, "-io","1"));
	self->ctx = zctx_new(io_threads);

	sprintf(endpoint, "tcp://*:%s", option(argc,argv,"-p","15555"));
	self->endpoint = zstrdup(endpoint);
	self->verbose = atoi(option(argc,argv,"-v","1"));

	self->heartbeat_liveness = atoi(option(argc,argv,"-liv","3"));
	self->heartbeat_interval = atoi(option(argc,argv,"-int","2500"));
	self->heartbeat_expiry = self->heartbeat_liveness * self->heartbeat_interval;
	self->heartbeat_at = zclock_time () + self->heartbeat_interval;

	self->worker_timeout = atoi(option(argc,argv,"-wto","7500")); //worker timeout 15s = 7.5+2.5*3
	self->workerclean_at = zclock_time () + self->worker_timeout;

	self->msg_timeout = atoi(option(argc,argv,"-mto","60000"));
	self->msgclean_at = zclock_time () + self->msg_timeout;

	self->register_token  = zstrdup( option(argc,argv,"-reg",NULL) );
	self->admin_token  = zstrdup( option(argc,argv,"-adm",NULL) );
	self->max_tmq_size = parse_size(option(argc,argv,"-tmq","2G"));
	self->max_smq_size = parse_size(option(argc,argv,"-smq","1G"));
	self->mq_size = 0;
	self->hwm = atol(option(argc,argv,"-hwm","1000"));
	self->log = zstrdup(option(argc,argv,"-log", "."));

	zlog_set_log_path(self->log);


	self->services = hash_new(&hash_ctrl_string_service,NULL);
	self->workers = hash_new(&hash_ctrl_string_worker,NULL);

	self->socket = zmq_socket(self->ctx, ZMQ_ROUTER);
	assert(self->socket);

	//size_t len = sizeof(self->hwm);
	//rc = zmq_setsockopt(self->socket, ZMQ_SNDHWM, &self->hwm, len);
    //assert(rc == 0);

	rc = zmq_bind(self->socket, self->endpoint);
	if(rc == -1){
		printf("zbus start failed, (%s) %s", self->endpoint, zmq_strerror(zmq_errno()));
		abort();
	}
	printf("zbus started: address(%s)\n", self->endpoint);
	zlog("zbus started: address(%s)\n", self->endpoint);
	return self;
}

void
zbus_destroy(zbus_t** self_p){
	assert(self_p);
	zbus_t* self = *self_p;
	if(self){
		if(self->endpoint)
			zfree(self->endpoint);
		if(self->register_token)
			zfree(self->register_token);
		if(self->admin_token)
			zfree(self->admin_token);
		if(self->log)
			zfree(self->log);

		if(self->services)
			hash_destroy(&self->services);
		if(self->workers)
			hash_destroy(&self->workers);

		if(self->ctx)
			zctx_destroy(&self->ctx);
		zfree(self);

		*self_p = NULL;
	}
}

void
client_command(zframe_t *address, zframe_t *service, char* status, char* content){
	zmsg_t* msg = zmsg_new();
	zmsg_wrap(msg, zframe_dup(address));
	zmsg_push_back(msg, zframe_newstr(MDPC_CLIENT));
	zmsg_push_back(msg, zframe_dup(service));
	zmsg_push_back(msg, zframe_newstr(status));
	zmsg_push_back(msg, zframe_newstr(content));

	zmsg_send(&msg, zbus->socket); //implicit destroy msg
}

void
worker_command (zframe_t *address, char *command, zmsg_t *msg){
	if(!msg)
		msg = zmsg_new();

	zmsg_push_front(msg, zframe_newstr(command));
	zmsg_push_front(msg, zframe_newstr(MDPW_WORKER));
	zmsg_wrap(msg, zframe_dup(address));

	zmsg_send(&msg, zbus->socket);
}

void
monitor_command(zframe_t *address, char* status, char* content){
	zmsg_t* msg = zmsg_new();
	zmsg_wrap(msg, zframe_dup(address));
	zmsg_push_back(msg, zframe_newstr(MDPM_MONITOR));
	zmsg_push_back(msg, zframe_newstr(status));
	zmsg_push_back(msg, zframe_newstr(content));

	zmsg_send(&msg, zbus->socket); //implicit destroy msg
}

void
monitor_command_msg(zframe_t *address, char* status, zmsg_t* msg){
	zmsg_push_front(msg, zframe_newstr(status));
	zmsg_push_front(msg, zframe_newstr(MDPM_MONITOR));
	zmsg_wrap(msg, zframe_dup(address));
	zmsg_send(&msg, zbus->socket); //implicit destroy msg
}
///////////////////////////// monitor ///////////////////////////
static char*
service_dump(service_t *service){
	assert( service );
	char* token = "";
	if(service->token) token = service->token;

	int len = strlen(service->name) + strlen(token) + 128;
	char* report_str = (char*) zmalloc(len);
	//<name><token><que-size><req-count>
	sprintf(report_str, "%s\t%d\t%s\t%lu\t%d\t%d\t%d\t%lu",
			service->name,
			service->type,
			token,
			service->mq_size,
			(int)list_size(service->requests),
			(int)list_size(service->workers), //waiting worker count
			(int)service->worker_cnt, //registered worker count
			service->serve_at
			);
	return report_str;
}

void
monitor_cmd_ls (zframe_t *sender, zmsg_t *params){ //ls command
	if(zmsg_frame_size(params) == 0){// list all
		zmsg_t* msg = zmsg_new();
		hash_iter_t* iter = hash_iter_new(zbus->services);
		hash_entry_t* he = hash_iter_next(iter);
		while(he){
			service_t* svc = (service_t*)hash_entry_val(he);
			char* svc_dump = service_dump(svc);
			zmsg_push_back(msg, zframe_newstr(svc_dump));
			zfree(svc_dump);

			he = hash_iter_next(iter);
		}
		hash_iter_destroy(&iter);
		monitor_command_msg(sender,"200", msg);

	} else if (zmsg_frame_size(params) == 1){//list specified service
		zframe_t* service_frame = zmsg_pop_front(params);
		//service lookup
		char* service_name = zframe_strdup(service_frame);
		service_t* service = (service_t*)hash_get(zbus->services,service_name);

		if(!service){
			char content[256];
			sprintf(content, "service( %s ), not found", service_name);
			monitor_command(sender, "500", content);
		} else {
			char* res = service_dump(service);
			monitor_command(sender, "200", res);
			zfree(res);
		}

		zfree(service_name);
		zframe_destroy(&service_frame);
	} else {
		monitor_command(sender, "500", "ls [service], require 0 or 1 parameter");
	}
}

// 3) clear svc
void
monitor_cmd_clear (zframe_t *sender, zmsg_t *params){ //clear command
	if(zmsg_frame_size(params) != 1){// clear svc
		monitor_command(sender, "500", "clear service, require 1 parameter");
		return;
	}

	zframe_t* service_frame = zmsg_pop_front(params);
	//service lookup
	char* service_name = zframe_strdup(service_frame);
	service_t* service = (service_t*)hash_get(zbus->services,service_name);

	if(!service){
		char content[256];
		sprintf(content, "service( %s ), not found", service_name);
		monitor_command(sender, "500", content);
	} else {
		if(service->requests){
			//destroy queued zmsgs
			zmsg_t* msg = service_deque_request(service);
			while(msg){
				zmsg_destroy(&msg);
				msg = service_deque_request(service);
			}
		}
		monitor_command(sender, "200", "OK");
	}

	zfree(service_name);
	zframe_destroy(&service_frame);
}

void
monitor_cmd_del (zframe_t *sender, zmsg_t *params){ //del command
	if(zmsg_frame_size(params) != 1){// clear svc
		monitor_command(sender, "500", "del service, require 1 parameter");
		return;
	}

	zframe_t* service_frame = zmsg_pop_front(params);
	//service lookup
	char* service_name = zframe_strdup(service_frame);
	service_t* service = (service_t*)hash_get(zbus->services,service_name);

	if(!service){
		char content[256];
		sprintf(content, "service( %s ), not found", service_name);
		monitor_command(sender, "500", content);
	} else {
		zlog("(-) unregister service(%s)\n", service_name);
		list_node_t* node = list_head(service->workers);
		while(node){
			worker_t* worker = (worker_t*) list_value(node);
			zmsg_t* msg = zmsg_new();
			zmsg_push_back(msg, zframe_newstr("service is going to be destroyed by broker"));
			worker_command (worker->address, MDPW_DISCONNECT, msg); //disconnect worker
			worker_unregister(worker);
			node = list_next(node);
		}
		hash_rem(zbus->services, service_name);
		monitor_command(sender, "200", "OK");
	}

	zfree(service_name);
	zframe_destroy(&service_frame);
}

// 4) msg svc start end
void
monitor_cmd_msg (zframe_t *sender, zmsg_t *params){ //msg command

}

// 5) reg svc token
void
monitor_cmd_reg (zframe_t *sender, zmsg_t *params){ //msg command
	if(zmsg_frame_size(params) != 3){// clear svc
		monitor_command(sender, "500", "reg <service> <token> <type>, require 3 parameter");
		return;
	}

	zframe_t* service_frame = zmsg_pop_front(params);
	zframe_t* token_frame = zmsg_pop_front(params);
	zframe_t* type_frame = zmsg_pop_front(params);

	char* type_str = zframe_strdup(type_frame);
	int type = atoi(type_str);
	zfree(type_str);

	//service lookup
	char* service_name = zframe_strdup(service_frame);
	service_t* service = (service_t*)hash_get(zbus->services,service_name);

	if(service){
		char content[256];
		sprintf(content, "service( %s ) already exists", service_name);
		monitor_command(sender, "500", content);
		goto destroy;
	}

	if(type != MODE_REQREP && type != MODE_MQ && type != MODE_PUBSUB){
		monitor_command(sender, "500", "type (1,2 or 3) wrong");
		goto destroy;
	}

	service = service_new(service_frame, token_frame, type);
	hash_put(zbus->services, service_name, service);
	zlog ("(+) register service(%s)\n", service_name);
	monitor_command(sender, "200", "OK");
destroy:
	zfree(service_name);
	zframe_destroy(&service_frame);
	zframe_destroy(&token_frame);
	zframe_destroy(&type_frame);
}


// 1) ls [svc]
// 2) del svc
// 3) clear svc
// 4) msg svc start end
// 5) reg svc token
void
monitor_process (zframe_t *sender, zmsg_t *msg){
	if(zmsg_frame_size(msg)<2){
		zmsg_destroy(&msg);
		return;
	}
	zframe_t* token_frame	= zmsg_pop_front(msg);
	zframe_t* cmd_frame = zmsg_pop_front(msg);

	if(zbus->admin_token && !zframe_streq(token_frame, zbus->admin_token)){
		monitor_command(sender, "403", "wrong administrator token");
		goto destroy;
	}

	if(zframe_streq(cmd_frame, "ls")){
		monitor_cmd_ls(sender, msg);
	} else if(zframe_streq(cmd_frame, "clear")){
		monitor_cmd_clear(sender, msg);
	} else if(zframe_streq(cmd_frame, "del")){
		monitor_cmd_del(sender, msg);
	} else if(zframe_streq(cmd_frame, "msg")){
		monitor_cmd_msg(sender, msg);
	} else if(zframe_streq(cmd_frame, "reg")){
		monitor_cmd_reg(sender, msg);
	} else {
		monitor_command(sender, "404", "unknown command");
	}

destroy:
	zframe_destroy(&token_frame);
	zframe_destroy(&cmd_frame);
	zmsg_destroy(&msg);
}
