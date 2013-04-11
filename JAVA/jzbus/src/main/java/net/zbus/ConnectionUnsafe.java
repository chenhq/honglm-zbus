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

class ConnectionUnsafe {
	private long handle;

	public ConnectionUnsafe(ConnectionConfig config) {
		if (config.getCtx() == null) {
			config.setCtx(ZContext.getInstance(1));
		}
		construct(config);
	}

	/** Initialize the JNI interface */
	protected native void construct(ConnectionConfig config);

	/** Free all resources used by JNI interface. */
	@Override
	protected native void finalize();

	public void destroy() {
		finalize();
	}

	protected long getHandle() {
		return handle;
	}

	protected native int zbusconn_reconnect();
	
	protected native int zbusconn_send(byte[][] message);

	protected native byte[][] zbusconn_recv(int timeout);

	protected native int zbusconn_route(byte[] sockId, byte[][] message);
	
	protected native int zbusconn_probe(int timeout);
	
	protected native byte[][] zbuscli_request(String service, String token, byte[][] message, int timeout);
	
	protected native byte[][] zbuscli_send(AsynCtrl ctrl, byte[][] message);
	
	protected native byte[][] zbuscli_recv(int pingInterval);
	
	protected native int zbuswrk_send(ConnectionUnsafe conn, Worker worker, byte[][] message);
	
	protected native byte[][] zbuswrk_recv(ConnectionUnsafe conn, Worker worker);
	
	protected native int zbuswrk_subscribe(ConnectionUnsafe conn, Worker worker, String topic);
	
	protected native int zbuswrk_unsubscribe(ConnectionUnsafe conn, Worker worker, String topic);
	
	protected native byte[][] zbusmon_request(String token, byte[][] message, int timeout);
}
