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

 
public class Connection {
	private ThreadLocal<ConnectionUnsafe> connection;
	
	public Connection(ConnectionConfig config){ 
		final ConnectionConfig cfg = config;
		this.connection = new ThreadLocal<ConnectionUnsafe>(){
			@Override
			protected ConnectionUnsafe initialValue() { 
				return new ConnectionUnsafe(cfg); 
			}
			@Override
			public void remove() { 
				this.get().destroy();
			} 
		};
		this.connection.get();
	}
	public int send(ZMsg message){
		ConnectionUnsafe conn = this.connection.get();
		int frames = message.frameSize();
		byte[][] req = new byte[frames][];
		for(int i=0;i<frames;i++){
			req[i] = message.popFront();
		}
		int res = conn.zbusconn_send(req);
		return res;
	}
	
	public ZMsg recv(int timeout){
		ConnectionUnsafe conn = this.connection.get();
		byte[][] res = conn.zbusconn_recv(timeout);
		ZMsg result = new ZMsg();
		if(res != null){
			for(byte[] data : res){
				result.pushBack(data);
			}
		}
		return result;
	}
	
	public int route(String sockId, ZMsg message){
		ConnectionUnsafe conn = this.connection.get();
		int frames = message.frameSize();
		byte[][] req = new byte[frames][];
		for(int i=0;i<frames;i++){
			req[i] = message.popFront();
		}
		int res = conn.zbusconn_route(sockId.getBytes(), req);
		return res;
	}
	
	public int probe(int timeout){
		ConnectionUnsafe conn = this.connection.get();
		return conn.zbusconn_probe(timeout);
	}
	
	public int reconnect(){
		ConnectionUnsafe conn = this.connection.get();
		return conn.zbusconn_reconnect();
	}
	
	ConnectionUnsafe get(){
		return this.connection.get();
	}
	
	public void destroy(){ 
		this.connection.remove();
	}
}
