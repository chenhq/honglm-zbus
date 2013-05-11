package net.zbus.loadbalance.client;

import net.zbus.BusClient;
import net.zbus.ConnectionConfig;
import net.zbus.ZMsg;

public class AsynRecv {

	public static void main(String[] args) {
		//1) 创建连接
		ConnectionConfig config = new ConnectionConfig(); 
		config.setHost("127.0.0.1");
		config.setPort(15555);
		config.setId("local_mq"); //指定PeerID，与发送方协商
		BusClient client = new BusClient(config); 

		//2） 异步处理消息
		while (true) {
			try {
				//recv参数指定发送心跳时间间隔，（防火墙切断链接）
				ZMsg res = client.recv(2000);
				res.dump();
			} catch (Exception e) {
				e.printStackTrace();
				break;
			}
		}
		
		//3)销毁连接
		client.destroy();
	}

}
