package net.zbus.proxy.trade;

import java.util.ArrayList;
import java.util.List;

public class Response {
	public String rawResponse;

	public String funcId = "";
	public String tradeNodeId = "";
	public String userInfo = "";
	public String returnCode = "";
	public String returnMsg = "";

	public List<List<String>> body = new ArrayList<List<String>>();

	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append(this.funcId);
		sb.append("|");
		sb.append(this.tradeNodeId);
		sb.append("|");
		sb.append(this.userInfo);
		sb.append("|");
		sb.append(this.returnCode);
		sb.append("|");
		sb.append(this.returnMsg);

		sb.append(";");

		for (int i = 0; i < this.body.size(); i++) {
			sb.append(this.body.get(i));
			if (i < this.body.size() - 1)
				sb.append("|");
		}

		return sb.toString();
	}

	public static Response load(String message) {
		Response response = new Response();

		response.rawResponse = message;

		String[] parts = message.split(";");
		if (parts.length < 1) {
			throw new RuntimeException("trade message should be ; seperated");
		}
		String headPart = parts[0];
		String bodyPart = parts.length>1?parts[1]:"";

		String[] head = headPart.split("[|]", -1);
		if (head.length < 5) {
			throw new RuntimeException(
					"trade response head should be 5 parts(|)");
		}
		response.funcId = head[0];
		response.tradeNodeId = head[1];
		response.userInfo = head[2];
		response.returnCode = head[3];
		response.returnMsg = head[4];

		String[] rows = bodyPart.split(";");
		for (String row : rows) {
			parts = row.split("[|]", -1);
			List<String> record = new ArrayList<String>();
			for (String value : parts)
				record.add(value);
			response.body.add(record);
		}
		return response;
	}
}
