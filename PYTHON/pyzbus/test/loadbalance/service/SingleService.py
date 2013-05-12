from pyzbus import Worker

worker = Worker(service='MyService', host='127.0.0.1', port=15555)

print 'Python Simple Worker Running...'

while True:
    msg = worker.recv()  
    print msg
    worker.send(['this is from python loadbalancer'])
