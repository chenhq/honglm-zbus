package net.zbus.ws;

import java.net.URL;

import javax.xml.namespace.QName;
import javax.xml.ws.Service;

public class ClientExample { 
	
	public static void main(String[] args) throws Exception { 
		URL url = new URL("http://localhost:15556/ws/zbus?wsdl");
		// 1st argument service URI, refer to wsdl document above
		// 2nd argument is service name, refer to wsdl document above
		QName qname = new QName("http://ws.zbus.net/", "ZBusWebServiceImplService");
		Service service = Service.create(url, qname);
		 
        ZBusWebService zbus = service.getPort(ZBusWebService.class);
        
        String[] res = zbus.callService("echo", "", "中文");
        for(String x : res){ 
        	System.out.println(x);
        	
        } 
	} 
}
