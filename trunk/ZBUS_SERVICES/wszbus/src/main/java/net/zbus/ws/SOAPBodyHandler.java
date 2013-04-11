package net.zbus.ws;

import java.util.Set;

import javax.xml.namespace.QName;
import javax.xml.soap.SOAPException;
import javax.xml.soap.SOAPMessage;
import javax.xml.ws.handler.MessageContext;
import javax.xml.ws.handler.soap.SOAPHandler;
import javax.xml.ws.handler.soap.SOAPMessageContext;


public class SOAPBodyHandler implements SOAPHandler<SOAPMessageContext> {
	private String newPrefix = "soap";
	private String oldPrefix = "S";
	
	public SOAPBodyHandler(){
		
	}
	public SOAPBodyHandler(String newPrefix, String oldPrefix){
		this.newPrefix = newPrefix;
		this.oldPrefix = oldPrefix;
	}
	@Override
	public boolean handleMessage(SOAPMessageContext context) { 
		if ((Boolean)context.get(MessageContext.MESSAGE_OUTBOUND_PROPERTY)) { 
			try { 
				SOAPMessage sm = context.getMessage(); 
				sm.getSOAPPart().getEnvelope().setPrefix(newPrefix);
				sm.getSOAPPart().getEnvelope().removeNamespaceDeclaration(oldPrefix);
				
				if(sm.getSOAPHeader() != null){
					sm.getSOAPHeader().setPrefix(newPrefix);
				}
				if(sm.getSOAPBody() != null){
					sm.getSOAPBody().setPrefix(newPrefix); 
				}
			} catch (SOAPException ex) {
				ex.printStackTrace();
			}
		}
		return true;
	}

	@Override
	public boolean handleFault(SOAPMessageContext context) {
		return false;
	}

	@Override
	public void close(MessageContext context) {
	}

	@Override
	public Set<QName> getHeaders() {
		return null;
	}
	
	public String getNewPrefix() {
		return newPrefix;
	}
	public void setNewPrefix(String newPrefix) {
		this.newPrefix = newPrefix;
	}
	public String getOldPrefix() {
		return oldPrefix;
	}
	public void setOldPrefix(String oldPrefix) {
		this.oldPrefix = oldPrefix;
	}

}
