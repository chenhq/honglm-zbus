package net.zbus.asyn;

import net.zbus.AsynCtrl;
import net.zbus.BusClient;
import net.zbus.ConnectionConfig;
import net.zbus.ZMsg;

public class ClientSend {
 
	public static void main(String[] args) { 
		ConnectionConfig config = new ConnectionConfig();
		BusClient client = new BusClient(config);
		AsynCtrl ctrl = new AsynCtrl();
		ctrl.setService("helloworld");
		ctrl.setMessageId("10001");
		ctrl.setPeerId("local_mq"); 
		
		while(true){
			try{
				ZMsg msg = new ZMsg();
				msg.pushBack("asyn call hello");
				ZMsg res = client.send(ctrl, msg);
				res.dump();
				break;
			}catch (Exception e) {
				break;
			}
		}
		
		client.destroy();
	} 
}
