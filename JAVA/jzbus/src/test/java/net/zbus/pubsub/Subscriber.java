package net.zbus.pubsub;

import net.zbus.SafeConnection;
import net.zbus.ConnectionConfig;
import net.zbus.Worker;
import net.zbus.WorkerConfig;
import net.zbus.ZMsg;

public class Subscriber {
 
	public static void main(String[] args) { 
		//1) 创建连接到ZBUS总线
		ConnectionConfig connCfg = new ConnectionConfig();
		SafeConnection connection = new SafeConnection(connCfg);
	
		//2) 注册发布订阅服务（MyPubSub）
		WorkerConfig workerCfg = new WorkerConfig();
		workerCfg.setService("MyPubSub");
		workerCfg.setMode(WorkerConfig.MODE_PUBSUB);
		Worker worker = new Worker(connection, workerCfg);
		
		//3) 订阅主题，可多主题订阅
		String topic = "topic1";
		worker.subscribe(topic);
		
		System.out.format("Subscriber on '%s', waiting for message...\n", topic);
		while(true){
			try{
				//等待主题消息
				ZMsg msg = worker.recv();
				
				msg.dump();
			}catch (Exception e) {
				e.printStackTrace();
				break;
			}
		}
		
		//4) 销毁连接相关
		worker.destroy();
		connection.destroy();
	}

}
