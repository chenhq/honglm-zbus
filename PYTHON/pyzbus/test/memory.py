import threading  
from pyzbus import BusClient  
class ClientThread(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self) 
        
    
    def run(self):   
        client = BusClient()
        for i in range(1000): #@UnusedVariable
            res = client.request('helloworld', '', bytearray(1024*1024))
            print len(res[0])

threads = []
for i in range(16):
    t = ClientThread()
    threads.append(t)
for t in threads:
    t.start()
for t in threads:
    t.join() 
     
 
            

