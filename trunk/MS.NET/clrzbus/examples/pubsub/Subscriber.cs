using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using zbus;
namespace zbus
{
    class PubSubWorker
    {
        public static void Main(string[] args)
        {
            ConnectionConfig connCfg = new ConnectionConfig();
            connCfg.Host = "127.0.0.1";
            connCfg.Port = 15555;
            Connection conn = new Connection(connCfg);

            WorkerConfig workerCfg = new WorkerConfig();
            workerCfg.Service = "pubsub";
            workerCfg.Mode = WorkerConfig.MODE_BC; //broadcast, now as PubSub

            Worker worker = new Worker(conn, workerCfg);
            while (true)
            {
                try
                {
                    ZMsg msg = worker.Recv();
                    msg.Dump(); 
                }
                catch (Exception e)
                {
                    Console.WriteLine(e);
                    break;
                }
            }
            conn.Destroy();
            worker.Destroy();
        } 
    }
}
