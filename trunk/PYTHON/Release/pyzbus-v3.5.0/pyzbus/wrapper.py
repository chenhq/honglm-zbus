'''
/************************************************************************
 *  Copyright (c) 2012-2013 HONG LEIMING.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the
 * following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 ***************************************************************************/
'''
import _pyzbus 
import threading  

g_zctx_instance = None
g_zctx_init_lock = threading.Lock()

def zlog_use_stdout():
    '''
    global logging use stdout
    '''
    _pyzbus.zlog_use_stdout()
def zlog_use_file(file_path):
    '''
    global logging use file, file_path specify file path in which log files roll over day
    '''
    _pyzbus.zlog_use_file(file_path)

class ZContext(object):
    def __init__(self, io_threads=1):
        '''
        io_threads specify how many threads of underlying zeromq send/recv routine
        '''
        self._handle = _pyzbus.zctx_new(io_threads) 
        
    @property
    def handle(self):
        '''
        PyCObject handler of the underlying C pointer of the zeromq context
        '''
        return self._handle
            
    @classmethod
    def instance(cls, io_threads=1): 
        '''
        thread-safe mode to get global ZContext instance, io_threads works only first time of creation
        '''
        global g_zctx_instance
        g_zctx_init_lock.acquire()
        if g_zctx_instance is None: 
            g_zctx_instance = cls(io_threads)
        g_zctx_init_lock.release()
        
        return g_zctx_instance
    
    def destroy(self):
        '''
        destroy underlying C pointer of zeromq context
        '''
        if self._handle:
            if _pyzbus: #global instance may destroyed after 
                _pyzbus.zctx_destroy(self._handle)
  

 
         
class Connection(threading.local):  
    def __init__(self, **kwargs): 
        '''
        ctx ZContext: zeromq context instance, optional
        host String: host of ZBUS broker, default to localhost, optional
        port Integer: port of ZBUS broker, default to 15555, optional
        verbose Boolean: show internal request message, default to False(0), optional
        '''
        zctx = kwargs.get('ctx', ZContext.instance())
        assert isinstance(zctx, ZContext)
       
        args = {}
        args['ctx'] = zctx.handle
        args['host'] = kwargs.get('host', '127.0.0.1')
        args['port'] = int(kwargs.get('port', 15555)) 
        args['verbose'] = int(kwargs.get('verbose', 0))
        args['id'] = kwargs.get('id','')
        
        self._handle = _pyzbus.zbusconn_new(**args)
    
    def recv(self, timeout=2500):
        return _pyzbus.zbusconn_recv(self._handle, timeout)
    
    def send(self, message):
        if not isinstance(message, (list, tuple)):
            message = [message]  
        msg = []
        for frame in message:
            msg.append(str(frame)) 
        return _pyzbus.zbusconn_send(self._handle, msg)
    
    def route(self, sock_id, message):
        if not isinstance(message, (list, tuple)):
            message = [message]  
        msg = []
        for frame in message:
            msg.append(str(frame)) 
        return _pyzbus.zbusconn_route(self._handle, str(sock_id), msg)
    
    def probe(self, timeout=2500):
        return _pyzbus.zbusconn_probe(self._handle, timeout)
    
    def destroy(self):
        '''
        destroy underlying C handler of this BusClient instance
        '''
        if self._handle:
            _pyzbus.zbusconn_destroy(self._handle)
    
    @property   
    def handle(self):
        return self._handle

MODE_LB = '1'
MODE_PUBSUB = '2'
MODE_BC = '3'

class AsynCtrl(object):
    def __init__(self, **kwargs):
        self.service = kwargs.get('service', None)
        self.token = kwargs.get('token', '')
        self.timeout = int(kwargs.get('timeout', 2500))
        self.msg_id = kwargs.get('msg_id', '')
        self.peer_id = kwargs.get('peer_id', '') 

class BusClient(object):
    def __init__(self, **kwargs): 
        if 'connection' in kwargs:
            conn = kwargs['connection']
            if isinstance(conn, Connection):
                self.connection = conn
                self.own_connection = False
            else:
                raise Exception('connection type should be Connection')
        else:
            self.connection = Connection(**kwargs)
            self.own_connection = True
    
    def request(self, service, token, message, timeout=2500):
        if not isinstance(message, (list, tuple)):
            message = [message]  
        msg = []
        for frame in message:
            msg.append(str(frame))
        return _pyzbus.zbuscli_request(self.connection.handle, service, token, msg, timeout)

    def send(self, asyn_ctrl, message):
        assert isinstance(asyn_ctrl, AsynCtrl) 
        if not isinstance(message, (list, tuple)):
            message = [message]  
        msg = []
        for frame in message:
            msg.append(str(frame))    
        return _pyzbus.zbuscli_send(conn=self.connection.handle, service=asyn_ctrl.service, 
                                    token=asyn_ctrl.token, peer_id=asyn_ctrl.peer_id, 
                                    msg_id=str(asyn_ctrl.msg_id),msg = msg, timeout=asyn_ctrl.timeout)

    def publish(self, service, token, topic, message, timeout=2500):
        ctrl = AsynCtrl(service=service, token=token, timeout=2500)
        if not isinstance(message, (list, tuple)):
            message = [message]
        message.insert(0, topic)
        res = self.send(ctrl, message) 
        if len(res)>0 and res[0] == '200': 
            return True
        return False
        
    def recv(self, probe_interval):
        return _pyzbus.zbuscli_recv(self.connection.handle, probe_interval)
    
    def monitor(self, token, message, timeout=2500):
        if not isinstance(message, (list, tuple)):
            message = [message]  
        msg = []
        for frame in message:
            msg.append(str(frame))
        return _pyzbus.zbusmon_request(self.connection.handle, token, msg, timeout)

        
    def destroy(self):
        if self.own_connection and self.connection:
            self.connection.destroy()
 
