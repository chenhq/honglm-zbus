package net.zbus.proxy;

import java.util.HashMap;
import java.util.Map;

import net.zbus.Pool;
import net.zbus.PoolConfig;
import net.zbus.proxy.kcxp.KCXPClient;
import net.zbus.proxy.kcxp.KCXPClientFactory;
import net.zbus.proxy.kcxp.KCXPConfig;
import net.zbus.proxy.kcxp.KCXPResult;


public class KCXPClientTest {  
	
	public static void main(String[] args) throws Exception {
		KCXPConfig config = new KCXPConfig(); 
		config.setVerbose(true);
		config.setService("KCXP"); 
		KCXPClientFactory factory = new KCXPClientFactory(config);
		
		PoolConfig poolCfg = new PoolConfig(); 
		poolCfg.setMaxActive(2); 
		Pool<KCXPClient> pool = new Pool<KCXPClient>(factory, poolCfg);
		
		KCXPClient client = pool.borrowResource(); 
		
		String funcId = "420301";
		String reqStr = "420301|Z|110000000197|1100|172.24.2.135|1100|d||||F4CE460321ED|Z|110000000197|bwm5UjxHEmJLjg6GR7Q96w==|||";

		for(int i=0;i<10;i++){
			Map<String, String> params = new HashMap<String, String>();
			params.put("request", reqStr);
	
			KCXPResult res = client.request(funcId, params);
			res.dump();
		}

		pool.returnResource(client);
		pool.destroy();
	}
}
