#include "jzbus.hpp"
#include "net_zbus_ConnectionUnsafe.h"

static zbusconn_t*
	get_connection (JNIEnv* env, jobject obj);
static void 
	put_connection (JNIEnv *env, jobject obj, zbusconn_t* conn);
static zbuswrk_t*
	get_worker (JNIEnv* env, jobject obj);

JNIEXPORT void JNICALL 
Java_net_zbus_ConnectionUnsafe_construct(JNIEnv *env, jobject obj, jobject jcfg){
	
	zbusconn_t* conn = get_connection(env, obj);
	if(conn) return;

	zbusconn_cfg_t cfg;
	memset(&cfg, 0, sizeof(cfg));

	jclass cls = env->GetObjectClass (jcfg);
	if(!cls){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_construct: cls is null");
		return;
	} 
	//context
	jfieldID zctx_field = env->GetFieldID(cls, "ctx", "Lnet/zbus/ZContext;");
	if(!zctx_field){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_construct: zctx_field is null");
		return;
	} 
	jobject ctx_obj = env->GetObjectField(jcfg, zctx_field);
	if(ctx_obj){  
		jclass ctx_cls = env->GetObjectClass (ctx_obj);  
		if(!ctx_cls){
			raise_exception(env, "Java_net_zbus_ConnectionUnsafe_construct: ctx_cls is null");
			return;
		} 
		jfieldID jctx = env->GetFieldID (ctx_cls, "handle", "J"); 
		if(!jctx){
			raise_exception(env, "Java_net_zbus_ConnectionUnsafe_construct: jctx is null");
			return;
		} 
		env->DeleteLocalRef (ctx_cls);
		void* ctx = (void*) env->GetLongField (ctx_obj, jctx);		
		cfg.ctx = ctx;
	}  
	//host 
	jfieldID host_field = env->GetFieldID(cls, "host", "Ljava/lang/String;");
	if(!host_field){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_construct: host_field is null");
		return;
	} 
	jstring jhost = (jstring)env->GetObjectField(jcfg, host_field);
	if(jhost){
		const char* host = env->GetStringUTFChars(jhost, 0); 
		strcpy(cfg.host, host);
		env->ReleaseStringUTFChars(jhost, host);
	} else {
		raise_exception(env, "ConnectionConfig, host required");
	}

	//port
	jfieldID port_field = env->GetFieldID(cls, "port", "I");
	if(!port_field){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_construct: port_field is null");
		return;
	} 
	int port = (int)env->GetIntField(jcfg, port_field);  
	cfg.port = port;

	//verbose
	jfieldID verbose_field = env->GetFieldID(cls, "verbose", "Z");
	if(!verbose_field){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_construct: verbose_field is null");
		return;
	} 
	int verbose = (int)env->GetBooleanField(jcfg, verbose_field); 
	cfg.verbose = verbose;   

	//id 
	jfieldID id_field = env->GetFieldID(cls, "id", "Ljava/lang/String;");
	if(!id_field){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_construct: id_field is null");
		return;
	} 
	jstring jid = (jstring)env->GetObjectField(jcfg, id_field);
	if(jid){
		const char* id = env->GetStringUTFChars(jid, 0); 
		strcpy(cfg.id, id);
		env->ReleaseStringUTFChars(jid, id);
	}    

	env->DeleteLocalRef (cls); 
 

	conn = zbusconn_new(&cfg); 
	if(!conn){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_construct: conn is null");
		return;
	} 

	put_connection(env, obj, conn);
}

JNIEXPORT void JNICALL 
Java_net_zbus_ConnectionUnsafe_finalize(JNIEnv* env, jobject obj){ 
	zbusconn_t* conn = get_connection(env, obj);
	if(!conn){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_finalize: conn is null");
		return;
	} 
	zbusconn_destroy(&conn);
}

