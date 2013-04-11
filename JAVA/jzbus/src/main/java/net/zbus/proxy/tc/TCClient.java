package net.zbus.proxy.tc;

import java.util.HashMap;
import java.util.Map;

import net.zbus.BusClient;
import net.zbus.ZBusException;
import net.zbus.ZMsg;

public class TCClient extends BusClient { 
	private TCConfig config;
	public TCClient(TCConfig config) {
		super(config);
		this.config = config;
	}

	public TCResult request(String serviceId, String mainFuncid,
			String subFuncid, Map<String, String> params) {
		ZMsg req = new ZMsg();
		req.pushBack(serviceId);
		req.pushBack(mainFuncid);
		req.pushBack(subFuncid);
		req.pushBack(toParamString(params));

		String service = this.config.getService();
		String token = this.config.getToken();
		int timeout = this.config.getTimeout();
		ZMsg res = this.request(service, token, req, timeout);
		String status = res.popFrontStr();
		if (status.equals("200")) {
			if (res.frameSize() < 1) {
				throw new ZBusException("result missing resultset length frame");
			}
			String lenStr = res.popFrontStr();
			TCResult result = new TCResult();
			int rsLen = Integer.valueOf(lenStr);
			for (int rsIdx = 0; rsIdx < rsLen; rsIdx++) {
				if (res.frameSize() < 1) {
					throw new ZBusException( "result missing resultset dimension frame");
				}
				String dimStr = res.popFrontStr();
				String[] parts = dimStr.split(",");
				if (parts.length != 2) {
					throw new ZBusException("result length format error");
				}
				int rows = Integer.valueOf(parts[0]);
				int cols = Integer.valueOf(parts[1]);
				if (res.frameSize() < rows * cols * 2) {
					throw new ZBusException("resultset length not matched");
				}
				TCTable table = new TCTable();
				for (int r = 0; r < rows; r++) {
					Map<String, byte[]> row = new HashMap<String, byte[]>();
					for (int c = 0; c < cols; c++) {
						String key = res.popFrontStr();
						byte[] val = res.popFront();
						row.put(key, val);
					}
					table.addRow(row);
				}
				result.addTable(table);
			}
			return result;
		} else {
			String error = new String(res.getLast());
			throw new ZBusException(error);
		}
	}

	public static String toParamString(Map<String, String> paras) {
		if (paras == null)
			return "";
		StringBuilder sb = new StringBuilder();
		int i = 0;
		for (Map.Entry<String, String> entry : paras.entrySet()) {
			String value = entry.getValue();
			if (value != null) {
				if (i > 0) {
					sb.append("&");
				}
				sb.append(entry.getKey()).append("=").append(value);
				i++;
			}
		}
		return sb.toString();
	}
}
