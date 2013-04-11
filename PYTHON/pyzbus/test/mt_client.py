import threading  
from pyzbus import BusClient

client = BusClient()
class ClientThread(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self) 
        
    
    def run(self):   
        
        body = 'a'*10 
        for i in range(1000000): #@UnusedVariable
            try:
                thread = threading.current_thread()
                handle = client.handle
                res = client.request('helloworld','', body)
                print thread, handle, res 
            except Exception, e:
                print e
        
        client.destroy()

threads = []
for i in range(32):
    t = ClientThread()
    threads.append(t)
for t in threads:
    t.start()
for t in threads:
    t.join() 
     
 
            

