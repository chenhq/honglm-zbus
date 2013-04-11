package net.zbus.ws;

import java.io.IOException;
import java.io.InputStream;
import java.util.List;
import java.util.Properties;

import javax.xml.ws.Binding;
import javax.xml.ws.Endpoint;
import javax.xml.ws.handler.Handler;

import net.zbus.ConnectionConfig;

public class Publisher {
	public static void main(String[] args) {
		Properties props = new Properties(); 
		InputStream is = ClassLoader.getSystemResourceAsStream("wszbus.properties");
		try {
			props.load(is);
		} catch (IOException e) { 
			e.printStackTrace();
		} 
		ConnectionConfig config = new ConnectionConfig();
		String host = props.getProperty("zbus-host", "localhost").trim();
		int port = Integer.valueOf(props.getProperty("zbus-port", "15555").trim());
		int timeout = Integer.valueOf(props.getProperty("zbus-timeout", "2500").trim()); 
		config.setHost(host);
		config.setPort(port);
		
		String wsAddress = props.getProperty("ws-address", "http://localhost:15556/ws/zbus");
		String newPrefix = props.getProperty("ws-newprefix", "soap");
		String oldPrefix = props.getProperty("ws-oldprefix", "S");
		
		
		final Endpoint ep =  Endpoint.create(new ZBusWebServiceImpl(config, timeout));
		final Binding binding = ep.getBinding();
		
		@SuppressWarnings("rawtypes")
		final List<Handler> handlerChain = binding.getHandlerChain();
		
	    handlerChain.add(new SOAPBodyHandler(newPrefix, oldPrefix));
	    binding.setHandlerChain(handlerChain);
		ep.publish(wsAddress);
	}
}
