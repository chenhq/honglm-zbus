package net.zbus.broadcast;

import net.zbus.Connection;
import net.zbus.ConnectionConfig;
import net.zbus.Worker;
import net.zbus.WorkerConfig;
import net.zbus.ZMsg;

public class Receiver {
 
	public static void main(String[] args) { 
		//1) 创建连接到ZBUS总线
		ConnectionConfig connCfg = new ConnectionConfig();  
		connCfg.setHost("127.0.0.1");
		connCfg.setPort(15555);
		Connection conn = new Connection(connCfg);
		
		//2) 注册服务（MyBroadcast）
		WorkerConfig workerCfg = new WorkerConfig();
		workerCfg.setService("MyBroadcast");
		workerCfg.setMode(Worker.MODE_BC); //广播模式
		Worker worker = new Worker(conn, workerCfg);
		
		System.out.format("Java Simple Broadcast Worker Running...\n");
		 
		while(true){
			try{ 
				ZMsg request = worker.recv(); 
				request.dump(); 
				 
			}catch (Exception e) {
				e.printStackTrace();
				break;
			}
		}
		
		//3) 清除连接相关
		worker.destroy();
		conn.destroy();
	}

}
