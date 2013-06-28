package net.zbus.proxy.trade;

import java.io.UnsupportedEncodingException;

import net.zbus.BusClient;
import net.zbus.ZMsg;


public class TradeClient extends BusClient{ 
	private TradeConfig config;
	
	protected TradeClient(TradeConfig config) {
		super(config); 
		this.config = config;
	}
	
	/**
	 * 
	 * @param algorithm AES or KDE
	 * @param password
	 * @param key
	 * @return
	 */
	public String encrypt(String algorithm, String password, String key){
		ZMsg msg = new ZMsg(); 
		msg.pushBack("encrypt");
		msg.pushBack(algorithm);
		msg.pushBack(password);
		msg.pushBack(key);
		
		String service = this.config.getCryptService();
		String token = this.config.getCryptToken();
		int timeout = this.config.getCryptTimeout();
		msg = this.request(service, token, msg, timeout); 
		
		
		String code = msg.popFrontStr();
		if(code.equals("200")){
			return new String(msg.getLast());
		}else {
			throw new RuntimeException(new String(msg.getLast()));
		} 
	}
	
	/**
	 * 
	 * @param algorithm WANGAN
	 * @param password
	 * @param key
	 * @return
	 */
	public String decrypt(String algorithm, String publicKey, String privateKey, String password){
		ZMsg msg = new ZMsg(); 
		msg.pushBack("decrypt");
		msg.pushBack(algorithm);
		msg.pushBack(publicKey);
		msg.pushBack(privateKey);
		msg.pushBack(password);
		
		String service = this.config.getCryptService();
		String token = this.config.getCryptToken();
		int timeout = this.config.getCryptTimeout();
		
		msg = this.request(service, token, msg, timeout); 
		String code = msg.popFrontStr();
		if(code.equals("200")){
			return new String(msg.getLast());
		}else {
			throw new RuntimeException(new String(msg.getLast()));
		} 
	}
	
	public Response trade(Request request){  
		ZMsg msg = new ZMsg(); 
		msg.pushBack(request.toString());
		
		String service = this.config.getService();
		String token = this.config.getToken();
		int timeout = this.config.getTimeout();
		msg = this.request(service, token, msg, timeout);   
		
		Response response = new Response();
		String code = msg.popFrontStr();
		if(code.equals("200")){
			try {
				String res = new String(msg.getLast(), "GBK");
				response = Response.load(res);
			} catch (UnsupportedEncodingException e) { 
				e.printStackTrace();
			}
			
		}else {
			throw new RuntimeException(new String(msg.getLast()));
		}
		return response; 
	}

}

