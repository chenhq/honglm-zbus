from pyzbus import BusClient
import time

client = BusClient(host='127.0.0.1', port=15555)

message = ['hello @%s'%time.time()]

result = client.broadcast('MyBroadcast','', message) 
print result

client.destroy()    

