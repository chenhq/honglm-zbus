from pyzbus import BusClient

client = BusClient(host='42.120.17.176', port=15555)

print client.request(service='MyService', token='', message=['hello', 'world'])

client.destroy()    

