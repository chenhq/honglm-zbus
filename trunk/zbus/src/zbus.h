/*
 * zbus.h
 *
 *  Created on: 3 Aug, 2012
 *      Author: hong
 */

#ifndef __ZBUS_H__
#define __ZBUS_H__

#include "include/zmsg.h"

#define ZBUS_VERSION		"v20121205"

//  This is the version of MDP/Client we implement
#define MDPC_CLIENT         "MDPC01"

//  This is the version of MDP/Worker we implement
#define MDPW_WORKER         "MDPW01"

//  This is the version of MDP/Monitor we implement
#define MDPM_MONITOR         "MDPM01"

//  MDP/Server commands, as strings
#define MDPW_READY          "\001"
#define MDPW_REQUEST        "\002"
#define MDPW_REPLY          "\003"
#define MDPW_HEARTBEAT      "\004"
#define MDPW_DISCONNECT     "\005"
#define MDPW_SYNC		    "\006"

#define MODE_REQREP				1
#define MODE_MQ					2
#define MODE_PUBSUB				3

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
	worker_process (zframe_t *sender, zmsg_t *msg);

void
	worker_command (zframe_t *worker, char *command, zmsg_t *msg);
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


void
	client_process (zframe_t *sender, zmsg_t *msg);
void
	client_command(zframe_t *sender, zframe_t *service, char* status, char* content);


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
	monitor_command(zframe_t *address, char* status, char* content);
void
	monitor_command_msg(zframe_t *address, char* status, zmsg_t* msg);
void
	monitor_process (zframe_t *sender, zmsg_t *msg);

#endif /* __ZBUS_H__ */
