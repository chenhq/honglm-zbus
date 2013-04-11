/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class net_zbus_ConnectionUnsafe */

#ifndef _Included_net_zbus_ConnectionUnsafe
#define _Included_net_zbus_ConnectionUnsafe
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     net_zbus_ConnectionUnsafe
 * Method:    construct
 * Signature: (Lnet/zbus/ConnectionConfig;)V
 */
JNIEXPORT void JNICALL Java_net_zbus_ConnectionUnsafe_construct
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_zbus_ConnectionUnsafe
 * Method:    finalize
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_net_zbus_ConnectionUnsafe_finalize
  (JNIEnv *, jobject);

/*
 * Class:     net_zbus_ConnectionUnsafe
 * Method:    zbusconn_reconnect
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_zbus_ConnectionUnsafe_zbusconn_1reconnect
  (JNIEnv *, jobject);

/*
 * Class:     net_zbus_ConnectionUnsafe
 * Method:    zbusconn_send
 * Signature: ([[B)I
 */
JNIEXPORT jint JNICALL Java_net_zbus_ConnectionUnsafe_zbusconn_1send
  (JNIEnv *, jobject, jobjectArray);

/*
 * Class:     net_zbus_ConnectionUnsafe
 * Method:    zbusconn_recv
 * Signature: (I)[[B
 */
JNIEXPORT jobjectArray JNICALL Java_net_zbus_ConnectionUnsafe_zbusconn_1recv
  (JNIEnv *, jobject, jint);

/*
 * Class:     net_zbus_ConnectionUnsafe
 * Method:    zbusconn_route
 * Signature: ([B[[B)I
 */
JNIEXPORT jint JNICALL Java_net_zbus_ConnectionUnsafe_zbusconn_1route
  (JNIEnv *, jobject, jbyteArray, jobjectArray);

/*
 * Class:     net_zbus_ConnectionUnsafe
 * Method:    zbusconn_probe
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_net_zbus_ConnectionUnsafe_zbusconn_1probe
  (JNIEnv *, jobject, jint);

/*
 * Class:     net_zbus_ConnectionUnsafe
 * Method:    zbuscli_request
 * Signature: (Ljava/lang/String;Ljava/lang/String;[[BI)[[B
 */
JNIEXPORT jobjectArray JNICALL Java_net_zbus_ConnectionUnsafe_zbuscli_1request
  (JNIEnv *, jobject, jstring, jstring, jobjectArray, jint);

/*
 * Class:     net_zbus_ConnectionUnsafe
 * Method:    zbuscli_send
 * Signature: (Lnet/zbus/AsynCtrl;[[B)[[B
 */
JNIEXPORT jobjectArray JNICALL Java_net_zbus_ConnectionUnsafe_zbuscli_1send
  (JNIEnv *, jobject, jobject, jobjectArray);

/*
 * Class:     net_zbus_ConnectionUnsafe
 * Method:    zbuscli_recv
 * Signature: (I)[[B
 */
JNIEXPORT jobjectArray JNICALL Java_net_zbus_ConnectionUnsafe_zbuscli_1recv
  (JNIEnv *, jobject, jint);

/*
 * Class:     net_zbus_ConnectionUnsafe
 * Method:    zbuswrk_send
 * Signature: (Lnet/zbus/ConnectionUnsafe;Lnet/zbus/Worker;[[B)I
 */
JNIEXPORT jint JNICALL Java_net_zbus_ConnectionUnsafe_zbuswrk_1send
  (JNIEnv *, jobject, jobject, jobject, jobjectArray);

/*
 * Class:     net_zbus_ConnectionUnsafe
 * Method:    zbuswrk_recv
 * Signature: (Lnet/zbus/ConnectionUnsafe;Lnet/zbus/Worker;)[[B
 */
JNIEXPORT jobjectArray JNICALL Java_net_zbus_ConnectionUnsafe_zbuswrk_1recv
  (JNIEnv *, jobject, jobject, jobject);

/*
 * Class:     net_zbus_ConnectionUnsafe
 * Method:    zbuswrk_subscribe
 * Signature: (Lnet/zbus/ConnectionUnsafe;Lnet/zbus/Worker;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_net_zbus_ConnectionUnsafe_zbuswrk_1subscribe
  (JNIEnv *, jobject, jobject, jobject, jstring);

/*
 * Class:     net_zbus_ConnectionUnsafe
 * Method:    zbuswrk_unsubscribe
 * Signature: (Lnet/zbus/ConnectionUnsafe;Lnet/zbus/Worker;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_net_zbus_ConnectionUnsafe_zbuswrk_1unsubscribe
  (JNIEnv *, jobject, jobject, jobject, jstring);

/*
 * Class:     net_zbus_ConnectionUnsafe
 * Method:    zbusmon_request
 * Signature: (Ljava/lang/String;[[BI)[[B
 */
JNIEXPORT jobjectArray JNICALL Java_net_zbus_ConnectionUnsafe_zbusmon_1request
  (JNIEnv *, jobject, jstring, jobjectArray, jint);

#ifdef __cplusplus
}
#endif
#endif
