package net.zbus;

public class Worker {
	public static final String MODE_LB = "1"; 
	public static final String MODE_PUBSUB = "2";
	public static final String MODE_BC = "3";
	
	private SafeConnection connection; 
	
	private final String service;
	private final String mode;
	private final String registerToken;
	private final String accessToken;
	
	public byte[] recvMsgId;  
	public byte[] recvSockId;
	

	public Worker(SafeConnection connection, WorkerConfig config){
		this.connection = connection; 
		this.service = config.getService();
		this.mode = config.getMode();
		this.registerToken = config.getRegisterToken();
		this.accessToken = config.getAccessToken();
		
		this.construct(config);
	}
	
	public ZMsg recv(){
		ConnectionUnsafe conn = this.connection.get();
		
		byte[][] res = conn.zbuswrk_recv(conn, this);
		ZMsg result = new ZMsg();
		if(res != null){
			if(res.length < 2){
				throw new ZBusException("zbuswrk_recv at lest 2 frame required");
			}
			this.recvSockId = res[0];
			this.recvMsgId = res[1];
			
			for(int i=2; i<res.length; i++){
				result.pushBack(res[i]);
			}
		}
		return result;
	}
	
	public int send(ZMsg message){
		ConnectionUnsafe conn = this.connection.get();
		if(this.recvSockId == null){
			throw new ZBusException("recvSockId should be set");
		}
		if(this.recvMsgId == null){
			throw new ZBusException("recvMsgId should be set");
		}
		
		int frames = message.frameSize()+2;
		byte[][] req = new byte[frames][];
		req[0] = this.recvSockId;
		req[1] = this.recvMsgId;
		
		this.recvMsgId = null;
		this.recvSockId = null;
		
		for(int i=2;i<frames;i++){
			req[i] = message.popFront();
		}
		
		return conn.zbuswrk_send(conn, this, req);
	}
	
	public int subscribe(String topic){
		ConnectionUnsafe conn = this.connection.get();
		return conn.zbuswrk_subscribe(conn, this, topic);
	}
	
	public int unsubscribe(String topic){
		ConnectionUnsafe conn = this.connection.get();
		return conn.zbuswrk_unsubscribe(conn, this, topic);
	}
	
	private long handle; 	
	/** Initialize the JNI interface */
	protected native void construct(WorkerConfig config);

	/** Free all resources used by JNI interface. */
	@Override
	protected native void finalize();

	public void destroy() { 
		finalize();
	}

	protected long getHandle() {
		return handle;
	} 
	
}


