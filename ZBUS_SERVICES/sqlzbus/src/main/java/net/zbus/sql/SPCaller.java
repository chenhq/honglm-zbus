package net.zbus.sql;

import java.io.InputStream;
import java.sql.CallableStatement;
import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Properties;

import net.zbus.ServiceHandler;
import net.zbus.WorkerPool;
import net.zbus.WorkerPoolConfig;
import net.zbus.ZMsg;

import org.apache.commons.dbcp.BasicDataSource;

import com.alibaba.fastjson.JSON;

class SPHandler implements ServiceHandler {
	private BasicDataSource dataSource;

	public SPHandler(BasicDataSource dataSource) {
		this.dataSource = dataSource;
	}

	public String callSP(String spName, Map<String, String> params) throws Exception {
		StringBuilder sql = new StringBuilder();
		sql.append("{ call ");
		sql.append(spName);
		sql.append(" (");
		if (params.size() == 0) {
			sql.append(")}");
		}
		for (int i = 0; i < params.size(); i++) {
			if (i == params.size() - 1) {
				sql.append(" ? )}");
			} else {
				sql.append(" ?,");
			}
		}
		Connection conn = null;
		CallableStatement stmt = null;
		try {
			conn = this.dataSource.getConnection();
			stmt = conn.prepareCall(sql.toString());
			
			for (Map.Entry<String, String> e : params.entrySet()) {
				stmt.setString(e.getKey(), e.getValue());
			}
			stmt.execute(); 
			
			ResultSet rs = stmt.getResultSet(); 
			if(rs != null){
				List<Map<String, Object>> rows = new ArrayList<Map<String, Object>>();
				ResultSetMetaData meta = rs.getMetaData();
				int cols = meta.getColumnCount();
				while (rs.next()) {
					Map<String, Object> row = new HashMap<String, Object>();
					for (int i = 0; i < cols; i++) {
						String key = meta.getColumnName(i + 1);
						Object val = rs.getObject(i + 1);
						row.put(key, val);
					}
					rows.add(row);
				}
				rs.close();
				
				return JSON.toJSONString(rows);
			} else {
				int count = stmt.getUpdateCount();
				return JSON.toJSONString(count);
			}
			
		} finally { 
			if(stmt != null) stmt.close();
			if(conn != null) conn.close();
		} 
	}

	@SuppressWarnings("unchecked")
	@Override
	public ZMsg handleRequest(ZMsg req) {

		if (req.frameSize() != 2) {
			ZMsg res = new ZMsg();
			res.pushBack("400");
			res.pushBack("<sp_id>, <json_params_str> required");
			return res;
		}

		try {
			String spName = req.popFrontStr();
			String jsonParamsStr = req.popFrontStr();
			Map<String, String> params = (Map<String, String>) JSON.parse(jsonParamsStr); 
			
			String jsonResult = callSP(spName, params); 
			
			ZMsg res = new ZMsg(); 
			res.pushBack("200");
			res.pushBack(jsonResult); 
			
			return res;

		} catch (Exception e) {
			ZMsg res = new ZMsg();
			res.pushBack("500");
			res.pushBack(e.toString());
			return res;
		}
	}
}

public class SPCaller {
	public static void main(String[] args) throws Exception {
		Properties props = new Properties();
		InputStream is = ClassLoader.getSystemResourceAsStream("sqlzbus.properties");
		props.load(is);  

		BasicDataSource dataSource = new BasicDataSource();

		dataSource.setDriverClassName(props.getProperty("sql.driver").trim());
		dataSource.setUsername(props.getProperty("sql.user").trim());
		dataSource.setPassword(props.getProperty("sql.password").trim());
		dataSource.setUrl(props.getProperty("sql.url").trim());
		dataSource.setMaxActive(Integer.valueOf(props.getProperty("sql.maxAcive", "10").trim()));
		dataSource.setMaxIdle(Integer.valueOf(props.getProperty("sql.maxIdle","5").trim()));
		dataSource.setInitialSize(Integer.valueOf(props.getProperty("sql.initialSize", "5").trim()));

		int threadCount = Integer.valueOf(props.getProperty("threadCount", "1"));
		
		WorkerPoolConfig cfg = new WorkerPoolConfig();
		String service = props.getProperty("service", "SP-localhost");
		String[] brokers = props.getProperty("brokers", "localhost:15555").split(",");
		String registerToken = props.getProperty("registerToken", ""); 
		String accessToken = props.getProperty("accessToken", ""); 
		boolean verbose = Boolean.valueOf(props.getProperty("verbose", "false"));
		
		cfg.setService(service);
		cfg.setBrokers(brokers);
		cfg.setRegisterToken(registerToken);
		cfg.setAccessToken(accessToken);
		cfg.setVerbose(verbose); 
		WorkerPool pool = new WorkerPool(cfg);
		pool.run(threadCount, new SPHandler(dataSource));
	}
}
