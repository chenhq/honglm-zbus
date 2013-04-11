from pyzbus import Worker

worker = Worker(service='helloworld') 
while True:
    msg = worker.recv()   
    worker.send(msg)  