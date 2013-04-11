#ifndef __JZBUS_HPP_INCLUDED__
#define __JZBUS_HPP_INCLUDED__


#include "config.hpp"

#include "../zbox/include/prelude.h"
#include "../zbox/include/zbusapi.h"
#include "../include/zmq.h"
#include <jni.h>
#include <assert.h>



#if defined HAVE_INTTYPES_H

#include <inttypes.h>

#elif defined HAVE_STDINT_H

#include <stdint.h>

#elif defined _MSC_VER

#ifndef int8_t
typedef __int8 int8_t;
#endif
#ifndef int16_t
typedef __int16 int16_t;
#endif
#ifndef int32_t
typedef __int32 int32_t;
#endif
#ifndef int64_t
typedef __int64 int64_t;
#endif
#ifndef uint8_t
typedef unsigned __int8 uint8_t;
#endif
#ifndef uint16_t
typedef unsigned __int16 uint16_t;
#endif
#ifndef uint32_t
typedef unsigned __int32 uint32_t;
#endif
#ifndef uint64_t
typedef unsigned __int64 uint64_t;
#endif

#else

#error "Don't know how to define stdint types on this platform"

#endif



/**
 * Raise an exception that includes 0MQ's error message.
 */
void raise_exception (JNIEnv *env, char* err);

#endif  /* #ifndef __JZBUS_HPP_INCLUDED__ */
