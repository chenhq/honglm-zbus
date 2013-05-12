package net.zbus.loadbalance.client;

import net.zbus.BusClient;
import net.zbus.ConnectionConfig;
import net.zbus.ZMsg;

public class SyncClient {

	public static void main(String[] args) {
		//1) 创建连接
		ConnectionConfig config = new ConnectionConfig(); 
		config.setHost("127.0.0.1");
		config.setPort(15555);
		BusClient client = new BusClient(config);
		
		//2) 组装消息（消息帧数组）
		ZMsg request = new ZMsg();
		request.pushBack("Frame1");     //消息帧1 -- 字符类型
		request.pushBack(new byte[10]); //消息帧2 -- 二进制串
		
		//3) 向ZBUS总线发送请求
		ZMsg result = client.request("MyService"/*服务标识*/, ""/*安全码*/, request, 2500/*超时时间（ms）*/);
		result.dump();
		
		//4) 销毁客户端
		client.destroy();
	}

}
