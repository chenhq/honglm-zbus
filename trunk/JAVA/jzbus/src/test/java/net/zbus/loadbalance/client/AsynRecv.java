package net.zbus.loadbalance.client;

import net.zbus.BusClient;
import net.zbus.BusClientFactory;
import net.zbus.ConnectionConfig;
import net.zbus.Pool;
import net.zbus.PoolConfig;
import net.zbus.ZMsg;

public class AsynRecv {

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
		
		//3)返回链接
		pool.returnResource(client);
		pool.destroy();
	}

}
