from pyzbus import Worker, MODE_PUBSUB

worker = Worker(service='pubsub', mode=MODE_PUBSUB)
worker.subscribe('topic1')
while True:
    print worker.recv() 

