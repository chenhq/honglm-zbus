package net.zbus.proxy;

import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

import net.zbus.ServiceHandler;
import net.zbus.WorkerPool;
import net.zbus.WorkerPoolConfig;
import net.zbus.ZMsg;

import com.eno.ENOInterface.ENOMsg;
import com.eno.InfoAgent.TCRSField;
import com.eno.etcpool.ENOProxy;
import com.eno.tcrs.ENORecordset;

class ETCProxyHandler implements ServiceHandler {

	@Override
	public ZMsg handleRequest(ZMsg req) {

		if (req.frameSize() != 4) {
			ZMsg res = new ZMsg();
			res.pushBack("400");
			res.pushBack("request <service_id>, <main_funcid>, <sub_funcid>, <params>");
			return res;
		}

		try {
			int serviceId = Integer.valueOf(req.popFrontStr());
			int mainFuncId = Integer.valueOf(req.popFrontStr());
			int subFuncId = Integer.valueOf(req.popFrontStr());

			byte[] reqData = req.getFirst();

			ENOMsg msg = new ENOMsg();

			byte[] data = ENOProxy.getData(serviceId, mainFuncId, subFuncId,
					reqData, "", true, msg);
			
			if(data == null){
				ZMsg res = new ZMsg();
				res.pushBack("500");
				res.pushBack(msg.getMessage());
				return res;
			}
			
			ENORecordset[] mrs = ENORecordset.constructMR(data, 0);

			if (mrs == null || mrs.length == 0) {
				ZMsg res = new ZMsg();
				res.pushBack("500");
				res.pushBack(msg.getMessage());
				return res;
			}
 
			if (mrs[0].isErrorInfo()) { //error
				mrs[0].fetchFirst();
				ZMsg res = new ZMsg();
				res.pushBack("500");
				res.pushBack(mrs[0].getString(0));
				return res;
			}
			
			ZMsg res = new ZMsg();
			res.pushBack("200"); 
			res.pushBack(String.format("%d", mrs.length)); 
			for(int rsIdx=0; rsIdx<mrs.length; rsIdx++){ 
				ENORecordset rs = mrs[rsIdx];
				int cols = rs.fieldCount();
				rs.fetchFirst();  
				int rows = 0;
				ZMsg zrs = new ZMsg();
				while (!rs.isEOF()) { 
					for (int i = 0; i < cols; i++) {
						TCRSField field = rs.fieldDesc(i);
						String key = field.fieldName;
						String val = rs.toString(i); //damned ENO's bugs and fucking design
						if(val == null){
							val = "";
						}
						zrs.pushBack(key);
						zrs.pushBack(val);
					} 
					rows++;
					rs.fetchNext();
				} 
				zrs.pushFront(String.format("%d,%d", rows, cols)); 
				int frameSize = zrs.frameSize();
				for(int i=0;i<frameSize;i++){
					res.pushBack(zrs.popFront());
				}
			} 
			return res;

		} catch (Exception e) {
			ZMsg res = new ZMsg();
			res.pushBack("500");
			res.pushBack(e.getMessage());
			return res;
		}
	}
}

public class TCProxy { 
	public static void main(String[] args){   
		Properties props = new Properties(); 
		InputStream is = ClassLoader.getSystemResourceAsStream("zbus.properties");
		try {
			props.load(is);
		} catch (IOException e) { 
			e.printStackTrace();
		}  
		WorkerPoolConfig cfg = new WorkerPoolConfig();
		String service = props.getProperty("service", "ETC-172.24.180.71");
		String[] brokers = props.getProperty("brokers", "localhost:15555").split(",");
		String registerToken = props.getProperty("registerToken", ""); 
		String accessToken = props.getProperty("accessToken", ""); 
		boolean verbose = Boolean.valueOf(props.getProperty("verbose", "true"));
		
		cfg.setService(service);
		cfg.setBrokers(brokers);
		cfg.setRegisterToken(registerToken);
		cfg.setAccessToken(accessToken);
		cfg.setVerbose(verbose);
		
		int threadCount = Integer.valueOf(props.getProperty("threadCount", "1")); 
		WorkerPool pool = new WorkerPool(cfg);
		pool.run(threadCount, new ETCProxyHandler()); 
	} 
}
