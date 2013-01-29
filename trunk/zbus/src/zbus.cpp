#include "zbox/include/prelude.h"
#include "zbox/include/list.h"
#include "zbox/include/hash.h"
#include "zbox/include/zmsg.h"

#include "zbus.h"

struct _zbus_t{
	void* 		ctx;
	void* 		socket;
	int 		verbose;
	char* 		endpoint;
	hash_t*		services;
	hash_t*		workers;

	int64_t 	heartbeat_at;      	//  when to send HEARTBEAT
	int 		heartbeat_liveness;
	int 		heartbeat_interval;	//	msecs
	int			heartbeat_expiry;

	int64_t 	msgclean_at;      	//  when to clean message
	int64_t     workerclean_at;
    int			worker_timeout;
    int			msg_timeout;

	int64_t		max_tmq_size; //total MQ size
	int64_t 	max_smq_size; //single service MQ size
	int64_t		mq_size;

	int64_t	    hwm;		  //high watermark of socket

	char* 		admin_token; 		// 	token authorised to admin this bus
	char* 		register_token;		//  token authorised to register new service

	char*		log;
};

struct _service_t{
	char *		name;              	//  Service name
	list_t *	requests;          	//  List of client requests
	list_t *	workers;           	//  List of waiting workers

	int64_t     serve_at;
	int64_t		mq_size;
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
};

zbus_t *zbus = NULL;
 
static int64_t
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
				worker_command(worker->address, MDPW_HBT, NULL);
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
static void
s_print_help(){
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

}
int main (int argc, char *argv []){
	if(argc>1 && ( strcmp(argv[1],"help")==0
				|| strcmp(argv[1],"-help")==0
				|| strcmp(argv[1],"--help")==0
				|| strcmp(argv[1],"-h")==0)){
		s_print_help();
		return 0;
	}
	zbus = zbus_new(argc, argv);
	assert(zbus);
	
	while(1){
		//heartbeat, every default of 2.5s
		if(zclock_time() > zbus->heartbeat_at){
			zbus_heartbeat();
		}
		//clean timeout workers if any, every default of 25s
		if(zclock_time() > zbus->workerclean_at){
			zbus_clean_worker();
		}

		zmsg_t* msg = zmsg_recv(zbus->socket);
		if(!msg) continue; //timeout

		if(zmsg_frame_size(msg)<3){
			zlog("[ERROR]: invalid message: should be at least 3 frames\n");
			if(zbus->verbose){
				zmsg_log(msg);
			}
			zmsg_destroy(&msg);
			continue;
		}  
		
		if(zbus->verbose){ //ignore heartbeat and probe message
			int ignore = 0, idx = 0;
			zframe_t* hbt = NULL, *mdp = NULL;
			list_node_t* node = list_head((list_t*)zmsg_frames(msg));
			while(node){ 
				if(idx == 2) mdp = (zframe_t*)list_value(node);
				if(idx == 3){
					hbt = (zframe_t*)list_value(node);
					break;
				}
				node = list_next(node);
				idx ++;
			}
			if(hbt && zframe_streq(hbt, MDPW_HBT)) ignore = 1;
			if(mdp && zframe_streq(mdp, MDPT)) ignore = 1;			
			
			if(!ignore) {
				zmsg_log(msg);
			}
		}

		zframe_t* sender = zmsg_pop_front(msg);
		zframe_t* empty  = zmsg_pop_front(msg);
		zframe_t* mdp = zmsg_pop_front(msg);

		if(!zframe_streq(empty, "")){
			zlog("[ERROR]: invalid, missing empty frame");
			zframe_destroy(&sender);
			zframe_destroy(&empty);
			zframe_destroy(&mdp);
			zmsg_destroy(&msg);
			continue;
		}
		
		if(zframe_streq(mdp, MDPW)){//worker
			worker_process(sender, msg);
		}else if(zframe_streq(mdp, MDPX)){//route
			route_process(sender, msg);
		}else if(zframe_streq(mdp, MDPT)){//route
			ping_process(sender, msg);
		}else if(zframe_streq(mdp, MDPC)){//client
			client_process(sender, msg);
		}else if(zframe_streq(mdp, MDPQ)){//monitor
			queue_process(sender, msg);
		}else if(zframe_streq(mdp, MDPM)){//monitor
			monitor_process(sender, msg);
		}else{
			zlog("[ERROR]: invalid, wrong mdp frame ");
			zmsg_destroy(&msg);
		}
		zframe_destroy(&sender);
		zframe_destroy(&empty);
		zframe_destroy(&mdp);
	}
	zbus_destroy(&zbus);
	return 0;
}
static void 
s_mdpc_reply(zframe_t *address, char* status, char* content){
	zmsg_t* msg = zmsg_new();
	zmsg_wrap(msg, zframe_dup(address));
	zmsg_push_back(msg, zframe_newstr(MDPC)); 
	zmsg_push_back(msg, zframe_new(NULL, 0));
	zmsg_push_back(msg, zframe_newstr(status));
	if(content)
		zmsg_push_back(msg, zframe_newstr(content));

	zmsg_send(&msg, zbus->socket); //implicit destroy msg
}

