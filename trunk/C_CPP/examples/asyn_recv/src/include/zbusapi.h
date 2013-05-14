#ifndef __ZBOX_ZBUSCLI_H__
#define __ZBOX_ZBUSCLI_H__

#include "zmsg.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (defined (__WINDOWS__))
#   if defined DLL_EXPORT
#       define ZBOX_EXPORT __declspec(dllexport)
#   else
#       define ZBOX_EXPORT __declspec(dllimport)
#   endif
#else
#   define ZBOX_EXPORT
#endif

#define MDPC        "MDPC01" 
#define MDPW        "MDPW01" 
#define MDPM        "MDPM01"
#define MDPX		"MDPX01"
#define MDPQ		"MDPQ01"
#define MDPT		"MDPT01"

//LoadBalance mode
#define MODE_LB		"1"
//BroadCast mode
#define MODE_BC		"3"

#define MDPW_REG	"\001"
#define MDPW_REQ    "\002"
#define MDPW_REP    "\003"
#define MDPW_HBT	"\004"
#define MDPW_DISC   "\005"
#define MDPW_SYNC	"\006"
#define MDPW_IDLE	"\007" 
#define MDPW_DATA	"\008" 

#define HEARTBEAT_INTERVAL		2500
#define HEARTBEAT_LIVENESS		3

/*****************************PROTOCOL***********************************/
//MDPC-REQ::= <ID>||C|<SVC>|<TOKEN>|...
//MDPC-REP::= <ID>||C|<MSG-ID>|...

//MDPT-REQ::= <ID>||T|...
//MDPT-REP::= <ID>||T|...

//MDPM-REQ::= <ID>||M|<TOKEN>|...
//MDPM-REP::= <ID>||M|<MSG-ID>|...

//MDPQ-REQ::= <ID>||Q|<SVC>|<TOKEN>|<MSG-ID>|...
//MDPQ-REP::= <ID>||C|<MSG-ID>|...

//MDPX::=     <ID>||X|{MDPC-REP}
//MDPW::=     <ID>||W|<CMD>|[PARAM-LIST]
/************************************************************************/


typedef struct _zbusconn_t zbusconn_t;
typedef struct _zbuswrk_t zbuswrk_t;

typedef struct _zbusconn_cfg_t{
	char	host[32];  //[in]required
	int		port;	   //[in]required, default zbus port: 1555
	int		verbose;   //[in]optional, verbose for debug info, log underlying message

	void*	ctx;       //[in]optional, zeromq context
	char	id[128];   //[in]optional, id of this socket
}zbusconn_cfg_t;


typedef struct _zbuswrk_cfg_t{
	char		service[128];	//[in]required, service name
	char		mode[32];		//[in]optional, service mode: MODE_LB or MODE_BC
	char		reg_token[128];	//[in]optional, zbus register token, if required
	char		acc_token[128];	//[in]optional, service access token  
} zbuswrk_cfg_t;


typedef struct _asyn_ctrl_t{
	char	service[128];		//[in]required, service name
	char	token[128];			//[in]optional, service access token
	int		timeout;			//[in]required, milliseconds (ms)
	char	peer_id[128];		//[in]optional, target peer socket id when under asynchronize mode
	char	msg_id[128];		//[in]optional, current sending out message id	
}asyn_ctrl_t;


//create zbus connection, internal called zbusconn_reconnect
ZBOX_EXPORT zbusconn_t*  
	zbusconn_new(zbusconn_cfg_t* cfg); 
//reconnect zbus connection, underly socket instance changed
ZBOX_EXPORT int 
	zbusconn_reconnect(zbusconn_t* self);
//destroy zbus connection,
ZBOX_EXPORT void 
	zbusconn_destroy(zbusconn_t** self_p); 
//asychronized send raw message to zbus, an empty frame prepended 
ZBOX_EXPORT int	    
	zbusconn_send(zbusconn_t* self, zmsg_t* msg); 
//asychronized recv raw message from zbus, an empty frame pop off head
ZBOX_EXPORT zmsg_t* 
	zbusconn_recv(zbusconn_t* self, int timeout);
//asychronised send MDPX message to zbus([MDPX, <sock_id>, <empty>] prepended)
ZBOX_EXPORT int     
	zbusconn_route(zbusconn_t* self, zframe_t* sock_id, zmsg_t* msg); 
//probe zbus broker, prevent from being disconnection such as firewall setting
ZBOX_EXPORT int 
	zbusconn_probe(zbusconn_t* self, int timeout);

//send MDPC message to zbus([MDPC, <service>, <token>, <empty>] prpended)
//wait for reply message [sychronized mode]
ZBOX_EXPORT zmsg_t* 
	zbuscli_request(zbusconn_t* self, char* service, char* token, zmsg_t* msg, int timeout);
//asychronized sending MDPQ message to zbus
//([MDPQ, <service>, <token>, <peer_id>, <msg_id>] prpended)
//indication of message delivery status 
ZBOX_EXPORT zmsg_t* 
	zbuscli_send(zbusconn_t* self, asyn_ctrl_t* ctrl, zmsg_t* msg);
//asychronized wait for reply message([<service>,<msg_id>] returned as first and second frame)
ZBOX_EXPORT zmsg_t* 
	zbuscli_recv(zbusconn_t* self, int probe_interval);


ZBOX_EXPORT zbuswrk_t*
	zbuswrk_new(zbuswrk_cfg_t* cfg);
ZBOX_EXPORT void
	zbuswrk_destroy(zbuswrk_t** self_p); 
//reply back msg to source socket(zbusconn_route called)
ZBOX_EXPORT int
	zbuswrk_send(zbusconn_t* self, zbuswrk_t* worker, zmsg_t* msg); 
ZBOX_EXPORT int
	zbuswrk_sendto(zbusconn_t* self, zframe_t* client_sock_id, zframe_t* msg_id, zmsg_t* msg); 

//recv client request message from zbus, source socket id and message id stored in worker for reply use
ZBOX_EXPORT zmsg_t*
	zbuswrk_recv(zbusconn_t* self, zbuswrk_t* worker); 
ZBOX_EXPORT void 
	zbuswrk_get_address(zbuswrk_t* worker, zframe_t** sock_id, zframe_t** msg_id);
ZBOX_EXPORT void 
	zbuswrk_set_address(zbuswrk_t* worker, zframe_t* sock_id, zframe_t* msg_id);

//send MDPM message to zbus([MDPM, <service>, <token>] prpended)
//wait for reply message
ZBOX_EXPORT zmsg_t*
	zbusmon_request(zbusconn_t* self, char* token, zmsg_t* command, int timeout);

//help functions
ZBOX_EXPORT zbusconn_cfg_t*
	zbusconn_cfg_new(char* host, int port, int verbose, void* ctx, char* id);
ZBOX_EXPORT zbuswrk_cfg_t*
	zbuswrk_cfg_new(char* service, char* mode, char* reg_token, char* acc_token);
ZBOX_EXPORT asyn_ctrl_t*
	asyn_ctrl_new(char* service, char* token, int timeout, char* peer_id, char* msg_id);
ZBOX_EXPORT void
	zbusconn_cfg_destroy(zbusconn_cfg_t** self_p);
ZBOX_EXPORT void
	zbuswrk_cfg_destroy(zbuswrk_cfg_t** self_p);
ZBOX_EXPORT void
	asyn_ctrl_destroy(asyn_ctrl_t** self_p);

#ifdef __cplusplus
}
#endif

#endif /* __ZBUSCLI_H__ */
