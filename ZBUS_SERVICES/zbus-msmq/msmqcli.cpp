#include "zbus/include/prelude.h"
#include "msmqcli.h"


#import "mqoa.dll"
using namespace MSMQ;


static 
IMSMQQueuePtr open_queue(IMSMQQueueInfoPtr qinfo, enum MQACCESS access){
	IMSMQQueuePtr queue;
	try{
		queue = qinfo->Open(access, MQ_DENY_NONE);
	}catch(_com_error& e){
		//wprintf(L"Error Code = 0x%X\nError Description = %s\n", e.Error(), (wchar_t *)e.Description());
		try{
			_variant_t IsTransactional(false);
			_variant_t IsWorldReadable(true);
			qinfo->Create(&IsTransactional, &IsWorldReadable);
			queue = open_queue(qinfo , access);
		}catch (_com_error& e){ 
			wprintf(L"Error Code = 0x%X\nError Description = %s\n", e.Error(), (wchar_t *)e.Description());
			assert(0);
		} 
	}
	return queue;
}


mqcli_t*
mqcli_new(char* server, char* client, int timeout){
	assert(server);
	assert(client);
	assert(timeout>0);

	mqcli_t* self = (mqcli_t*)zmalloc(sizeof(mqcli_t));
	assert(self);
	memset(self, 0, sizeof(mqcli_t));

	self->timeout = timeout;
	::CoInitializeEx(NULL,COINIT_MULTITHREADED); 

	self->info_send = IMSMQQueueInfoPtr("MSMQ.MSMQQueueInfo");

	char ip[64];
	sprintf(ip, "%s", client);
	for(int i=0;i<strlen(ip);i++)
		if(ip[i] == '.') ip[i] = '_';


	char buff[128];
	sprintf(buff,"DIRECT=TCP:%s\\PRIVATE$\\%s_recv", server, ip); 
	_bstr_t path_name = buff;  
	self->info_send ->FormatName = path_name;
	self->info_send ->Label = path_name ;

	self->qsend = open_queue(self->info_send , MQ_SEND_ACCESS);  
	assert(self->qsend);  


	self->info_recv = IMSMQQueueInfoPtr("MSMQ.MSMQQueueInfo");
	sprintf(buff,".\\PRIVATE$\\%s_send", ip);  
	path_name = buff;  
	self->info_recv->PathName = path_name;
	self->info_recv->Label = path_name ;
	self->qrecv = open_queue(self->info_recv, MQ_RECEIVE_ACCESS);
	assert(self->qrecv);

	return self;
}


void
mqcli_destroy(mqcli_t** self_p){
	assert(self_p);
	mqcli_t* self = *self_p;
	if(self){
		if(self->qsend){
			self->qsend->Close();
		}
		if(self->qrecv){
			self->qrecv->Close();
		}
		zfree(self);
		::CoUninitialize();
	} 
}


int 
mqcli_send(mqcli_t* self, char* msg){
	assert(self);
	IMSMQMessagePtr pmsg("MSMQ.MSMQMessage");  
	pmsg->Body = msg;
	pmsg->MaxTimeToReachQueue = self->timeout;
	pmsg->MaxTimeToReceive = self->timeout;
	try{
		pmsg->Send(self->qsend);
	}catch(_com_error& e){ 
		wprintf(L"Error Code = 0x%X\nError Description = %s\n", e.Error(), (wchar_t *)e.Description());
		return -1;
	} 
	return 0;
}
