from pyzbus import BusClient

client = BusClient(host='127.0.0.1', port=15555)

print client.publish('MyPubSub', '', 'topic1', ['frame1', 'frame2']) 

client.destroy()    

