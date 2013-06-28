package net.zbus.broadcast;

import net.zbus.BusClient;
import net.zbus.BusClientFactory;
import net.zbus.ConnectionConfig;
import net.zbus.Pool;
import net.zbus.PoolConfig;
import net.zbus.ZMsg;

public class Broadcast {

	public static void main(String[] args) {
		//1) 创建连接
		ConnectionConfig connCfg = new ConnectionConfig();
		connCfg.setHost("127.0.0.1");
		connCfg.setPort(15555); 
		BusClientFactory factory = new BusClientFactory(connCfg);
		
		
		PoolConfig config = new PoolConfig(); 
		config.setMaxActive(2); 
		Pool<BusClient> pool = new Pool<BusClient>(factory, config);
		
		BusClient client = pool.borrowResource();  
		
		//2) 组装消息（消息帧数组）
		ZMsg request = new ZMsg();
		request.pushBack("Frame1");     //消息帧1 -- 字符类型
		request.pushBack(new byte[10]); //消息帧2 -- 二进制串
		
		//3) 向ZBUS总线发送请求
		boolean res = client.broadcast("MyBroadcast"/*服务标识*/, ""/*安全码*/, request, 2500/*超时时间（ms）*/);
		System.out.println("broadcast result: "+res);
		
		//4) 销毁客户端 
		pool.returnResource(client);
				
		pool.destroy();
	}

}
