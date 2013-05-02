package net.zbus.ws;

import java.io.InputStream;
import java.util.Properties;

import javax.xml.ws.Endpoint;

import net.zbus.ConnectionConfig;

public class Publisher {
	public static void main(String[] args) throws Exception {
		Properties props = new Properties(); 
		InputStream is = ClassLoader.getSystemResourceAsStream("wszbus.properties");
		props.load(is); 
		
		ConnectionConfig config = new ConnectionConfig();
		String host = props.getProperty("zbus-host", "localhost").trim();
		int port = Integer.valueOf(props.getProperty("zbus-port", "15555").trim());
		int timeout = Integer.valueOf(props.getProperty("zbus-timeout", "2500").trim()); 
		config.setHost(host);
		config.setPort(port);
		
		String wsAddress = props.getProperty("ws-address", "http://localhost:15556/ws/zbus");
		
		final Endpoint ep =  Endpoint.create(new ZBusWebServiceImpl(config, timeout));		 
		ep.publish(wsAddress);
	}
}
