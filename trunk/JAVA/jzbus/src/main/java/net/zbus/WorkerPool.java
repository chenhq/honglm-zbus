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
 
class WorkerThread extends Thread{
	private ConnectionConfig connectionConfig;
	private WorkerConfig workerConfig;
	private Object handler;
	public WorkerThread(ConnectionConfig connectionConfig, WorkerConfig workerCfg, Object handler){
		this.connectionConfig = connectionConfig;
		this.workerConfig = workerCfg;
		if(!(handler instanceof ServiceHandler) && !(handler instanceof WorkerHandler)){
			throw new ZBusException("handler invalid type");
		}
		this.handler = handler;
	}
	
	@Override
	public void run() { 
		Connection connection = new Connection(this.connectionConfig);
		Worker worker = new Worker(connection, workerConfig);
		if(handler instanceof ServiceHandler){
			ServiceHandler serviceHandler = (ServiceHandler)handler;
			while(true){
				try{
					ZMsg request = worker.recv();
					if(request == null) break;
					ZMsg result = serviceHandler.handleRequest(request);
					if(result != null)
						worker.send(result);
				}catch (Exception e) { 
					e.printStackTrace();
					break;
				}
			} 
		} else {//worker handler
			WorkerHandler workerHandler = (WorkerHandler)handler;
			workerHandler.handle(worker);
		}
		connection.destroy();
		worker.destroy();
	} 
}


public class WorkerPool {    
	protected WorkerPoolConfig config;   
	
	public WorkerPool(WorkerPoolConfig config){ 
		if(config.getService() == null){
			throw new ZBusException("client config missing service");
		}
		if(config.getBrokers() == null){
			throw new ZBusException("client config missing brokers");
		}
		this.config = config; 
	} 

	
	private void runHandler(int threadCount,final Object handler) {
		if(!(handler instanceof ServiceHandler) && !(handler instanceof WorkerHandler)){
			throw new ZBusException("handler invalid type");
		} 
		
		boolean ownCtx = false;
		ZContext ctx = null;
		if(this.config.getCtx() == null){
			ctx = new ZContext(1);
			ownCtx = true;
			this.config.setCtx(ctx);
		}
		
		String[] brokers = this.config.getBrokers();
		Thread[] threads = new Thread[threadCount*brokers.length]; 
		for(int i=0; i<brokers.length; i++){
			final WorkerConfig workerCfg = new WorkerConfig();
			final ConnectionConfig connCfg = new ConnectionConfig();
			
			String host = brokers[i].split(":")[0];
			int port = Integer.valueOf(brokers[i].split(":")[1]);
			connCfg.setCtx(this.config.getCtx()); 
			connCfg.setHost(host);
			connCfg.setPort(port);
			connCfg.setVerbose(this.config.isVerbose());
			 
			workerCfg.setService(this.config.getService());
			workerCfg.setMode(this.config.getMode());
			workerCfg.setRegisterToken(this.config.getRegisterToken());
			workerCfg.setAccessToken(this.config.getAccessToken());
			
			for(int j=0; j<threadCount; j++){ 
				Thread thread = new WorkerThread(connCfg, workerCfg, handler);
				threads[i*threadCount+j] = thread;
			}
		}
		for(Thread thread : threads){
			thread.start();
		} 
		for(Thread thread : threads){
			try {
				thread.join();
			} catch (InterruptedException e) { 
				e.printStackTrace();
			}
		}  
		
		if(ownCtx && ctx != null){
			ctx.destroy();
		}
	} 
	
	public void run(int threadCount,final ServiceHandler handler) {  
		this.runHandler(threadCount, handler);
	} 
	
	public void run(int threadCount,final WorkerHandler handler) {  
		this.runHandler(threadCount, handler);
	} 
	
	public static void main(String[] args) { 
		WorkerPoolConfig cfg = new WorkerPoolConfig();
		cfg.setService("helloworld");
		cfg.setBrokers(new String[]{"127.0.0.1:15555","172.24.180.27:15555"});
		
		WorkerPool wc = new WorkerPool(cfg); 
		 
		wc.run(2, new ServiceHandler() { 			
			@Override
			public ZMsg handleRequest(ZMsg request) {
				request.dump(); 
				return request;
			} 
		});
	}
}