class Worker(object):  
    def __init__(self, **kwargs):
        if 'service' not in kwargs:
            raise Exception('service name required')
        
        args = {}
        self.service        = args['service']        = kwargs.get('service')
        self.register_token = args['register_token'] = kwargs.get('register_token', '')
        self.access_token   = args['access_token']   = kwargs.get('access_token', '') 
        self.mode           = args['mode']           = kwargs.get('mode', MODE_LB)
        if args['mode'] not in(MODE_LB, MODE_PUBSUB, MODE_BC):
            raise Exception('service mode not support')
        
        if 'connection' in kwargs:
            self.connection = kwargs.get('connection')
            assert isinstance(self.connection, Connection)
        else:
            self.connection = Connection(**kwargs)
        
        self._handle = _pyzbus.zbuswrk_new(**args)
        
        self.recv_sock_id = None
        self.recv_msg_id = None
    
    def recv(self): 
        message = _pyzbus.zbuswrk_recv(self.connection.handle, self._handle)
        if not isinstance(message, (list, tuple)):
            message = [message] 
        if len(message) < 2:
            raise Exception('worker message received error')   
        self.recv_sock_id = message[0]
        self.recv_msg_id = message[1]
        return message[2:]
    
    
    def send(self, message):   
        assert self.recv_msg_id is not None
        assert self.recv_sock_id is not None
        assert message is not None
        
        if not isinstance(message, (list, tuple)):
            message = [message] 
        msg = [self.recv_sock_id, self.recv_msg_id]
        for frame in message:
            msg.append(str(frame))
        return _pyzbus.zbuswrk_send(self.connection.handle, self._handle, msg)

    def subscribe(self, topic):
        return _pyzbus.zbuswrk_subscribe(self.connection.handle, self._handle, topic)

    def unsubscribe(self, topic):
        return _pyzbus.zbuswrk_unsubscribe(self.connection.handle, self._handle, topic)
   
    def destroy(self):
        self.connection.destroy()
        _pyzbus.zbuswrk_destroy(self._handle)



class ServiceHandler(object): 
    '''
    simple message handler, callable
    '''
    def __call__(self, req):
        res = self.handle_request(req)    
        if not isinstance(res, (tuple, list)):
            res = [res]  
        return res
          
    def handle_request(self, msg):  
        '''
        msg String/Tuple/List: message(frames) received from ZBUS broker
        '''
        raise Exception('unimplemented')  

class WorkerHandler(object):   
    '''
    worker handler, callable
    '''
    def __call__(self, worker):  
        return self.handle_worker(worker)     
          
    def handle_worker(self, worker): 
        '''
        worker Worker: worker instance to manipulate details
        ''' 
        raise Exception('unimplemented')  
    

class WorkerThread(threading.Thread):
    def __init__(self, worker_kwargs, handler):
        threading.Thread.__init__(self)  
        self.worker_kwargs = worker_kwargs
        self.handler = handler
        if not isinstance(self.handler, (ServiceHandler, WorkerHandler)):
            raise Exception('handler not support')
    
    def run(self):   
        worker = Worker(**self.worker_kwargs)
        if isinstance(self.handler, WorkerHandler):
            self.handler(worker)
        else: 
            while True:
                req = worker.recv()  
                reply = self.handler(req)  
                if reply is not None:
                    worker.send(reply)
        
        worker.destroy()



class WorkerPool(object):   
    def __init__(self, **kwargs):
        '''
        service String: name of this service register to ZBUS broker, required
        mode String: worker mode: one of LoadBalance/BroadCast, optional
        ctx ZContext: zeromq context instance, optional
        broker String/Tuple/List: ZBUS broker(s)--host:port eg. ['127.0.0.1:15555'], optional
        register_token String: register token set by ZBUS broker if required to register new service, optional
        access_token String: access token set by this service to access control of this service, optional
        verbose Boolean: show internal message, default to False(0), optional
        '''
        
        self.service = kwargs.get('service', None) 
        if self.service is None:
            raise Exception('service name required')
        
        self.brokers = kwargs.get('broker', ['127.0.0.1:15555'])#default to local 
        
        if not isinstance(self.brokers, (list,tuple)):
            self.brokers = [self.brokers]   
        
        self.kwargs = kwargs 
   
    def run(self, thread_count, handler):  
        '''
        thread_count Integer: how many threads to run concurrently 
        hanlder ServiceHandler/WorkerHandler: handler instance
        '''
        if not isinstance(handler, (ServiceHandler, WorkerHandler)):
            raise Exception('handler not support')
        own_ctx = False
        ctx = None
        if 'ctx' not in self.kwargs:
            ctx = ZContext(1)
            own_ctx = True
            self.kwargs['ctx'] = ctx
            
        threads = []
        import copy
        for broker in self.brokers:
            parts = broker.split(':')
            assert len(parts) == 2
            kwargs = copy.copy(self.kwargs)
            kwargs['host'] = parts[0]
            kwargs['port'] = int(parts[1])
            
            for i in range(thread_count): #@UnusedVariable
                thread = WorkerThread(kwargs, handler)
                thread.start()
                threads.append(thread)
               
        for thread in threads:
            thread.join()
        
        if own_ctx and ctx:
            ctx.destroy()