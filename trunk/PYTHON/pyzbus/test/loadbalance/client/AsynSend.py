from pyzbus import BusClient, AsynCtrl
import time

client = BusClient(host='127.0.0.1', port=15555)

ctrl = AsynCtrl()
ctrl.service = 'MyService' 
ctrl.msg_id = int(time.time())
ctrl.peer_id = 'local_mq'
ctrl.timeout = 2500 #ms

message = ['hello @%s'%time.time()]

result = client.send(ctrl, message) 
print result

client.destroy()    

