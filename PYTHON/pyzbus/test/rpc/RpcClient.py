# -*- coding: utf-8 -*-
from pyzbus import Rpc  
rpc = Rpc(service = 'MyRpc', host='127.0.0.1', port=15555)
 
print rpc.plus(234,134)

rpc.destroy()

