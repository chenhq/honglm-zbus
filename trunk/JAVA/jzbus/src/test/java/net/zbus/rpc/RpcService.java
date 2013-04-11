package net.zbus.rpc;

import java.util.List;
import java.util.Map;

import net.zbus.ServiceHandler;
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
		MyService service = new MyService();
		
		WorkerPoolConfig config = new WorkerPoolConfig();
		config.setService("rpc");
		//config.setBrokers(new String[]{"localhost:15555"});
		
		ServiceHandler handler = new JsonServiceHandler(service);
		
		WorkerPool pool = new WorkerPool(config); 
		
		pool.run(2, handler);
	}
}
