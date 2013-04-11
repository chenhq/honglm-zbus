package net.zbus.proxy;

import net.zbus.proxy.trade.Request;
import net.zbus.proxy.trade.Response;
import net.zbus.proxy.trade.TradeClient;
import net.zbus.proxy.trade.TradeConfig;


public class TradeClientTest{ 
	public static void main2(String[] args) throws Exception {
		TradeConfig config = new TradeConfig(); 
		config.setService("Trade");
		TradeClient client = new TradeClient(config);
		
		String bnn = "BF6C2C496593917FEEDFE0F6C62BA237C32A99886D66CC3D20DBAEB38484D001C86EE38576C6A92CA3C94C03B1AD284A0F85498D3DEB9134DFC57BABE8271401";
	    String bnd = "2D160168583065B8C83E9AF204C30A363015BC8BD198C0CA350F091AE73F90EE321E8767FED9CAA9FDD58960436B320FF4B7CFD06BFDA418D31290CA40DAE0F1";
	    
	    String password = "IOOF5tX20S6tyLPzjMTjce8h9YxX8OwmpY1poI3qPomwRxQhML9vmMGVQS+mMl9wQd76SIVr1HCDBtqtFH/p6w==";
	    String res = client.decrypt("WANGAN", bnn, bnd, password);
	    System.out.println(res);
		
	}

	public static void main(String[] args) throws Exception {
		TradeConfig config = new TradeConfig(); 
		config.setService("Trade");  
		TradeClient client = new TradeClient(config);
		
		Request t = new Request();
		t.funcId = "420301";
		t.tradeNodeId = "";
		t.sessionId = "";
		t.userInfo = "0~~127.0.0.1~1100";
		 
		t.loginType = "Z";
		t.loginId = "110000001804";
		t.custOrg = "1100"; 
		t.operIp = "172.16.8.107";
		t.operOrg = "yyt";
		t.operType = "g"; 
		
		
		String password = client.encrypt("KDE", "1", "110000001804");
		System.out.println(password);
		
		t.params.add("Z");
		t.params.add("110000001804");
		t.params.add(password);

		 
		Response res = client.trade(t);
		System.out.println(res.toString());
		
		client.destroy();
	} 

}

