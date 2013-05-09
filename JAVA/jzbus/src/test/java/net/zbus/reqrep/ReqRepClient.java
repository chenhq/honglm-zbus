package net.zbus.reqrep;

import net.zbus.BusClient;
import net.zbus.ConnectionConfig;
import net.zbus.ZMsg;

public class ReqRepClient {

	public static void main(String[] args) {
		ConnectionConfig config = new ConnectionConfig();
		//config.setVerbose(true);
		// config.setHost("192.168.1.10");
		BusClient client = new BusClient(config);
		for (int i = 0; i < 1; i++) {
			ZMsg msg = new ZMsg();
			msg.pushBack("helloworld");
			msg = client.request("echo", "", msg, 2500);
			msg.dump();
		}

		client.destroy();
	}

}
