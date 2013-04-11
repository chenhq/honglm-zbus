#!/apps/lab/python
#coding=utf8

import tornado.web
import tornado.ioloop
import tornado.httpserver 
from tornado.options import define, options

import os,socket

import pyzbus 

this_ip = [ip for ip in socket.gethostbyname_ex(socket.gethostname())[2] if not ip.startswith("127.")][:1][0]        

define("http_port", default=8080, type=int)  

def type_str(t):
    if t == '1':
        return '负载均衡'
    if t == '2':
        return '消息队列'
    if t == '3':
        return '发布订阅'
    return 'UNKHOWN'


class BaseHandler(tornado.web.RequestHandler):
    
    @property
    def buses(self):
        return self.application.buses 
    
    def get_login_url(self):
        return u'/login'
    
    def get_current_user(self):
        broker = self.get_cookie('broker')
        return broker
 
    def zbus_command(self, cmd):
        broker = self.get_cookie('broker')
        token = self.get_cookie('token', '')
        if broker not in self.buses: 
            self.clear_cookie('broker')
            self.clear_cookie('token')
            
            
        monitor_client = self.buses[broker]    
        return monitor_client.monitor(token, cmd, 2500) 
    
class LoginHandler(BaseHandler): 
    def get(self):
        self.render("login.html", server_ip=this_ip, next=self.get_argument("next","/"))
    
    def post(self):
        broker = self.get_argument('broker')
        token = self.get_argument('token', '')
        next = self.get_argument('next', u'/')
        parts = broker.split(':')
        monitor_client = pyzbus.BusClient(host=parts[0], port=parts[1])
        self.buses[broker] =  monitor_client 
        self.set_cookie('broker', broker)
        self.set_cookie('token', token) 
        self.redirect(next)


class LogoutHandler(BaseHandler): 
    def get(self):
        self.clear_cookie('broker')
        self.clear_cookie('token')
        self.redirect('/login')
        
        
class RegisterHandler(BaseHandler): 
    @tornado.web.authenticated
    def get(self):
        self.render('register.html')
    @tornado.web.authenticated    
    def post(self):
        svc_id = str(self.get_argument('name'))
        token = str(self.get_argument('token',''))
        type = str(self.get_argument('type'))
        
        if svc_id is None or svc_id == '':
            self.write('service id not provided')
        
        res = self.zbus_command(['reg', svc_id, token, type])
        if res[0] == '500':
            self.write(res[1])
        else:
            self.redirect('/')

class ClearHandler(BaseHandler): 
    @tornado.web.authenticated
    def get(self):
        svc_id = str(self.get_argument('svc')) 
        if svc_id is None or svc_id == '':
            self.write('service id not provided')
        
        res = self.zbus_command(['clear', svc_id])
        if res[0] == '500':
            self.write(res[1])
        else:
            self.redirect('/')  
            
class DelHandler(BaseHandler): 
    @tornado.web.authenticated
    def get(self):
        svc_id = str(self.get_argument('svc')) 
        if svc_id is None or svc_id == '':
            self.write('service id not provided')
        
        res = self.zbus_command(['del', svc_id])
        if res[0] == '500':
            self.write(res[1])
        else:
            self.redirect('/')          
        
class DashboardHandler(BaseHandler): 
    
    @tornado.web.authenticated
    def get(self):  
        res = self.zbus_command(['ls'])  
        broker = self.get_cookie('broker')
        if res[0] == '200':
            res = res[1:]
            svcs = []
            for svc_str in res:
                blocks = svc_str.split('\t')
                svc = {
                    'name': blocks[0],
                    'type': type_str(blocks[1]),
                    'token': blocks[2],
                    'mq_size': blocks[3],
                    'requests': blocks[4],
                    'idle_workers': blocks[5],
                    'total_workers': blocks[6],
                    'last_serve': blocks[7]
                }
                svcs.append(svc) 
            self.render('index.html', svcs = svcs, broker=broker)
        else: 
            self.write(res[0])
            
    
        
handlers = [
    (r'/', DashboardHandler), 
    (r'/login', LoginHandler), 
    (r'/logout', LogoutHandler), 
    (r'/register', RegisterHandler), 
    (r'/clear', ClearHandler), 
    (r'/del', DelHandler), 
]         

class Application(tornado.web.Application):
    def __init__(self):  
        settings = dict(
            debug = False,
            cookie_secret="32oETzKXQAGaYdkL5gEmGeJJFuYh7EQnp2XdTP1o/Vo=",
            static_path = os.path.join(os.path.dirname(__file__), 'static'),
            template_path = os.path.join(os.path.dirname(__file__),'template'),
            static_prefix = 'http://%s:%d/static'%(this_ip, options.http_port), 
        )
        
        tornado.web.Application.__init__(self, handlers, **settings)  
        self.buses = {} #bus_ip_port => monitor_client
         
server = tornado.httpserver.HTTPServer(Application())
server.listen(options.http_port)
tornado.ioloop.IOLoop.instance().start()  
