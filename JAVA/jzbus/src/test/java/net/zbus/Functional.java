package net.zbus;


public class Functional {

	public static void main(String[] args) {
		ConnectionConfig config = new ConnectionConfig();
		config.setVerbose(true); 
		Connection conn = new Connection(config);
		conn.probe(2500);
		
		conn.destroy();
	}

}
