#include <assert.h> 
#include "jzbus.hpp"

/**
 * Raise an exception that includes 0MQ's error message.
 */
void raise_exception (JNIEnv *env, char* err)
{
    //  Get exception class.
    jclass exception_class = env->FindClass ("net/zbus/ZBusException");
    assert (exception_class);

    //  Get exception class constructor
    jmethodID constructor_method = env->GetMethodID(exception_class,
    	"<init>", "(Ljava/lang/String;)V");
    assert (constructor_method);
    
    jstring err_str = env->NewStringUTF(err);

    //  Create exception class instance
    jthrowable exception = static_cast<jthrowable>(env->NewObject(
    	exception_class, constructor_method, err_str));

    //  Raise the exception.
    int rc = env->Throw (exception);
    env->DeleteLocalRef (exception_class);
    env->DeleteLocalRef (err_str);

    assert (rc == 0);
}
