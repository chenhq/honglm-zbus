#include "jzbus.hpp"
#include "net_zbus_Worker.h"
  
static zbuswrk_t*
	get_worker (JNIEnv* env, jobject obj);
static void 
	put_worker (JNIEnv *env, jobject obj, zbuswrk_t* wrk); 


JNIEXPORT void JNICALL Java_net_zbus_Worker_construct
(JNIEnv *env, jobject obj, jobject jcfg){
	zbuswrk_t* self = get_worker(env, obj);
	if(self) return;
		
	zbuswrk_cfg_t cfg;
	memset(&cfg, 0, sizeof(cfg));

	jclass cls = env->GetObjectClass (jcfg);
	assert (cls); 
	//service
	jfieldID svc_field = env->GetFieldID(cls, "service", "Ljava/lang/String;");
	assert (svc_field);
	jstring jsvc = (jstring)env->GetObjectField(jcfg, svc_field);
	if(jsvc){
		const char* svc = env->GetStringUTFChars(jsvc, 0); 
		strcpy(cfg.service, svc);
		env->ReleaseStringUTFChars(jsvc, svc);
	} else {
		raise_exception(env, "WorkerConfig, service required");
	}
	//mode
	jfieldID mode_field = env->GetFieldID(cls, "mode", "Ljava/lang/String;");
	assert (mode_field);
	jstring jmode = (jstring)env->GetObjectField(jcfg, mode_field);
	if(jmode){
		const char* mode = env->GetStringUTFChars(jmode, 0); 
		strcpy(cfg.mode, mode);
		env->ReleaseStringUTFChars(jmode, mode);

		if(strcmp(cfg.mode, MODE_LB)!=0 && 
			strcmp(cfg.mode, MODE_BC)!=0 && strcmp(cfg.mode, MODE_PUBSUB)!=0 ){
			raise_exception(env, "WorkerConfig, mode not support");
		}
	} 
	
	//registerToken
	jfieldID regtoken_field = env->GetFieldID(cls, "registerToken", "Ljava/lang/String;");
	assert (regtoken_field);
	jstring jregtoken = (jstring)env->GetObjectField(jcfg, regtoken_field);
	if(jregtoken){
		const char* regtoken = env->GetStringUTFChars(jregtoken, 0); 
		strcpy(cfg.reg_token, regtoken);
		env->ReleaseStringUTFChars(jregtoken, regtoken);
	} 

	//accessToken
	jfieldID acctoken_field = env->GetFieldID(cls, "accessToken", "Ljava/lang/String;");
	assert (acctoken_field);
	jstring jacctoken = (jstring)env->GetObjectField(jcfg, acctoken_field);
	if(jacctoken){
		const char* acctoken = env->GetStringUTFChars(jacctoken, 0); 
		strcpy(cfg.acc_token, acctoken);
		env->ReleaseStringUTFChars(jacctoken, acctoken);
	} 
 
	self = zbuswrk_new(&cfg);
	put_worker(env, obj, self); 
}

JNIEXPORT void JNICALL 
Java_net_zbus_Worker_finalize(
	JNIEnv* env, jobject obj)
{
	zbuswrk_t* self = get_worker(env, obj);
	assert(self);
	zbuswrk_destroy(&self); 
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
static void 
put_worker(JNIEnv *env, jobject obj, zbuswrk_t* wrk)
{
	jclass cls = env->GetObjectClass (obj);
	if(!cls){
		raise_exception(env, "put_worker: cls is null");
		return;
	} 
	jfieldID jhandle = env->GetFieldID (cls, "handle", "J");
	if(!jhandle){
		raise_exception(env, "put_worker: jhandle is null");
		return;
	} 
	env->DeleteLocalRef (cls); 

	env->SetLongField(obj, jhandle, (jlong)wrk);
} 