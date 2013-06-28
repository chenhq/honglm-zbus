package net.zbus.proxy;

import java.util.HashMap;
import java.util.Map;

import net.zbus.Pool;
import net.zbus.PoolConfig;
import net.zbus.proxy.tc.TCClient;
import net.zbus.proxy.tc.TCClientFactory;
import net.zbus.proxy.tc.TCConfig;
import net.zbus.proxy.tc.TCResult;

public class TCClientTest{ 

	public static void main2(String[] args) throws Exception {
		TCConfig config = new TCConfig(); 
		config.setService("ETC-172.24.180.71"); 
		config.setVerbose(true);
		TCClientFactory factory = new TCClientFactory(config);
		
		PoolConfig poolCfg = new PoolConfig(); 
		poolCfg.setMaxActive(2); 
		Pool<TCClient> pool = new Pool<TCClient>(factory, poolCfg);
		
		TCClient client = pool.borrowResource();

		Map<String, String> params = new HashMap<String, String>();
		params.put("comb_id", "815");
	    TCResult res = client.request("300", "720", "3", params);
	    
	    res.dump();

		pool.returnResource(client);
		pool.destroy();
	}
	public static void main(String[] args) throws Exception {
		TCConfig config = new TCConfig(); 
		config.setService("ETC-172.24.180.71"); 
		config.setVerbose(true);
		TCClientFactory factory = new TCClientFactory(config);
		
		PoolConfig poolCfg = new PoolConfig(); 
		poolCfg.setMaxActive(2); 
		Pool<TCClient> pool = new Pool<TCClient>(factory, poolCfg);
		
		TCClient client = pool.borrowResource();
		
		Map<String, String> params = new HashMap<String, String>();
		params.put("sendtime_type", "1");
		params.put("send_type", "1");
		params.put("channels", "1;5");
		params.put("validate_chans", "1;5");
		params.put("sms_channel", "1");
		params.put("sms_subcode", "");
		params.put("sms_format", "32");
		params.put("rece_type", "2");
		params.put("rece_codes", "13632906105;");
		params.put("msg_level", "0");
		params.put("msg_id", "");
		params.put("summary", "d0c5cfa2b1eacce2d0c5cfa2b1eacce2d0c5cfa2b1eacce2");
		params.put("content", "d0c5cfa2b1eacce2d0c5cfa2b1eacce2d0c5cfa2b1eacce2d0c5cfa2b1eacce2d0c5cfa2b1eacce2");
		params.put("send_level", "3");
		params.put("sender_org_code", "1100");
		params.put("charge_org_code", "1100");
		params.put("sender_code", "1100");
		params.put("sender_name", "���ں�����·֤ȯӪҵ��");
		params.put("charge_user_code", "00000");
		params.put("product_id", "0");
		params.put("send_sys", "9996");
		params.put("available_time", "20121231083939");
		params.put("resend_channel", "4");
		params.put("stk_code", "600251.2");
		params.put("stk_name", "��ũ�ɷ�");
		params.put("msg_cls", "");
		params.put("sms_flag", "0");
		params.put("sms_content", "");
		params.put("msg_type", "0");

		TCResult res = client.request("770", "1", "0", params); 

		res.dump();

		pool.returnResource(client);
		pool.destroy();
	}
}
