/*
 * zbus.h
 *
 *  Created on: 3 Aug, 2012
 *      Author: hong
 */

#ifndef __ZBUS_H__
#define __ZBUS_H__
#include "include/zmq.h"
#include "zbox/include/zmsg.h"

#define ZBUS_VERSION		"v20130113"
 
//the following three will be removed
#define MODE_MQ		2 
#define MDPW_REQ    "\002"
#define MDPW_REP    "\003"

#define MDPC        "MDPC01" 
#define MDPW        "MDPW01" 
#define MDPM        "MDPM01"
#define MDPX		"MDPX01"
#define MDPQ		"MDPQ01"
#define MDPT		"MDPT01"

#define MODE_LB		1
#define MODE_BC		3

#define MDPW_REG	"\001"
#define MDPW_JOB    "\002"
#define MDPW_HBT	"\004"
#define MDPW_DISC   "\005"
#define MDPW_SYNC	"\006"
#define MDPW_IDLE	"\007"  

#define HEARTBEAT_INTERVAL		2500
#define HEARTBEAT_LIVENESS		3
 
#define VERBOSE_CONSOLE			3

typedef struct _zbus_t zbus_t;
typedef struct _service_t service_t;
typedef struct _worker_t worker_t;

zbus_t*
	zbus_new(int argc, char* argv[]);
void
	zbus_destroy(zbus_t** self_p);

void
	zbus_heartbeat();
void
	zbus_clean_worker();


void
	zlog(const char *format, ...);
void
	zlog_msg(zmsg_t *msg);


void
	client_process (zframe_t *sender, zmsg_t *msg);
void
	route_process  (zframe_t *sender, zmsg_t *msg);
void
	worker_process (zframe_t *sender, zmsg_t *msg);
void
	monitor_process(zframe_t *sender, zmsg_t *msg);
void
	queue_process  (zframe_t *sender, zmsg_t *msg);
void
	ping_process   (zframe_t *sender, zmsg_t *msg);

void
	worker_command (zframe_t *worker, char* command, zmsg_t *msg);
worker_t*
	worker_new(zframe_t* address);
void
	worker_destroy(worker_t** self_p);
void
	worker_register (worker_t *worker, zmsg_t *msg);
void
	worker_unregister (worker_t* worker);
void
	worker_waiting (worker_t *worker);
void
	worker_invalid (worker_t* worker, char* reason);

 
service_t*
	service_new(zframe_t* service_name, zframe_t* access_token, int type);
void
	service_destroy(service_t** self_p);
void
	service_dispatch (service_t *service, zmsg_t *msg);
inline void
	service_enque_request(service_t *service, zmsg_t *msg);
inline zmsg_t*
	service_deque_request(service_t *service);
void
	queue_dispatch (service_t* service, zframe_t* sender, zmsg_t* msg);

#endif /* __ZBUS_H__ */
