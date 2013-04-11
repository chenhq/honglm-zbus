package net.zbus.rpc;

public class RpcClient {
 
	public static void main(String[] args) throws Exception { 
		RpcConfig cfg = new RpcConfig(); 
		cfg.setService("rpc");  
		Rpc rpc = new Rpc(cfg);
		 
 
		//JSONObject user = (JSONObject)rpc.invoke("user", "hong");
		//System.out.println(user); 
		
		int res = (Integer)rpc.invoke("plus", 1, 2);
		System.out.println(res);
		rpc.destroy(); 
	}
}
