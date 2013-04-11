from pyzbus import BusClient

client = BusClient()

print client.request(service='helloworld', token='', message='hello')

client.destroy()    

