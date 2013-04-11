package net.zbus.monitor;

import java.util.Iterator;

import net.zbus.BusClient;
import net.zbus.ConnectionConfig;
import net.zbus.ZMsg;

public class MonitorClient {

	public static void main(String[] args) {
		ConnectionConfig config = new ConnectionConfig();
		// config.setVerbose(true); 
		BusClient client = new BusClient(config);

		ZMsg msg = new ZMsg();
		msg.pushBack("ls"); 
		msg = client.monitor("", msg, 2500);
		 
		Iterator<byte[]> it = msg.iterator();
		while(it.hasNext()){
			byte[] data = it.next();
			System.out.println(new String(data));
		}
		
		client.destroy();
	}

}
