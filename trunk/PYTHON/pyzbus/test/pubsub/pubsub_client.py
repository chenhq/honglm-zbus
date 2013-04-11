from pyzbus import BusClient

client = BusClient()

print client.publish('pubsub', '', 'hello world') 

client.destroy()    

