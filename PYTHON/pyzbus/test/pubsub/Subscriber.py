from pyzbus import Worker, MODE_PUBSUB

worker = Worker(service='MyPubSub', mode=MODE_PUBSUB, host='127.0.0.1', port=15555)
worker.subscribe('topic1')
print 'Python Subscribe %s'%'topic1'
while True:
    print worker.recv() 

