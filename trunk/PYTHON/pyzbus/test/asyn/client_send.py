from pyzbus import BusClient, AsynCtrl
import time


ctrl = AsynCtrl()
ctrl.service = 'helloworld' 
ctrl.msg_id = int(time.time())
ctrl.peer_id = 'local_mq'

client = BusClient()

res = client.send(ctrl, 'hello @%s'%time.time()) 


client.destroy()    

