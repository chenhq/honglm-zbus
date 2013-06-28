package net.zbus.monitor;

import java.util.Iterator;

import net.zbus.BusClient;
import net.zbus.BusClientFactory;
import net.zbus.ConnectionConfig;
import net.zbus.Pool;
import net.zbus.PoolConfig;
import net.zbus.ZMsg;

public class MonitorClient {

	public static void main(String[] args) {
		ConnectionConfig connCfg = new ConnectionConfig();
		connCfg.setHost("127.0.0.1");
		connCfg.setPort(15555); 
		BusClientFactory factory = new BusClientFactory(connCfg);
		
		
		PoolConfig config = new PoolConfig(); 
		config.setMaxActive(2); 
		Pool<BusClient> pool = new Pool<BusClient>(factory, config);
		
		BusClient client = pool.borrowResource();  

		ZMsg msg = new ZMsg();
		msg.pushBack("ls"); 
		msg = client.monitor("", msg, 2500);
		 
		Iterator<byte[]> it = msg.iterator();
		while(it.hasNext()){
			byte[] data = it.next();
			System.out.println(new String(data));
		}
		
		pool.returnResource(client);
		pool.destroy();
	}

}
