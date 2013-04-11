import json
import inspect 
from wrapper import ServiceHandler, BusClient 

class Rpc(object):       
    def __init__(self, **kwargs):  
        self.service = kwargs.get('service', None)
        assert self.service 
        
        client = kwargs.get('client', None)  
        self.token = kwargs.get('token', '')
        self.method = kwargs.get('method', None)
        self.timeout = kwargs.get('timeout', 2500)
        
        if client is None:   
            self.client = BusClient(**kwargs)
            self.own_client = True
        else:
            self.client = client
            self.own_client = False     

            
    def destroy(self):
        if self.client and self.own_client:
            self.client.destroy()
     

    def __getattr__(self, name):
        return Rpc(client = self.client, 
                   service = self.service, 
                   token = self.token,
                   method = name,
                   timeout = self.timeout)
    
    def rpc_json(self, args):
        req = {'method': self.method, 'params': args, 'id': 'jsonrpc'}
        return self.client.request(self.service, self.token, json.dumps(req), timeout=self.timeout)

    def __call__(self, *args): 
        res = self.rpc_json(args) 
        if res is None:
            raise Exception('request timeout')
        assert len(res) == 2
        if res[0] != '200':
            raise Exception(res[1])
        result = json.loads(res[1])
        if 'error' in result and result['error'] != None:
            raise Exception(result['error'])
        
        if 'result' in result:
            return result['result']
        else:
            return None
    
    
    def invoke(self, method, *args):
        self.method = method
        return self.__call__(*args)


def Remote( _id = None ):
    def func(fn):
        fn.remote_id = _id or fn.__name__ 
        return fn
    return func 
 
class JsonServiceHandler(ServiceHandler):
    def __init__(self, service):
        if not isinstance(service, (list,tuple)):
            service_instances = [ service ]
        else:
            service_instances = service 
            
        self.methods = {}
        for instance in service_instances:
            self.init_command_table(instance)
    
    def init_command_table(self, service):  
        methods = inspect.getmembers(service, predicate=inspect.ismethod)
        for method in methods:
            if hasattr(method[1], 'remote_id'):
                remote_id = getattr(method[1], 'remote_id')
                if remote_id in self.methods:
                    print '%s duplicated'%remote_id
                self.methods[remote_id] = method[1]
    
    def handle_request(self, msg):    
        ret = ['200'] 
        try:
            for part in msg:
                ret.append(self.handle_json_request(part))
        except:
            ret = ['500','internal server error']
        return  ret
    
    def handle_json_request(self, json_str):
        error = None
        result = None
        req_id = ''
        try:
            req = json.loads(json_str) 
        except Exception, e:
            error = Exception('json format error: %s'%str(e))
            
        if error is None:
            try:
                req_id = req['id']
                method_name = req['method']
                args = req['params']
            except:
                error = Exception('parameter error: %s'%json_str)
        
        if error is None:
            if method_name not in self.methods:
                error = Exception('%s method not found'%method_name) 
            else:
                method = self.methods[method_name]
        
        if error is None:
            try:
                result = method(*args)
            except Exception, e:
                error = e
        
        #return result
        if error is not None:
            error = str(error)
            result = None
        try:
            data = json.dumps({'id': req_id, 'result': result, 'error': error})
        except:
            error = 'Result Object Not Serializable'
            data = json.dumps({'result': None, 'id': req_id,'error': error })
        
        return data 

__all__ = [Remote, Rpc,  JsonServiceHandler]