package net.zbus.proxy.kcxp;

import net.zbus.ConnectionConfig;

public class KCXPConfig extends ConnectionConfig {
	private String service = "KCXP";
	private String token = "";
	private int timeout = 2500;//ms
	
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
	
}
