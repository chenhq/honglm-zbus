from pyzbus import ServiceHandler, WorkerPool

class EchoServiceHandler(ServiceHandler):     
    def handle_request(self, msg):   
        return ['From Python Pooled Service']
 
    
pool = WorkerPool(service='MyService', broker=['127.0.0.1:15555', '42.120.17.176:15555'])

thread_count = 2
print 'Python MyService(%d) running...'%thread_count
pool.run(thread_count, EchoServiceHandler() )

