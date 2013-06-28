package net.zbus;

public class ZBusException extends RuntimeException {  
	private static final long serialVersionUID = 6006204188240205218L;
    public ZBusException(String message){
    	super(message);
    }
	public ZBusException() {
		super(); 
	}
	public ZBusException(String message, Throwable cause,
			boolean enableSuppression, boolean writableStackTrace) {
		super(message, cause, enableSuppression, writableStackTrace); 
	}
	public ZBusException(String message, Throwable cause) {
		super(message, cause); 
	}
	public ZBusException(Throwable cause) {
		super(cause); 
	}  
    
}
