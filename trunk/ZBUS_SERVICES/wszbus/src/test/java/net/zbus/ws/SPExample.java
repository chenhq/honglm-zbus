package net.zbus.ws;

import java.net.URL;
import java.util.HashMap;
import java.util.Map;

import javax.xml.namespace.QName;
import javax.xml.ws.Service;

import com.alibaba.fastjson.JSON;

public class SPExample { 
	
	public static void main(String[] args) throws Exception {
		URL url = new URL("http://172.24.180.27:15556/ws/zbus?wsdl"); 
		QName qname = new QName("http://ws.zbus.net/", "ZBusWebServiceImplService");
		Service service = Service.create(url, qname); 
        ZBusWebService zbus = service.getPort(ZBusWebService.class);
  
        
        Map<String, String> params = new HashMap<String, String>();
		params.put("@type", "task");
		params.put("@operation", "add");
		params.put("@subject", "");
		params.put("@receiveTime", "20121221");
		params.put("@appCode", "IB");
		params.put("@appWorkFlowId", "appWorkFlowId");

		params.put("@appModule", "");
		params.put("@appFlowName", ""); 
		params.put("@appWorkFlowUrl", "");
		params.put("@createPeopleId", "");
		params.put("@createPeopleName", "");
		params.put("@createTime", "");
		params.put("@currNode", ""); 
		params.put("@currPeople", "洪磊明 16011");
		params.put("@exInput", "");
		params.put("@importance", "");
		params.put("@isMobile", "");
		params.put("@messageType", ""); 
		params.put("@prePeopleId", "");
		params.put("@prePeopleName", "");
		params.put("@readPeople", "");  
		params.put("@summary", "");
		params.put("@timeLimit", "");
		
		String spName = "taskSynch";
        String paramsStr = JSON.toJSONString(params);
		
        
        String[] res = zbus.callService("SP-172.24.180.133", "", spName, paramsStr);
        if(res != null){
        	for(String part : res){
        		System.out.println(part);
        	}
        } 
        
	} 
}
