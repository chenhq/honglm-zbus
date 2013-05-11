package net.zbus.loadbalance.service;

import net.zbus.ServiceHandler;
import net.zbus.Worker;
import net.zbus.WorkerPool;
import net.zbus.WorkerPoolConfig;
import net.zbus.ZMsg;

public class PooledService {
 
	public static void main(String[] args) {
		//1) 配置服务信息
		WorkerPoolConfig cfg = new WorkerPoolConfig();
		cfg.setService("PooledService1");
		cfg.setMode(Worker.MODE_LB);
		cfg.setBrokers(new String[]{"127.0.0.1:15555","42.120.17.176:15555"}); //总线地址（多总线注册）
		
		int threadCount = 2;
		WorkerPool wc = new WorkerPool(cfg); 
		System.out.format("Pooled Workers(%d) Run...\n", threadCount);
		
		//2)以指定的线程数运行服务处理业务逻辑代码
		wc.run(threadCount, new ServiceHandler() { 			
			@Override
			public ZMsg handleRequest(ZMsg request) {
				request.dump(); 
				ZMsg result = new ZMsg();
				result.pushBack("this is from pooled java worker");
				return result;
			} 
		});
	}
}
