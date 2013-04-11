#!/usr/bin/python
#coding=utf8

import tornado.web
import tornado.ioloop
import tornado.httpserver 
from tornado.options import define, options

import os,socket 

this_ip = [ip for ip in socket.gethostbyname_ex(socket.gethostname())[2] if not ip.startswith("127.")][:1][0]        

define("http_port", default=8080, type=int)  
 
class BaseHandler(tornado.web.RequestHandler):
    pass

class MainHandler(BaseHandler): 
    def get(self):   
        self.render('index.html') 

class BrokerHandler(BaseHandler): 
    def get(self):   
        self.render('broker.html')   
        
class CHandler(BaseHandler): 
    def get(self):  
        self.render('c_cpp.html')          
class JAVAHandler(BaseHandler): 
    def get(self):   
        self.render('java.html')  
class PythonHandler(BaseHandler): 
    def get(self):   
        self.render('python.html') 
class ClrHandler(BaseHandler): 
    def get(self):   
        self.render('clr.html')                 
handlers = [
    (r'/', MainHandler), 
    (r'/broker', BrokerHandler), 
    (r'/c_cpp', CHandler), 
    (r'/java', JAVAHandler), 
    (r'/python', PythonHandler), 
    (r'/clr', ClrHandler), 
]         

class Application(tornado.web.Application):
    def __init__(self):  
        settings = dict(
            debug = True,
            cookie_secret="32oETzKXQAGaYdkL5gEmGeJJFuYh7EQnp2XdTP1o/Vo=",
            static_path = os.path.join(os.path.dirname(__file__), 'static'),
            template_path = os.path.join(os.path.dirname(__file__),'template'),
            static_prefix = 'http://%s:%d/static'%(this_ip, options.http_port)
        )
        
        tornado.web.Application.__init__(self, handlers, **settings)  
         
server = tornado.httpserver.HTTPServer(Application())
server.listen(options.http_port)
tornado.ioloop.IOLoop.instance().start()  