//////////////////////////////////MDPC PROCESS///////////////////////////////

//client_id:  client_id can not be destroy in this function
//msg zmsg_t:  must be destroy in this function
void client_process (zframe_t *client_id, zmsg_t *msg)
{
	if(zmsg_frame_size(msg)<2){ 
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
		s_mdpc_reply(client_id, "404","service not found");
		zmsg_destroy(&msg);
		goto destroy;
	}

	if(service->token && !zframe_streq(token_frame, service->token)){
		s_mdpc_reply(client_id, "403", "forbidden, wrong token");
		zmsg_destroy(&msg);
		goto destroy;
	}

	zmsg_wrap(msg, zframe_dup(client_id));
	service_dispatch(service, msg);

destroy:
	zframe_destroy(&service_frame);
	zframe_destroy(&token_frame);
}


static void 
s_mdpq_reply(zframe_t *address){
	zmsg_t* msg = zmsg_new();
	zmsg_wrap(msg, zframe_dup(address));
	zmsg_push_back(msg, zframe_newstr(MDPQ)); 
	zmsg_push_back(msg, zframe_new(NULL, 0)); //empty msg_id
	zmsg_push_back(msg, zframe_newstr("200")); 

	zmsg_send(&msg, zbus->socket); //implicit destroy msg
}
//////////////////////////////////MDPQ PROCESS///////////////////////////////
//sender:  client_id can not be destroy in this function
//msg zmsg_t:  must be destroy in this function
void queue_process (zframe_t *sender, zmsg_t *msg)
{
	if(zmsg_frame_size(msg)<3){ 
		//invalid request just discard
		zmsg_destroy(&msg);
		return;
	}
	zframe_t* service_frame = zmsg_pop_front(msg);
	zframe_t* token_frame	= zmsg_pop_front(msg);
	zframe_t* peerid_frame = zmsg_pop_front(msg); 

	//service lookup
	char* service_name = zframe_strdup(service_frame);
	service_t* service = (service_t*)hash_get(zbus->services,service_name);
	zfree(service_name);
	if(!service){
		s_mdpc_reply(sender, "404","service not found");
		zmsg_destroy(&msg);
		goto destroy;
	}

	if(service->token && !zframe_streq(token_frame, service->token)){
		s_mdpc_reply(sender, "403", "forbidden, wrong token");
		zmsg_destroy(&msg);
		goto destroy;
	}
	
	zmsg_wrap(msg, zframe_dup(peerid_frame));
	queue_dispatch(service, sender, msg);

destroy:
	zframe_destroy(&service_frame);
	zframe_destroy(&token_frame);
	zframe_destroy(&peerid_frame); 
}