JNIEXPORT jint JNICALL 
Java_net_zbus_ConnectionUnsafe_zbusconn_1reconnect(JNIEnv *env, jobject obj){
	zbusconn_t* conn = get_connection(env, obj);
	if(!conn){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_zbusconn_1reconnect: conn is null");
		return -3;
	} 
	return zbusconn_reconnect(conn);
}
JNIEXPORT jint 
JNICALL Java_net_zbus_ConnectionUnsafe_zbusconn_1send(JNIEnv *env, jobject obj, jobjectArray jmsg){
	zmsg_t* req = zmsg_new();
	jsize req_size = env->GetArrayLength(jmsg);
	for(int i=0; i<req_size; i++){
		jbyteArray frame = (jbyteArray)env->GetObjectArrayElement(jmsg, i);
		jbyte* data = env->GetByteArrayElements(frame, 0);
		jsize size = env->GetArrayLength(frame); 
		zmsg_push_back(req, zframe_new(data, size));

		env->ReleaseByteArrayElements(frame, data, 0);
	} 
	
	zbusconn_t* conn = get_connection(env, obj);
	if(!conn){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_zbusconn_1send: conn is null");
		return -3;
	} 

	return zbusconn_send(conn, req);
}

JNIEXPORT jobjectArray JNICALL 
Java_net_zbus_ConnectionUnsafe_zbusconn_1recv(JNIEnv *env, jobject obj, jint timeout){
	zbusconn_t* conn = get_connection(env, obj);
	if(!conn){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_zbusconn_1recv: conn is null");
		return NULL;
	}
	zmsg_t* res = zbusconn_recv(conn, timeout);
	if(!res){
		raise_exception(env, "request timeout");
		return NULL;
	}

	jsize res_size = zmsg_frame_size(res); 
	jclass cls_byte_array = env->FindClass("[B");
	if(!cls_byte_array){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_zbusconn_1recv: cls_byte_array is null");
		return NULL;
	} 

	jobjectArray result = env->NewObjectArray(res_size, cls_byte_array, 0);
	env->DeleteLocalRef(cls_byte_array);


	for(jsize i=0;i<res_size;i++){
		zframe_t* frame = zmsg_pop_front(res);
		jsize size = zframe_size(frame);
		jbyteArray jframe = env->NewByteArray(size); 
		env->SetByteArrayRegion(jframe, 0, size,(jbyte*) zframe_data(frame));
		env->SetObjectArrayElement(result, i, jframe);
		zframe_destroy(&frame);
	} 
	zmsg_destroy(&res);   
	return result;
}


JNIEXPORT jint JNICALL 
Java_net_zbus_ConnectionUnsafe_zbusconn_1route
(JNIEnv *env, jobject obj, jbyteArray jsock_id, jobjectArray jmsg){
	zbusconn_t* conn = get_connection(env, obj);
	if(!conn){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_zbusconn_1route: conn is null");
		return -3;
	} 
	
	jbyte* data = env->GetByteArrayElements(jsock_id, 0);
	jsize size = env->GetArrayLength(jsock_id); 

	zframe_t* sock_id = zframe_new(data, size);
	env->ReleaseByteArrayElements(jsock_id, data, 0);


	zmsg_t* req = zmsg_new();
	jsize req_size = env->GetArrayLength(jmsg);
	for(int i=0; i<req_size; i++){
		jbyteArray frame = (jbyteArray)env->GetObjectArrayElement(jmsg, i);
		jbyte* data = env->GetByteArrayElements(frame, 0);
		jsize size = env->GetArrayLength(frame); 
		zmsg_push_back(req, zframe_new(data, size));

		env->ReleaseByteArrayElements(frame, data, 0);
	} 
	return zbusconn_route(conn, sock_id, req);
	
}

JNIEXPORT jint JNICALL 
Java_net_zbus_ConnectionUnsafe_zbusconn_1probe(JNIEnv *env, jobject obj, jint timeout){
	zbusconn_t* conn = get_connection(env, obj);
	if(!conn){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_zbusconn_1probe: conn is null");
		return -3;
	} 
	return zbusconn_probe(conn, timeout); 
}



