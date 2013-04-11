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

import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import net.zbus.ServiceHandler;
import net.zbus.ZBusException;
import net.zbus.ZMsg;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.alibaba.fastjson.serializer.SerializerFeature;

class MethodInstance{
	public Method method;
	public Object instance;
	
	public MethodInstance(Method method, Object instance){
		this.method = method;
		this.instance = instance;
	}
}
public class JsonServiceHandler implements ServiceHandler {
	private Map<String,MethodInstance> methods = new HashMap<String, MethodInstance>();
	
	public JsonServiceHandler(Object... services){ 
		for(Object service: services){
			this.initCommandTable(service);
		}
	}
	
	private void initCommandTable(Object service){
		Class<?>[] classes = new Class<?>[service.getClass().getInterfaces().length+1];
		classes[0] = service.getClass(); 
		for(int i=1; i<classes.length; i++){
			classes[i] = service.getClass().getInterfaces()[i-1];
		}
		try { 
			for(Class<?> clazz : classes){
				Method [] methods = clazz.getMethods(); 
				for (Method m : methods) { 
					Remote cmd = m.getAnnotation(Remote.class);
					if(cmd != null){
						String key = cmd.id();
						if("".equals(key)){
							key = m.getName();
						} 
						if(this.methods.containsKey(key)){
							System.out.println(key+ " duplicated"); 
						}
						m.setAccessible(true);
						this.methods.put(key, new MethodInstance(m, service));  
					}
				} 
			}
		} catch (SecurityException e) {
			e.printStackTrace();
		}   
	}
	
	public String handleJsonRequest(byte[] jsonData){
		Exception error = null;
		Object result = null;
		String id = "";
		
		JSONObject req = null;
		String method = null;
		JSONArray args = null;
		MethodInstance target = null;
		try{
			req = (JSONObject) JSON.parse(jsonData);
		}catch (Exception e) {
			e.printStackTrace();
			error = e;
		}
		
		if(error == null){
			try{
				id = req.getString("id");
				method = req.getString("method");
				args = req.getJSONArray("params");
			}catch (Exception e) {
				error = e;
			}
			if(id == null){
				error = new ZBusException("missing id");
			}
			if(method == null){
				error = new ZBusException("missing method name");
			}
		}
		
		if(error == null){
			if(this.methods.containsKey(method)){
				target = this.methods.get(method); 
			}else{
				error = new ZBusException(method + " not found");
			}
		}
		
		if(error == null){
			try { 
				Class<?>[] types = target.method.getParameterTypes();
				if(types.length == args.size()){
					Object[] params = new Object[types.length]; 
					for(int i=0;  i<types.length; i++){
						params[i] = args.getObject(i, types[i]);
					}
					result = target.method.invoke(target.instance, params); 
				}else{
					error = new ZBusException("number of argument not match");
				} 
			}catch (Exception e) {
			    e.printStackTrace();
				error = e;
			}
		}
		JSONObject data = new JSONObject();
		data.put("id", id);
		if(error == null){  
			data.put("error", null);
			data.put("result", result); 
		}else{  
			data.put("error", error.getMessage());
			data.put("result", null);
		}
		//support null value
		return JSON.toJSONString(data, SerializerFeature.WriteMapNullValue);
	}

	@Override
	public ZMsg handleRequest(ZMsg request) { 
		ZMsg reply = new ZMsg();
		reply.pushBack("200"); //assume to be successful
		try {
			Iterator<byte[]> it = request.iterator();
			while (it.hasNext()) {
				byte[] data = it.next();
				String res = this.handleJsonRequest(data);
				reply.pushBack(res);
			}
		} catch (Exception e) {
			reply.clear();
			reply.pushBack("500");
			reply.pushBack("interal error: " + e.getMessage());
		}
		return reply;
	} 
}
