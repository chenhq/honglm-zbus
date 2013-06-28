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
package net.zbus.rpc;

import java.nio.charset.Charset;

import net.zbus.BusClient;
import net.zbus.ZBusException;
import net.zbus.ZMsg;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import com.alibaba.fastjson.serializer.SerializerFeature;


public class Rpc extends BusClient {   
	 private String service; 
	 private String token = "";  
	 private Charset encoding;
	 private int timeout = 2500;//ms
	 
	 protected Rpc(RpcConfig config){
		 super(config);
		 this.service = config.getService();
		 this.token = config.getToken();
		 this.encoding = Charset.forName(config.getEncoding());
		 this.timeout = config.getTimeout(); 
	 }
	 
	 public Object invoke(String method, Object... args){
		 
		 JSONObject req = new JSONObject();
		 req.put("id", "jsonrpc");
		 req.put("method", method);
		 req.put("params", args);
		 ZMsg msg = new ZMsg();
		 msg.pushBack(JSON.toJSONBytes(req, SerializerFeature.WriteMapNullValue));
		 msg = this.request(this.service, this.token, msg, this.timeout); 
		 if(msg == null){
			 throw new ZBusException("json rpc request timeout");
		 }
		 if(msg.frameSize() != 2){
			 throw new ZBusException("json rpc format error[2 frames required]");
		 }
		 String status = msg.popFrontStr();
		 if(status.equals("200")){
			 try{
				 String text = new String(msg.popFront(), this.encoding);
				 JSONObject res = (JSONObject) JSON.parse(text);
				 if(res.containsKey("result"))
					 return res.get("result");
				 if(res.containsKey("error"))
					 throw new ZBusException(res.getString("error"));
				 
				 return null;
			 }catch(Exception e){
				 throw new ZBusException(e.getMessage());
			 }
		 }else{
			 throw new ZBusException(msg.popFrontStr());
		 } 
	 }
}
