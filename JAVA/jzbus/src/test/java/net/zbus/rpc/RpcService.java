package net.zbus.rpc;

import java.util.List;
import java.util.Map;

import net.zbus.ServiceHandler;
import net.zbus.Worker;
import net.zbus.WorkerPool;
import net.zbus.WorkerPoolConfig;


class MyService {
	@Remote()
	public String echo(String ping) {
		return ping;
	}

	@Remote(id = "user")
	public User user(String name) { 
		User user = new User();
		user.setAddr("深圳");
		user.setName("HONG");
		return user; 
	}
	 
	@Remote
	public String save(User user) {  
		System.out.println(user.getName());
		System.out.println(user.getAddr());
		return "OK";
	}
	
	@Remote
	public String saveUserList(List<Map<String,Object>> users) {  
		for(Map<String,Object> user : users){
			for(Map.Entry<String, Object> e : user.entrySet()){
				System.out.println(e.getKey()+"=>"+e.getValue());
			}
		}
		return "OK";
	}

	@Remote()
	public int plus(int a, int b) {
		return a + b;
	}
	
	@Remote
	public byte[] bin(){
		byte[] res = new byte[100];
		for(int i=0;i<res.length;i++) res[i] = (byte)i;
		return res;
	}
}

public class RpcService {
	public static void main(String[] args) { 
		//1) 配置服务信息
		WorkerPoolConfig cfg = new WorkerPoolConfig();
		cfg.setService("MyRpc");
		cfg.setMode(Worker.MODE_LB);
		cfg.setBrokers(new String[]{"127.0.0.1:15555","42.120.17.176:15555"}); //总线地址（多总线注册）
		
		int threadCount = 2;
		WorkerPool wc = new WorkerPool(cfg); 
		System.out.format("Pooled RPC(%d) Run...\n", threadCount);
		
		//2)以指定的线程数运行RPC实例
		ServiceHandler handler = new JsonServiceHandler(new MyService());
		wc.run(threadCount, handler);
		
	}
}
