package net.zbus.pubsub;

import net.zbus.Connection;
import net.zbus.ConnectionConfig;
import net.zbus.Worker;
import net.zbus.WorkerConfig;
import net.zbus.ZMsg;

public class PubSubWorker {
 
	public static void main(String[] args) { 
		WorkerConfig workerCfg = new WorkerConfig();
		workerCfg.setService("pubsub");
		workerCfg.setMode(WorkerConfig.MODE_BC);
		
		ConnectionConfig connCfg = new ConnectionConfig();
		Connection connection = new Connection(connCfg);
		
		Worker worker = new Worker(connection, workerCfg);
		
		while(true){
			try{
				ZMsg msg = worker.recv();
				msg.dump();
			}catch (Exception e) {
				e.printStackTrace();
				break;
			}
		}
		connection.destroy();
		worker.destroy();
	}

}
