import threading   
from pyzbus import Worker,ZContext
 
class WorkerThread(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)  
    
    def run(self):  
        print 'started'  
        worker = Worker(service='helloworld')
        reply = None 
        
        while True:
            req = worker.recv(reply) 
            reply = req
            
        worker.destroy()

threads = []
for i in range(2):
    t = WorkerThread()
    threads.append(t)
for t in threads:
    t.start()
for t in threads:
    t.join() 
     
 
            

