from pyzbus import ServiceHandler, WorkerPool

class EchoServiceHandler(ServiceHandler):     
    def handle_request(self, msg):  
        return msg
 
    
pool = WorkerPool(service='helloworld')
pool.run(4, EchoServiceHandler() )

