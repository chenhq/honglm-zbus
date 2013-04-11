# -*- coding: utf-8 -*-
from pyzbus import Rpc  
rpc = Rpc(service = 'rpc')# host='172.24.180.27')

#user = {'name': u'中国', 'addr': 'address'}
print rpc.user('hong')

rpc.destroy()

