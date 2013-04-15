from pyzbus import BusClient

client = BusClient()



print client.publish('pubsub', '', 'topic1', 'hello world') 

client.destroy()    

