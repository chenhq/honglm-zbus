package net.zbus.pool;

import net.zbus.ServiceHandler;
import net.zbus.Worker;
import net.zbus.WorkerHandler;
import net.zbus.WorkerPool;
import net.zbus.WorkerPoolConfig;
import net.zbus.ZMsg;

public class PoolWorker {
 
	public static void main(String[] args) { 
		WorkerPoolConfig cfg = new WorkerPoolConfig();
		cfg.setService("helloworld");
		cfg.setBrokers(new String[]{"127.0.0.1:15555","172.24.180.27:15555"});
		
		WorkerPool wc = new WorkerPool(cfg); 
		 
		wc.run(2, new ServiceHandler() { 			
			@Override
			public ZMsg handleRequest(ZMsg request) {
				request.dump(); 
				return request;
			} 
		});
	}

	public static void main2(String[] args) { 
		WorkerPoolConfig cfg = new WorkerPoolConfig();
		cfg.setService("helloworld"); 
		
		WorkerPool wc = new WorkerPool(cfg); 
		wc.run(2, new WorkerHandler() { 
			@Override
			public void handle(Worker worker) {
				while(true){
					ZMsg request = worker.recv();
					if(request == null) break;
					worker.send(request); 
				}
			}
		});
	}
}
