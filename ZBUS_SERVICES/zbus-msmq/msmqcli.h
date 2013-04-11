#ifndef __MSMQCLI_H__
#define __MSMQCLI_H__

#include <Windows.h> 

#import "mqoa.dll"
using namespace MSMQ;

typedef struct _mqcli_t{
	IMSMQQueuePtr qsend;
	IMSMQQueuePtr qrecv;

	IMSMQQueueInfoPtr info_send;
	IMSMQQueueInfoPtr info_recv;

	int		timeout;
} mqcli_t;


mqcli_t*
	mqcli_new(char* server, char* client, int timeout);
void
	mqcli_destroy(mqcli_t** self_p); 
int 
	mqcli_send(mqcli_t* self, char* msg);

#endif