JNIEXPORT jobjectArray JNICALL 
Java_net_zbus_ConnectionUnsafe_zbuscli_1request
(JNIEnv *env, jobject obj, jstring jsvc, jstring jtoken, jobjectArray jmsg, jint timeout){
	zbusconn_t* conn = get_connection(env, obj);
	if(!conn){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_zbuscli_1request: conn is null");
		return NULL;
	} 

	zmsg_t* req = zmsg_new();
	jsize req_size = env->GetArrayLength(jmsg);
	for(int i=0; i<req_size; i++){
		jbyteArray frame = (jbyteArray)env->GetObjectArrayElement(jmsg, i);
		jbyte* data = env->GetByteArrayElements(frame, 0);
		jsize size = env->GetArrayLength(frame); 
		zmsg_push_back(req, zframe_new(data, size));

		env->ReleaseByteArrayElements(frame, data, 0);
	} 

	const char* svc = env->GetStringUTFChars(jsvc, 0);
	const char* token = env->GetStringUTFChars(jtoken, 0);

	zmsg_t* res = zbuscli_request(conn, (char*)svc,(char*)token, req, timeout);

	env->ReleaseStringUTFChars(jsvc, svc);
	env->ReleaseStringUTFChars(jtoken, token);

	if(!res){
		raise_exception(env, "request timeout");
		return NULL;
	}

	jsize res_size = zmsg_frame_size(res); 

	jclass cls_byte_array = env->FindClass("[B");
	if(!cls_byte_array){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_zbuscli_1request: cls_byte_array is null");
		return NULL;
	} 

	jobjectArray result = env->NewObjectArray(res_size, cls_byte_array, 0);
	env->DeleteLocalRef(cls_byte_array);


	for(jsize i=0;i<res_size;i++){
		zframe_t* frame = zmsg_pop_front(res);
		jsize size = zframe_size(frame);
		jbyteArray jframe = env->NewByteArray(size); 
		env->SetByteArrayRegion(jframe, 0, size,(jbyte*) zframe_data(frame));
		env->SetObjectArrayElement(result, i, jframe);
		zframe_destroy(&frame);
	} 
	zmsg_destroy(&res);   
	return result;
}

