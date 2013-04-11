import threading  
from pyzbus import BusClient 
import time
class ClientThread(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self) 
        
    
    def run(self):   
        for i in range(10000):
            client = BusClient()
            client.destroy()
            time.sleep(0.01)

threads = []
for i in range(16):
    t = ClientThread()
    threads.append(t)
for t in threads:
    t.start()
for t in threads:
    t.join() 
     
 
            

