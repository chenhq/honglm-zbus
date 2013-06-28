package net.zbus;

class MyThread extends Thread {
	private BusClient client;

	public MyThread(BusClient client) {
		this.client = client;
	}

	@Override
	public void run() {
		while (true) {
			try {
				System.out.println("thread "+Thread.currentThread().getId());
				ZMsg msg = new ZMsg();
				msg.pushBack("hello");
				msg = client.request("MyService", "", msg, 2500);
				msg.dump();
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

}

public class Functional {

	
	public static void main(String[] args) throws Exception {
		ConnectionConfig config = new ConnectionConfig();
		BusClient client = new BusClient(config);
		
		ZMsg msg = new ZMsg();
		msg.pushBack("hello");
		msg = client.request("Trade", "", msg, 2500);
		if(msg != null){
			msg.dump();
		}
		client.destroy();
	}

}
