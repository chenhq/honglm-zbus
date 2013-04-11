#include "jzbus.hpp" 
#include "net_zbus_ZContext.h"


static void*
	get_context (JNIEnv *env, jobject obj);
static void 
	put_context (JNIEnv *env, jobject obj, void *s);


/**
 * Called to construct a Java Context object.
 */
JNIEXPORT void JNICALL 
Java_net_zbus_ZContext_construct(
	JNIEnv *env,jobject obj, jint io_threads)
{  
	void *c = get_context (env, obj);
    if (c) return;

    c = zmq_init (io_threads);
    int err = zmq_errno();
    put_context (env, obj, c);

    if (c == NULL) {
		char error[256];
		sprintf(error, "context init error: %s",zmq_strerror(err)); 
        raise_exception (env, error);
        return;
    }
}

/**
 * Called to destroy a Java Context object.
 */
JNIEXPORT void JNICALL 
Java_net_zbus_ZContext_finalize (
	JNIEnv *env,jobject obj)
{
    void *c = get_context (env, obj);
    if (! c)  return;

    int rc = zmq_term (c);
    int err = zmq_errno();
    c = NULL;
    put_context (env, obj, c);

    if (rc != 0) {
		char error[256];
		sprintf(error, "context term error: %s",zmq_strerror(err)); 
		raise_exception (env, error);
        return;
    } 
}

 
 
static void *
get_context (JNIEnv *env, jobject obj)
{ 
	jclass cls = env->GetObjectClass (obj);
	if(!cls){
		raise_exception(env, "get_context: cls is null");
		return NULL;
	} 
	jfieldID jcontext = env->GetFieldID (cls, "handle", "J");
	if(!jcontext){
		raise_exception(env, "get_context: jcontext is null");
		return NULL;
	} 
	env->DeleteLocalRef (cls);
    void *s = (void*) env->GetLongField (obj, jcontext);
    return s;
}
 
static void 
put_context (JNIEnv *env, jobject obj, void *s)
{
	jclass cls = env->GetObjectClass (obj);
	if(!cls){
		raise_exception(env, "put_context: cls is null");
		return;
	} 
	jfieldID jcontext = env->GetFieldID (cls, "handle", "J");
	if(!jcontext){
		raise_exception(env, "put_context: jcontext is null");
		return;
	} 
	env->DeleteLocalRef (cls);
    env->SetLongField (obj, jcontext, (jlong) s);
}
