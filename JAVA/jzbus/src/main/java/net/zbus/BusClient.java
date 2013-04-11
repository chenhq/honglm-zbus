/************************************************************************
 *  Copyright (c) 2011-2012 HONG LEIMING.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the
 * following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 ***************************************************************************/
/************************************************************************
 *  Copyright (c) 2011-2012 HONG LEIMING.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the
 * following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 ***************************************************************************/
package net.zbus;
 

public class BusClient { 
	private Connection connection;
	private boolean ownConnection;
	
	public BusClient(Connection connection){
		this.connection = connection;
		this.ownConnection = false;
	}
	
	public BusClient(ConnectionConfig config){
		this.connection = new Connection(config);
		this.ownConnection = true;
	}
	
	public ZMsg request(String service, String token, ZMsg message, int timeout){
		ConnectionUnsafe conn = this.connection.get();
		int frames = message.frameSize();
		byte[][] req = new byte[frames][];
		for(int i=0;i<frames;i++){
			req[i] = message.popFront();
		}
		
		byte[][] res = conn.zbuscli_request(service, token, req, timeout);
		
		ZMsg result = new ZMsg();
		if(res != null){
			for(byte[] data : res){
				result.pushBack(data);
			}
		}
		return result;
	}
	
	public ZMsg send(AsynCtrl ctrl, ZMsg message){
		ConnectionUnsafe conn = this.connection.get();
		int frames = message.frameSize();
		byte[][] req = new byte[frames][];
		for(int i=0;i<frames;i++){
			req[i] = message.popFront();
		}
		
		byte[][] res = conn.zbuscli_send(ctrl, req);
		
		ZMsg result = new ZMsg();
		if(res != null){
			for(byte[] data : res){
				result.pushBack(data);
			}
		}
		return result;
	}
	
	public boolean publish(String service, String token, String topic, ZMsg message, int timeout){
		AsynCtrl ctrl = new AsynCtrl();
		ctrl.setService(service);
		ctrl.setToken(token);
		ctrl.setTimeout(timeout);
		
		message.pushFront(topic);
		ZMsg res = this.send(ctrl, message);
		String status = res.popFrontStr();
		if(status != null && status.equals("200")){
			return true;
		}
		return false;
	}
	
	/**
	 * 
	 * @param probeInterval should be at least 1000(ms), otherwise, it will default to minimum value 1000
	 * @return
	 */
	public ZMsg recv(int probeInterval){
		ConnectionUnsafe conn = this.connection.get();
		byte[][] res = conn.zbuscli_recv(probeInterval);
		
		ZMsg result = new ZMsg();
		if(res != null){
			for(byte[] data : res){
				result.pushBack(data);
			}
		}
		return result;
	}
	
	
	public ZMsg monitor(String token, ZMsg message, int timeout){
		ConnectionUnsafe conn = this.connection.get();
		int frames = message.frameSize();
		byte[][] req = new byte[frames][];
		for(int i=0;i<frames;i++){
			req[i] = message.popFront();
		}
		
		byte[][] res = conn.zbusmon_request(token, req, timeout);
		
		ZMsg result = new ZMsg();
		if(res != null){
			for(byte[] data : res){
				result.pushBack(data);
			}
		}
		return result;
	}
	
	
	public void destroy(){ 
		if(this.ownConnection && this.connection != null){
			this.connection.destroy();
			this.connection = null;
		}
	}  
}
