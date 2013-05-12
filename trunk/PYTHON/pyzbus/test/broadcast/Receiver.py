from pyzbus import Worker, MODE_BC

worker = Worker(service='MyBroadcast', mode= MODE_BC, host='127.0.0.1', port=15555)

print 'Python Simple Broadcast Worker Running...'

while True:
    msg = worker.recv()  
    print msg 
