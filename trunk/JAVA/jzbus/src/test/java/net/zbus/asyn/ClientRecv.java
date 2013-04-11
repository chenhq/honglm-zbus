package net.zbus.asyn;

import net.zbus.BusClient;
import net.zbus.ConnectionConfig;
import net.zbus.ZMsg;

public class ClientRecv {

	public static void main(String[] args) {
		ConnectionConfig config = new ConnectionConfig();
		config.setId("local_mq");
		BusClient client = new BusClient(config);

		while (true) {
			try {
				ZMsg res = client.recv(2000);
				res.dump();
			} catch (Exception e) {
				e.printStackTrace();
				break;
			}
		}

		client.destroy();
	}

}
