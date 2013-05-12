# -*- coding: utf-8 -*-
from pyzbus import WorkerPool, Remote, JsonServiceHandler

class MyService(object): 
    
    @Remote()
    def echo(self, ping):
        return ping
    
    @Remote('user')
    def user(self, username):
        return {'Name': username, 'Addr': u'中文'}
    
    @Remote()
    def plus(self, a, b):
        return a + b 

pool = WorkerPool(service = 'MyRpc', broker=['127.0.0.1:15555'])
print 'Python Rpc running...'
pool.run(2, JsonServiceHandler(MyService()))
