package net.zbus.proxy.trade;

import net.zbus.ConnectionConfig;

public class TradeConfig extends ConnectionConfig {
	private String service = "Trade";
	private String token = "";
	private int timeout = 2500;//ms
	
	private String cryptService = "Utils";
	private String cryptToken = "";
	private int cryptTimeout = 2500; //ms
	
	public String getService() {
		return service;
	}
	public void setService(String service) {
		this.service = service;
	}
	public String getToken() {
		return token;
	}
	public void setToken(String token) {
		this.token = token;
	}
	public int getTimeout() {
		return timeout;
	}
	public void setTimeout(int timeout) {
		this.timeout = timeout;
	}
	public String getCryptService() {
		return cryptService;
	}
	public void setCryptService(String cryptService) {
		this.cryptService = cryptService;
	}
	public String getCryptToken() {
		return cryptToken;
	}
	public void setCryptToken(String cryptToken) {
		this.cryptToken = cryptToken;
	}
	public int getCryptTimeout() {
		return cryptTimeout;
	}
	public void setCryptTimeout(int cryptTimeout) {
		this.cryptTimeout = cryptTimeout;
	} 
	
	
}
