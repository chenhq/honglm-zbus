package net.zbus.loadbalance.client;

import net.zbus.BusClient;
import net.zbus.BusClientFactory;
import net.zbus.BusClientPool;
import net.zbus.BusClientPoolConfig;
import net.zbus.Pool;
import net.zbus.ConnectionConfig;
import net.zbus.PoolConfig;
import net.zbus.ZMsg;

public class SyncClient {

	public static void main(String[] args) throws Exception {
		//1) 创建连接  
		BusClientPoolConfig config = new BusClientPoolConfig();
		config.setHost("127.0.0.1");
		config.setPort(15555);
		
		BusClientPool pool = new BusClientPool(config);
		 
		BusClient client = pool.borrowResource();     
		
		//2) 组装消息（消息帧数组）
		ZMsg request = new ZMsg();
		request.pushBack("Frame1");     //消息帧1 -- 字符类型
		request.pushBack(new byte[10]); //消息帧2 -- 二进制串
		request.pushBack("request from JAVA"); //消息帧3 -- 字符类型
		
		//3) 向ZBUS总线发送请求
		ZMsg result = client.request("MyService"/*服务标识*/, ""/*安全码*/, request, 2500/*超时时间（ms）*/);
		result.dump();
		
		//4) 销毁客户端
		pool.returnResource(client);
		 
		pool.destroy();
	}

}
