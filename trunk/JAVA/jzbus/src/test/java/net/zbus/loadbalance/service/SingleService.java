package net.zbus.loadbalance.service;

import net.zbus.Connection;
import net.zbus.ConnectionConfig;
import net.zbus.Worker;
import net.zbus.WorkerConfig;
import net.zbus.ZMsg;

public class SingleService {
 
	public static void main(String[] args) { 
		//1) 创建连接到ZBUS总线
		ConnectionConfig connCfg = new ConnectionConfig();  
		connCfg.setHost("127.0.0.1");
		connCfg.setPort(15555);
		Connection conn = new Connection(connCfg);
		
		//2) 注册服务（Service1）
		WorkerConfig workerCfg = new WorkerConfig();
		workerCfg.setService("MyService");
		workerCfg.setMode(Worker.MODE_LB); //负载均衡模式
		Worker worker = new Worker(conn, workerCfg);
		
		System.out.format("Java Simple Worker Running...\n");
		
		//3) 服务业务逻辑循环体（等待ZBUS总线分发请求，处理请求，返回请求结果）
		while(true){
			try{
				//3.1) 等待ZBUS总线分发请求消息
				ZMsg request = worker.recv();
				
				//3.2) 业务逻辑处理请求消息
				request.dump(); 
				
				//3.3) 返回处理后的消息
				ZMsg response = new ZMsg();
				response.pushBack("this is from java loadbalancer");
				worker.send(response); 
			}catch (Exception e) {
				e.printStackTrace();
				break;
			}
		}
		
		//4) 清除连接相关
		worker.destroy();
		conn.destroy();
	}

}
