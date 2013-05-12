from pyzbus import BusClient

client = BusClient(host='127.0.0.1', port=15555)


print client.request(service='MyService', token='', message=['hello', 'world'])


client.destroy()    