JNIEXPORT jobjectArray JNICALL 
Java_net_zbus_ConnectionUnsafe_zbuscli_1send
(JNIEnv * env, jobject obj, jobject jctrl, jobjectArray jmsg){
	zbusconn_t* conn = get_connection(env, obj);
	if(!conn){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_zbuscli_1send: conn is null");
		return NULL;
	} 

	asyn_ctrl_t ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	jclass cls = env->GetObjectClass (jctrl);
	if(!cls){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_zbuscli_1send: cls is null");
		return NULL;
	} 
	//service 
	jfieldID svc_field = env->GetFieldID(cls, "service", "Ljava/lang/String;");
	if(!svc_field){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_zbuscli_1send: svc_field is null");
		return NULL;
	}
	jstring jsvc = (jstring)env->GetObjectField(jctrl, svc_field);
	if(jsvc){
		const char* svc = env->GetStringUTFChars(jsvc, 0); 
		strcpy(ctrl.service, svc);
		env->ReleaseStringUTFChars(jsvc, svc);
	} else {
		raise_exception(env, "ConnectionConfig, service required");
	}
	

	//token 
	jfieldID token_field = env->GetFieldID(cls, "token", "Ljava/lang/String;");
	if(!token_field){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_zbuscli_1send: token_field is null");
		return NULL;
	} 
	jstring jtoken = (jstring)env->GetObjectField(jctrl, token_field);
	if(jtoken){
		const char* token = env->GetStringUTFChars(jtoken, 0); 
		strcpy(ctrl.token, token);
		env->ReleaseStringUTFChars(jtoken, token);
	} 

	//peer_id 
	jfieldID peer_id_field = env->GetFieldID(cls, "peerId", "Ljava/lang/String;");
	if(!peer_id_field){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_zbuscli_1send: peer_id_field is null");
		return NULL;
	} 
	jstring jpeer_id = (jstring)env->GetObjectField(jctrl, peer_id_field);
	if(jpeer_id){
		const char* peer_id = env->GetStringUTFChars(jpeer_id, 0); 
		strcpy(ctrl.peer_id, peer_id);
		env->ReleaseStringUTFChars(jpeer_id, peer_id);
	}
	

	//msg_id 
	jfieldID msg_id_field = env->GetFieldID(cls, "messageId", "Ljava/lang/String;");
	if(!msg_id_field){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_zbuscli_1send: msg_id_field is null");
		return NULL;
	} 
	jstring jmsg_id = (jstring)env->GetObjectField(jctrl, msg_id_field);
	if(jmsg_id){
		const char* msg_id = env->GetStringUTFChars(jmsg_id, 0); 
		strcpy(ctrl.msg_id, msg_id);
		env->ReleaseStringUTFChars(jmsg_id, msg_id);
	}
 
	//timeout 
	jfieldID timeout_field = env->GetFieldID(cls, "timeout", "I");
	if(!timeout_field){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_zbuscli_1send: timeout_field is null");
		return NULL;
	} 
	int timeout = (int)env->GetIntField(jctrl, timeout_field);  
	ctrl.timeout = timeout;



	zmsg_t* req = zmsg_new();
	jsize req_size = env->GetArrayLength(jmsg);
	for(int i=0; i<req_size; i++){
		jbyteArray frame = (jbyteArray)env->GetObjectArrayElement(jmsg, i);
		jbyte* data = env->GetByteArrayElements(frame, 0);
		jsize size = env->GetArrayLength(frame);
		zmsg_push_back(req, zframe_new(data, size));

		env->ReleaseByteArrayElements(frame, data, 0);
	} 

	zmsg_t* res = zbuscli_send(conn, &ctrl, req);

	if(!res){
		raise_exception(env, "request timeout");
		return NULL;
	}
	jsize res_size = zmsg_frame_size(res); 
	jclass cls_byte_array = env->FindClass("[B");
	if(!cls_byte_array){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_zbuscli_1send: cls_byte_array is null");
		return NULL;
	} 
	jobjectArray result = env->NewObjectArray(res_size, cls_byte_array, 0);
	env->DeleteLocalRef(cls_byte_array);


	for(jsize i=0;i<res_size;i++){
		zframe_t* frame = zmsg_pop_front(res);
		jsize size = zframe_size(frame);
		jbyteArray jframe = env->NewByteArray(size); 
		env->SetByteArrayRegion(jframe, 0, size,(jbyte*) zframe_data(frame));
		env->SetObjectArrayElement(result, i, jframe);
		zframe_destroy(&frame);
	} 
	zmsg_destroy(&res);   
	return result;

	return NULL;

}
JNIEXPORT jobjectArray JNICALL 
Java_net_zbus_ConnectionUnsafe_zbuscli_1recv(JNIEnv * env, jobject obj, jint ping_interval){
	zbusconn_t* conn = get_connection(env, obj);
	if(!conn){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_zbuscli_1recv: conn is null");
		return NULL;
	} 

	zmsg_t* res = zbuscli_recv(conn,  ping_interval);

	if(!res){
		raise_exception(env, "request timeout");
		return NULL;
	}

	jsize res_size = zmsg_frame_size(res); 
	jclass cls_byte_array = env->FindClass("[B");
	if(!cls_byte_array){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_zbuscli_1recv: cls_byte_array is null");
		return NULL;
	} 
	jobjectArray result = env->NewObjectArray(res_size, cls_byte_array, 0);
	env->DeleteLocalRef(cls_byte_array);


	for(jsize i=0;i<res_size;i++){
		zframe_t* frame = zmsg_pop_front(res);
		jsize size = zframe_size(frame);
		jbyteArray jframe = env->NewByteArray(size); 
		env->SetByteArrayRegion(jframe, 0, size,(jbyte*) zframe_data(frame));
		env->SetObjectArrayElement(result, i, jframe);
		zframe_destroy(&frame);
	} 
	zmsg_destroy(&res);   
	return result;
}


