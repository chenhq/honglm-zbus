package net.zbus.rpc;

public class RpcClient {
 
	public static void main(String[] args) throws Exception { 	
		//1) 配置RPC链接
		RpcConfig cfg = new RpcConfig(); 
		cfg.setService("MyRpc");
		cfg.setHost("127.0.0.1");
		cfg.setPort(15555);
		Rpc rpc = new Rpc(cfg);
		
		//2) 远程方法调用
		int res = (Integer)rpc.invoke("plus", 23441, 22);
		System.out.println(res);
		
		//3) 销毁链接
		rpc.destroy(); 
	}
}
