#encoding=utf8
from pyzbus import BusClient

#1)创建接收客户端，id是接收端的唯一标识
client = BusClient(host='127.0.0.1', port=15555, id = 'local_mq') #id required to receive

while True:
    #probe防止链接给网络设备切断，发心跳频率为2s
    res = client.recv(probe_interval=2000) #
    
    print res

client.destroy()    