JNIEXPORT jint JNICALL Java_net_zbus_ConnectionUnsafe_zbuswrk_1send
(JNIEnv * env, jobject obj, jobject jconn, jobject jworker, jobjectArray jmsg){
	zbusconn_t* conn = get_connection(env, jconn);
	if(!conn){
		raise_exception(env, "ConneciontUnsafe instance required");
		return -3;
	}

	zbuswrk_t* worker = get_worker(env, jworker);
	if(!worker){
		raise_exception(env, "Worker instance required");
		return -3;
	}
	
	jsize req_size = env->GetArrayLength(jmsg);
	if(req_size < 2){
		raise_exception(env, "Worker, jmsg at lest 2 frames required");
		return -3;
	}

	zmsg_t* req = zmsg_new(); 
	for(int i=0; i<req_size; i++){
		jbyteArray frame = (jbyteArray)env->GetObjectArrayElement(jmsg, i);
		jbyte* data = env->GetByteArrayElements(frame, 0);
		jsize size = env->GetArrayLength(frame); 
		zmsg_push_back(req, zframe_new(data, size));

		env->ReleaseByteArrayElements(frame, data, 0);
	} 
	
	zframe_t* sock_id = zmsg_pop_front(req);
	zframe_t* msg_id = zmsg_pop_front(req);
	zbuswrk_set_address(worker, sock_id, msg_id);
	
	return zbuswrk_send(conn, worker, req);

}


JNIEXPORT jobjectArray JNICALL Java_net_zbus_ConnectionUnsafe_zbuswrk_1recv
(JNIEnv *env, jobject obj, jobject jconn, jobject jworker){
	zbusconn_t* conn = get_connection(env, jconn);
	if(!conn){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_zbuswrk_1recv: instance required");
		return NULL;
	}

	zbuswrk_t* worker = get_worker(env, jworker);
	if(!worker){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_zbuswrk_1recv: Worker instance required");
		return NULL;
	}

	zmsg_t* res = zbuswrk_recv(conn, worker);
	
	if(!res){
		raise_exception(env, "zbuswrk_recv null");
		return NULL;
	}
 
	zframe_t* sock_id, * msg_id;
	zbuswrk_get_address(worker, &sock_id, &msg_id);
	zmsg_push_front(res, msg_id); 
	zmsg_push_front(res, sock_id); 

	jsize res_size = zmsg_frame_size(res); 

	jclass cls_byte_array = env->FindClass("[B");
	if(!cls_byte_array){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_zbuswrk_1recv: cls_byte_array is null");
		return NULL;
	}
	jobjectArray result = env->NewObjectArray(res_size, cls_byte_array, 0);
	env->DeleteLocalRef(cls_byte_array);


	for(jsize i=0;i<res_size;i++){
		zframe_t* frame = zmsg_pop_front(res);
		jsize size = zframe_size(frame);
		jbyteArray jframe = env->NewByteArray(size); 
		env->SetByteArrayRegion(jframe, 0, size,(jbyte*) zframe_data(frame));
		env->SetObjectArrayElement(result, i, jframe);
		zframe_destroy(&frame);
	} 
	zmsg_destroy(&res);   
	return result; 
}


JNIEXPORT jint JNICALL Java_net_zbus_ConnectionUnsafe_zbuswrk_1subscribe
(JNIEnv * env, jobject obj, jobject jconn, jobject jworker, jstring jtopic){
	zbusconn_t* conn = get_connection(env, jconn);
	if(!conn){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_zbuswrk_1subscribe: instance required");
		return NULL;
	}

	zbuswrk_t* worker = get_worker(env, jworker);
	if(!worker){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_zbuswrk_1subscribe: Worker instance required");
		return NULL;
	}
 
	const char* topic = env->GetStringUTFChars(jtopic, 0);

	int res = zbuswrk_subscribe(conn, worker, (char*)topic);
 
	env->ReleaseStringUTFChars(jtopic, topic);
 
	return res; 
}

JNIEXPORT jint JNICALL Java_net_zbus_ConnectionUnsafe_zbuswrk_1unsubscribe
(JNIEnv * env, jobject obj, jobject jconn, jobject jworker, jstring jtopic){
	zbusconn_t* conn = get_connection(env, jconn);
	if(!conn){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_zbuswrk_1unsubscribe: instance required");
		return NULL;
	}

	zbuswrk_t* worker = get_worker(env, jworker);
	if(!worker){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_zbuswrk_1unsubscribe: Worker instance required");
		return NULL;
	}

	const char* topic = env->GetStringUTFChars(jtopic, 0);

	int res = zbuswrk_unsubscribe(conn, worker, (char*)topic);

	env->ReleaseStringUTFChars(jtopic, topic);

	return res; 
}


