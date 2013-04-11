package net.zbus.ws;

import java.net.URL;
import java.util.HashMap;
import java.util.Map;

import javax.xml.namespace.QName;
import javax.xml.ws.Service;

import com.alibaba.fastjson.JSON;

public class ClientExample { 
	
	public static void main(String[] args) throws Exception { 
		URL url = new URL("http://localhost:15556/ws/zbus?wsdl");
		// 1st argument service URI, refer to wsdl document above
		// 2nd argument is service name, refer to wsdl document above
		QName qname = new QName("http://ws.zbus.net/", "ZBusWebServiceImplService");
		Service service = Service.create(url, qname);
		 
        ZBusWebService zbus = service.getPort(ZBusWebService.class);
  
        Map<String,String> params = new HashMap<String, String>();
		params.put("557", "9999,0001,");
		params.put("988", "1"); 
		params.put("9110", "200"); 
		
        String[] res = zbus.callService("APEX", "", "302201", JSON.toJSONString(params));
        if(res != null){
        	for(String part : res){ 
        		System.out.println(part);
        	}
        }
	} 
}
