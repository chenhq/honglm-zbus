from pyzbus import BusClient

client = BusClient(id = 'local_mq')
while True:
    res = client.recv(probe_interval=100) #
    print res

client.destroy()    