JNIEXPORT jobjectArray JNICALL Java_net_zbus_ConnectionUnsafe_zbusmon_1request
(JNIEnv *env, jobject obj, jstring jtoken, jobjectArray jmsg, jint timeout){
	zbusconn_t* conn = get_connection(env, obj);
	if(!conn){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_zbusmon_1request: conn instance required");
		return NULL;
	}

	zmsg_t* req = zmsg_new();
	jsize req_size = env->GetArrayLength(jmsg);
	for(int i=0; i<req_size; i++){
		jbyteArray frame = (jbyteArray)env->GetObjectArrayElement(jmsg, i);
		jbyte* data = env->GetByteArrayElements(frame, 0);
		jsize size = env->GetArrayLength(frame); 
		zmsg_push_back(req, zframe_new(data, size));

		env->ReleaseByteArrayElements(frame, data, 0);
	} 
 
	const char* token = env->GetStringUTFChars(jtoken, 0);

	zmsg_t* res = zbusmon_request(conn,(char*)token, req, timeout);
 
	env->ReleaseStringUTFChars(jtoken, token);

	if(!res){
		raise_exception(env, "request timeout");
		return NULL;
	}

	jsize res_size = zmsg_frame_size(res); 

	jclass cls_byte_array = env->FindClass("[B");
	if(!cls_byte_array){
		raise_exception(env, "Java_net_zbus_ConnectionUnsafe_zbusmon_1request: cls_byte_array is null");
		return NULL;
	}
	jobjectArray result = env->NewObjectArray(res_size, cls_byte_array, 0);
	env->DeleteLocalRef(cls_byte_array);


	for(jsize i=0;i<res_size;i++){
		zframe_t* frame = zmsg_pop_front(res);
		jsize size = zframe_size(frame);
		jbyteArray jframe = env->NewByteArray(size); 
		env->SetByteArrayRegion(jframe, 0, size,(jbyte*) zframe_data(frame));
		env->SetObjectArrayElement(result, i, jframe);
		zframe_destroy(&frame);
	} 
	zmsg_destroy(&res);   
	return result;

}


static zbusconn_t*
get_connection (JNIEnv* env, jobject obj){
	jclass cls = env->GetObjectClass (obj);
	if(!cls){
		raise_exception(env, "get_connection: cls is null");
		return NULL;
	}
	jfieldID jhandle = env->GetFieldID (cls, "handle", "J");
	if(!jhandle){
		raise_exception(env, "get_connection: jhandle is null");
		return NULL;
	}
	env->DeleteLocalRef (cls); 

	zbusconn_t* conn = (zbusconn_t*) env->GetLongField(obj, jhandle);
	return conn;
}

static void 
put_connection (JNIEnv *env, jobject obj, zbusconn_t* conn){
	jclass cls = env->GetObjectClass (obj);
	if(!cls){
		raise_exception(env, "put_connection: cls is null");
		return;
	}
	jfieldID jhandle = env->GetFieldID (cls, "handle", "J");
	if(!jhandle){
		raise_exception(env, "put_connection: jhandle is null");
		return;
	}
	env->DeleteLocalRef (cls); 

	env->SetLongField(obj, jhandle, (jlong)conn);
}

static zbuswrk_t*
get_worker (JNIEnv* env, jobject obj)
{
	jclass cls = env->GetObjectClass (obj);
	if(!cls){
		raise_exception(env, "get_worker: cls is null");
		return NULL;
	}
	jfieldID jhandle = env->GetFieldID (cls, "handle", "J");
	if(!jhandle){
		raise_exception(env, "get_worker: jhandle is null");
		return NULL;
	}
	env->DeleteLocalRef (cls); 

	zbuswrk_t* wrk = (zbuswrk_t*) env->GetLongField(obj, jhandle);
	return wrk;
}