using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using zbus;
namespace zbus
{
    class SingleService
    {
        public static void Main(string[] args)
        {
            //1) 创建连接到ZBUS总线
            ConnectionConfig connCfg = new ConnectionConfig();
            connCfg.Host = "127.0.0.1";
            connCfg.Port = 15555;
            Connection conn = new Connection(connCfg);

            //2) 注册服务（MyService）
            WorkerConfig workerCfg = new WorkerConfig();
            workerCfg.Service = "MyService";
            workerCfg.Mode = WorkerConfig.MODE_LB;//负载均衡模式
            Worker worker = new Worker(conn, workerCfg);

            Console.WriteLine("C# Simple Worker Running...");
            //3) 服务业务逻辑循环体（等待ZBUS总线分发请求，处理请求，返回请求结果）
            while (true)
            {
                try
                {
                    //3.1) 等待ZBUS总线分发请求消息
                    ZMsg msg = worker.Recv();

                    //3.2) 业务逻辑处理请求消息
                    msg.Dump();

                    //3.3) 返回处理后的消息
                    msg = new ZMsg();
                    msg.PushBack("this is from c# loadbalancer");
                    worker.Send(msg);
                }
                catch (Exception e)
                {
                    Console.WriteLine(e);
                    break;
                }
            }

            //4) 清除连接相关
            conn.Destroy();
            worker.Destroy();
        } 
    }
}
