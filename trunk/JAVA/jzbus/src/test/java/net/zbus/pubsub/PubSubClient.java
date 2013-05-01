package net.zbus.pubsub;

import net.zbus.BusClient;
import net.zbus.ConnectionConfig;
import net.zbus.ZMsg;

public class PubSubClient {
 
	public static void main(String[] args) { 
		
		ConnectionConfig connCfg = new ConnectionConfig(); 
		BusClient client = new BusClient(connCfg);
		
		for(int i=0;i<1;i++){
		ZMsg message = new ZMsg();
		message.pushBack("topic2");
		message.pushBack("contentxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
		boolean status = client.publish("pubsub","","topic1", message, 2500);
		
		System.out.println(status);
		}
		client.destroy();
		
	}

}
