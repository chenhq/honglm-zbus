using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using zbus;
namespace zbus
{
    class AsynRecv
    {
        public static void Main(string[] args)
        {
            //1) 创建连接
            ConnectionConfig connCfg = new ConnectionConfig();
            connCfg.Host = "127.0.0.1";
            connCfg.Port = 15555;
            connCfg.Id = "local_mq"; 
            BusClient client = new BusClient(connCfg);

            //2）异步处理消息
            while (true)
            {
                try
                {
                    //recv参数指定发送心跳时间间隔，（防火墙切断链接）
                    ZMsg msg = client.Recv(2500);//2.5s for ping
                    msg.Dump();
                }
                catch (Exception e)
                {
                    Console.WriteLine(e);
                    break;
                }
            }

            //3)销毁连接
            client.Destroy();
           
        } 
    }
}
