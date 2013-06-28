package net.zbus.rpc;


public class RpcClient {
 
	public static void main(String[] args) throws Exception { 	
		//1) 配置RPC链接
		RpcPoolConfig config = new RpcPoolConfig();
		config.setHost("127.0.0.1");
		config.setPort(15555);
		config.setService("MyRpc");
		
		RpcPool pool = new RpcPool(config);
		
		Rpc rpc = pool.borrowResource();
		
		//2) 远程方法调用
		int res = (Integer)rpc.invoke("plus", 23441, 22);
		System.out.println(res);
		
		//3) 销毁链接
		pool.returnResource(rpc);
		
		pool.destroy();
	}
}
