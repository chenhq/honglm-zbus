package net.zbus.ws;

import java.util.Iterator;

import javax.jws.WebService;

import net.zbus.BusClient;
import net.zbus.ConnectionConfig;
import net.zbus.ZMsg;
 
@WebService(endpointInterface = "net.zbus.ws.ZBusWebService")
public class ZBusWebServiceImpl implements ZBusWebService {

	private BusClient client;
	private int timeout = 2500;
	public ZBusWebServiceImpl(ConnectionConfig config,int timeout){
		this.client = new BusClient(config);
		this.timeout = timeout;
	}
	@Override
	public String[] callService(String service, String token, String... params) { 
		ZMsg request = new ZMsg();
		for(String param : params){
			request.pushBack(param);
		} 
		ZMsg res = this.client.request(service, token, request, this.timeout);
		String[] result = null;
		if(res != null){
			result = new String[res.frameSize()];
			int i = 0;
			Iterator<byte[]> it = res.iterator();
			while(it.hasNext()){
				result[i++] = new String(it.next());
			}
		}
		return result; 
	}
	
	@Override
	public String jsonService(String service, String token, String funcName, String jsonParams) { 
		String[] res = this.callService(service, token, funcName, jsonParams);
		if(res == null){
			return "request null";
		} else {
			if(res.length > 1){
				return res[1];
			}else {
				return res[0];
			}
		}
	}

}
