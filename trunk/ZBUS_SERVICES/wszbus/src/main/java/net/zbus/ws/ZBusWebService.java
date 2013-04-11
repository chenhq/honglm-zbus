package net.zbus.ws;

import javax.jws.WebMethod;
import javax.jws.WebService;
import javax.jws.soap.SOAPBinding;
import javax.jws.soap.SOAPBinding.Style;

@WebService
@SOAPBinding(style = Style.RPC)
public interface ZBusWebService {
	@WebMethod
	String[] callService(String service, String token, String... params);
	
	@WebMethod
	String jsonService(String service, String token, String funcName, String jsonParams);
}
