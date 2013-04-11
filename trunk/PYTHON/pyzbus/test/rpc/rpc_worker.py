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

handler = JsonServiceHandler(MyService())
pool = WorkerPool(service = 'rpc')
pool.run(2, handler)
