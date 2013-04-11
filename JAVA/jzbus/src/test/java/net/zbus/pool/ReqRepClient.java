package net.zbus.pool;

import net.zbus.BusClient;
import net.zbus.ConnectionConfig;
import net.zbus.ZMsg;

public class ReqRepClient {

	public static void main(String[] args) {
		ConnectionConfig config = new ConnectionConfig();
		// config.setVerbose(true);
		BusClient client = new BusClient(config);

		ZMsg msg = new ZMsg();
		msg.pushBack("hello");
		msg = client.request("helloworld", "", msg, 2500);
		msg.dump();
		
		client.destroy();
	}

}