//////////////////////////////////MDPW PROCESS///////////////////////////////
void 
worker_process (zframe_t *sender, zmsg_t *msg){
	if(zmsg_frame_size(msg) < 1){ //invalid message
		zmsg_destroy(&msg);
		return;
	}

	char* worker_id = zframe_strhex(sender);
	worker_t* worker = (worker_t*)hash_get(zbus->workers, worker_id);
	
	zframe_t* command = zmsg_pop_front(msg);
	if(zframe_streq(command,MDPW_REG)){
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
	 
	if(zframe_streq(command, MDPW_REP)){ //will be deprecated
		if(worker->service->type == MODE_LB){
			zframe_t* client = zmsg_unwrap(msg);
			char* svc_name = worker->service->name;
			zmsg_push_front(msg, zframe_newstr(svc_name));
			zmsg_push_front(msg, zframe_newstr(MDPC));
			zmsg_wrap(msg, client); 
			zmsg_send(&msg, zbus->socket); 

			worker_waiting(worker);
		} else if(worker->service->type == MODE_MQ){
			worker_waiting(worker);
		}
	} else if(zframe_streq(command,MDPW_HBT)){
		worker->expiry = zclock_time() + zbus->heartbeat_expiry;
	} else if(zframe_streq(command,MDPW_DISC)){
		worker_unregister(worker);
	/////////////////////////////NEW API///////////////////////////
	} else if(zframe_streq(command,MDPW_IDLE)){
		if(worker->service->type == MODE_LB){ //broadcast mode ignored
			worker_waiting(worker);
		}
	} else {
		zlog("invalid worker message\n");
	} 
destroy:
	zfree(worker_id); 
	zframe_destroy(&command);
	zmsg_destroy(&msg);
}



//////////////////////////////////MDPX PROCESS///////////////////////////////
void 
route_process (zframe_t *sender, zmsg_t *msg){
	//sender ignored, route to target
	zmsg_send(&msg, zbus->socket);	
}
//////////////////////////////////MDPT PROCESS///////////////////////////////
void 
ping_process (zframe_t *sender, zmsg_t *msg){
	zmsg_destroy(&msg);
	msg = zmsg_new();
	zmsg_wrap(msg, zframe_dup(sender));
	zmsg_push_back(msg, zframe_newstr(MDPT)); 
	zmsg_push_back(msg, zframe_new(NULL, 0)); //empty msg_id

	zmsg_send(&msg, zbus->socket); //implicit destroy msg
}
//////////////////////////////////MDPM PROCESS///////////////////////////////

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

	if(type != MODE_LB && type != MODE_MQ && type != MODE_BC){
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
	worker_command (worker->address, MDPW_DISC, msg);
	worker_destroy(&worker);
}

static void
s_service_dispatch(service_t* service){
	assert(service); 
	if(service->type == MODE_LB || service->type == MODE_MQ){ //TODO, remove MQ
		while(list_size(service->workers) && list_size(service->requests)){
			worker_t* worker = (worker_t*)list_pop_front(service->workers);
			zmsg_t* msg = service_deque_request(service);
			worker_command(worker->address, MDPW_JOB, msg);
		}
	} else if(service->type == MODE_BC){//broadcast
		while(list_size(service->workers) && list_size(service->requests)){
			zmsg_t* msg = service_deque_request(service);
			list_node_t* node = list_head(service->workers);
			while(node){
				zmsg_t* msg_copy = zmsg_dup(msg);
				worker_t* worker = (worker_t*)list_value(node);
				worker_command(worker->address, MDPW_JOB, msg_copy);
				node = list_next(node);
			}
			zmsg_destroy(&msg);
		}
	} else {
		zlog("service type not support");
	}
}

void
queue_dispatch (service_t* service, zframe_t* sender, zmsg_t* msg){
	assert(msg);
	size_t msg_size = zmsg_content_size(msg);
	if(zbus->mq_size + msg_size > zbus->max_tmq_size){ 
		s_mdpc_reply(sender, "500", "Total MQ full"); 
		zmsg_destroy(&msg);
	}else if ((service->mq_size + msg_size) > zbus->max_smq_size){
		//service message queue full 
		s_mdpc_reply(sender, "500", "Single MQ full"); 
		zmsg_destroy(&msg);
	}else{  
		s_mdpc_reply(sender, "200", NULL); 
		service_enque_request(service, msg);
	} 
	s_service_dispatch(service);
}
void
service_dispatch (service_t* service, zmsg_t* msg){
	if(msg){
		size_t msg_size = zmsg_content_size(msg);
		if(zbus->mq_size + msg_size > zbus->max_tmq_size){
			zframe_t* sender = zmsg_unwrap(msg); 
			s_mdpc_reply(sender, "500", "Total MQ full");
			zframe_destroy(&sender); 
			zmsg_destroy(&msg);
		}else if ((service->mq_size + msg_size) > zbus->max_smq_size){
    		//service message queue full
    		zframe_t* sender = zmsg_unwrap(msg); 
			s_mdpc_reply(sender, "500", "Single MQ full");
			zframe_destroy(&sender); 
			zmsg_destroy(&msg);
		}else{  
			service_enque_request(service, msg);
		}
	} 
	s_service_dispatch(service);
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
	self->verbose = atoi(option(argc,argv,"-v","3"));

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
	self->log = zstrdup(option(argc,argv,"-log", NULL));

	if(self->log){
		zlog_use_file(self->log);
	} else {
		zlog_use_stdout();
	} 

	self->services = hash_new(&hash_ctrl_string_service,NULL);
	self->workers = hash_new(&hash_ctrl_string_worker,NULL);

	self->socket = zmq_socket(self->ctx, ZMQ_ROUTER);
	assert(self->socket);
 
	int sndhwm = self->hwm;
	rc = zmq_setsockopt(self->socket, ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm));
    assert(rc == 0);
	int rcvhwm = self->hwm;
	rc = zmq_setsockopt(self->socket, ZMQ_RCVHWM, &rcvhwm, sizeof(rcvhwm));
	assert(rc == 0);

	int timeout = self->heartbeat_interval;
	rc = zmq_setsockopt(self->socket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
	assert(rc == 0);

	rc = zmq_bind(self->socket, self->endpoint);
	if(rc == -1){
		printf("zbus start failed, (%s) %s", self->endpoint, zmq_strerror(zmq_errno()));
		abort();
	} 
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
worker_command (zframe_t* worker_id, char* cmd, zmsg_t* args){
	if(!args) args = zmsg_new();  
	zmsg_push_front(args, zframe_newstr(cmd));
	zmsg_push_front(args, zframe_newstr(MDPW));
	zmsg_wrap(args, zframe_dup(worker_id));

	zmsg_send(&args, zbus->socket);
}

static void
s_mdpm_reply(zframe_t *address, char* status, char* content){
	zmsg_t* msg = zmsg_new();
	zmsg_wrap(msg, zframe_dup(address));
	zmsg_push_back(msg, zframe_newstr(MDPM));
	zmsg_push_back(msg, zframe_new(NULL, 0)); //empty
	zmsg_push_back(msg, zframe_newstr(status));
	zmsg_push_back(msg, zframe_newstr(content));

	zmsg_send(&msg, zbus->socket); //implicit destroy msg
}

static void
s_mdpm_reply_msg(zframe_t *address, char* status, zmsg_t* msg){
	zmsg_push_front(msg, zframe_newstr(status));
	zmsg_push_front(msg, zframe_new(NULL, 0)); //empty
	zmsg_push_front(msg, zframe_newstr(MDPM));
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
	sprintf(report_str, "%s\t%d\t%s\t%lld\t%d\t%d\t%d\t%lld",
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
		s_mdpm_reply_msg(sender,"200", msg);

	} else if (zmsg_frame_size(params) == 1){//list specified service
		zframe_t* service_frame = zmsg_pop_front(params);
		//service lookup
		char* service_name = zframe_strdup(service_frame);
		service_t* service = (service_t*)hash_get(zbus->services,service_name);

		if(!service){
			char content[256];
			sprintf(content, "service( %s ), not found", service_name);
			s_mdpm_reply(sender, "500", content);
		} else {
			char* res = service_dump(service);
			s_mdpm_reply(sender, "200", res);
			zfree(res);
		}

		zfree(service_name);
		zframe_destroy(&service_frame);
	} else {
		s_mdpm_reply(sender, "500", "ls [service], require 0 or 1 parameter");
	}
}

// 3) clear svc
void
monitor_cmd_clear (zframe_t *sender, zmsg_t *params){ //clear command
	if(zmsg_frame_size(params) != 1){// clear svc
		s_mdpm_reply(sender, "500", "clear service, require 1 parameter");
		return;
	}

	zframe_t* service_frame = zmsg_pop_front(params);
	//service lookup
	char* service_name = zframe_strdup(service_frame);
	service_t* service = (service_t*)hash_get(zbus->services,service_name);

	if(!service){
		char content[256];
		sprintf(content, "service( %s ), not found", service_name);
		s_mdpm_reply(sender, "500", content);
	} else {
		if(service->requests){
			//destroy queued zmsgs
			zmsg_t* msg = service_deque_request(service);
			while(msg){
				zmsg_destroy(&msg);
				msg = service_deque_request(service);
			}
		}
		s_mdpm_reply(sender, "200", "OK");
	}

	zfree(service_name);
	zframe_destroy(&service_frame);
}

void
monitor_cmd_del (zframe_t *sender, zmsg_t *params){ //del command
	if(zmsg_frame_size(params) != 1){// clear svc
		s_mdpm_reply(sender, "500", "del service, require 1 parameter");
		return;
	}

	zframe_t* service_frame = zmsg_pop_front(params);
	//service lookup
	char* service_name = zframe_strdup(service_frame);
	service_t* service = (service_t*)hash_get(zbus->services,service_name);

	if(!service){
		char content[256];
		sprintf(content, "service( %s ), not found", service_name);
		s_mdpm_reply(sender, "500", content);
	} else {
		zlog("(-) unregister service(%s)\n", service_name);
		list_node_t* node = list_head(service->workers);
		while(node){
			worker_t* worker = (worker_t*) list_value(node);
			zmsg_t* msg = zmsg_new();
			zmsg_push_back(msg, zframe_newstr("service is going to be destroyed by broker"));
			worker_command (worker->address, MDPW_DISC, msg); //disconnect worker
			worker_unregister(worker);
			node = list_next(node);
		}
		hash_rem(zbus->services, service_name);
		s_mdpm_reply(sender, "200", "OK");
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
		s_mdpm_reply(sender, "500", "reg <service> <token> <type>, require 3 parameter");
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
		s_mdpm_reply(sender, "500", content);
		goto destroy;
	}

	if(type != MODE_LB && type != MODE_MQ && type != MODE_BC){
		s_mdpm_reply(sender, "500", "type (1,2 or 3) wrong");
		goto destroy;
	}

	service = service_new(service_frame, token_frame, type);
	hash_put(zbus->services, service_name, service);
	zlog ("(+) register service(%s)\n", service_name);
	s_mdpm_reply(sender, "200", "OK");
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
		s_mdpm_reply(sender, "403", "wrong administrator token");
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
		s_mdpm_reply(sender, "404", "unknown command");
	}

destroy:
	zframe_destroy(&token_frame);
	zframe_destroy(&cmd_frame);
	zmsg_destroy(&msg);
}
