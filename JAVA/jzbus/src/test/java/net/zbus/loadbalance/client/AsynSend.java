package net.zbus.loadbalance.client;

import net.zbus.AsynCtrl;
import net.zbus.BusClient;
import net.zbus.BusClientFactory;
import net.zbus.ConnectionConfig;
import net.zbus.Pool;
import net.zbus.PoolConfig;
import net.zbus.ZMsg;

public class AsynSend {
 
	public static void main(String[] args) { 
		//1) 创建连接
		ConnectionConfig connCfg = new ConnectionConfig();
		connCfg.setHost("127.0.0.1");
		connCfg.setPort(15555); 
		BusClientFactory factory = new BusClientFactory(connCfg);
		
		
		PoolConfig config = new PoolConfig(); 
		config.setMaxActive(2); 
		Pool<BusClient> pool = new Pool<BusClient>(factory, config);
		
		BusClient client = pool.borrowResource();
		
		
		//2) 异步控制结构
		AsynCtrl ctrl = new AsynCtrl();
		ctrl.setService("MyService");
		ctrl.setMessageId("MSG_"+System.currentTimeMillis()); //本条消息ID，用于异步消息标识
		ctrl.setPeerId("local_mq"); //消息投递目标ID，与接收方协商
		ctrl.setTimeout(2500); //异步投递失败提示最长时间
		
		//3) 消息体（帧组成）
		ZMsg msg = new ZMsg();
		msg.pushBack("asyn call frame1");
		msg.pushBack("asyn call frame2");
		
		//4) 发送异步消息
		ZMsg res = client.send(ctrl, msg);
		res.dump();
		
		//5）返回链接
		pool.returnResource(client);
		pool.destroy();
		
	} 
}
