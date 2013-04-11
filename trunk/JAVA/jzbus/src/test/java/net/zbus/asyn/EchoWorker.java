package net.zbus.asyn;

import net.zbus.Connection;
import net.zbus.ConnectionConfig;
import net.zbus.Worker;
import net.zbus.WorkerConfig;
import net.zbus.ZMsg;

public class EchoWorker {
 
	public static void main(String[] args) { 
		WorkerConfig workerCfg = new WorkerConfig();
		workerCfg.setService("helloworld");
		
		ConnectionConfig connCfg = new ConnectionConfig();  
		Connection conn = new Connection(connCfg);
		
		Worker worker = new Worker(conn, workerCfg);
		
		while(true){
			try{
				ZMsg msg = worker.recv(); 
				worker.send(msg);
			}catch (Exception e) {
				e.printStackTrace();
				break;
			}
		}
		worker.destroy();
	}

}
