package net.zbus.proxy.kcxp;

import java.io.UnsupportedEncodingException;
import java.util.HashMap;
import java.util.Map;

import net.zbus.BusClient;
import net.zbus.ZBusException;
import net.zbus.ZMsg;


public class KCXPClient extends BusClient {  
	private KCXPConfig config;

	protected KCXPClient(KCXPConfig config) {
		super(config);
		this.config = config;
	}
	
	public KCXPResult request(String funcId, Map<String, String> params) {
		Map<String, byte[]> byteParams = new HashMap<String, byte[]>();
		for(Map.Entry<String, String> e : params.entrySet()){
			byteParams.put(e.getKey(), e.getValue().getBytes());
		}
		return this.requestRaw(funcId, byteParams);
	}

	public KCXPResult requestRaw(String funcId, Map<String, byte[]> params) {

		ZMsg req = new ZMsg();
		req.pushBack(funcId);
		if (params != null) {
			for (Map.Entry<String, byte[]> e : params.entrySet()) {
				req.pushBack(e.getKey().trim());
				req.pushBack(e.getValue());
			}
		}
		ZMsg msg = null;
		try{
			String service = this.config.getService();
			String token = this.config.getToken();
			int timeout = this.config.getTimeout();
			msg = this.request(service, token, req, timeout); 
		}catch (ZBusException e) { 
			throw new ZBusException("request time out, "+e.getMessage());
		}
		if (msg.frameSize() < 1) {
			throw new RuntimeException("result invalid");
		} 
		String code = msg.popFrontStr(); 

		KCXPResult result = new KCXPResult();
		if (code.equals("200")) {
			if (msg.frameSize() == 0) {
				throw new RuntimeException("table count required");
			} 
			int tableCount = Integer.valueOf(msg.popFrontStr());
			for (int t = 0; t < tableCount; t++) {
				if (msg.frameSize() == 0) {
					throw new RuntimeException("table count required");
				} 
				String sizeString = msg.popFrontStr(); 
				String[] dim = sizeString.split(",");
				if (dim.length != 2) {
					throw new RuntimeException("table dimension format wrong "
							+ sizeString);
				}
				int row = Integer.valueOf(dim[0]);
				int col = Integer.valueOf(dim[1]);

				int kvCount = row * col * 2;
				if (msg.frameSize() < kvCount) {
					throw new RuntimeException("table following row/col size not matched ");
				}

				KCXPTable table = new KCXPTable();
				for (int r = 0; r < row; r++) {
					Map<String, byte[]> rowMap = new HashMap<String, byte[]>();
					for (int c = 0; c < col; c++) { 
						String key = msg.popFrontStr().trim();
						byte[] val = msg.popFront();
						rowMap.put(key, val); 
					}
					table.addRow(rowMap);
				}
				result.addTable(table);
			}
			result.setStatus("0");
		} else if (code.startsWith("9")) {// 9XX
			result.setStatus(new String(msg.getFirst()));
			try {
				result.setErrorMessage(new String(msg.getLast(),"GBK"));
			} catch (UnsupportedEncodingException e) {
				result.setErrorMessage(new String(msg.getLast()));
			}
		} else {
			throw new RuntimeException(new String(msg.getLast()));
		}
		return result;
	}
}
