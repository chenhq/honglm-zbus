package net.zbus.sql;

import java.nio.charset.Charset;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.Map;

import net.zbus.BusClient;
import net.zbus.ConnectionConfig;
import net.zbus.ZMsg;

import com.alibaba.fastjson.JSON;

public class Test {
	 
	public static void main(String[] args) throws SQLException { 
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
 
		ConnectionConfig config = new ConnectionConfig();
		String service = "SP-172.24.180.133";
		
		BusClient client = new BusClient(config);
		ZMsg req = new ZMsg();
		req.pushBack(spName);
		req.pushBack(JSON.toJSONString(params));
		ZMsg res = client.request(service, "", req, 2500);
		System.out.println(new String(res.getLast(), Charset.forName("GBK")));
		
	}

}
