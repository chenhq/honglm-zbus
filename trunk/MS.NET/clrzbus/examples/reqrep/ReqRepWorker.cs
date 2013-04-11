using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using zbus;
namespace zbus
{
    class ReqRepWorker
    {
        public static void Main(string[] args)
        {
            ConnectionConfig connCfg = new ConnectionConfig();
            connCfg.Host = "127.0.0.1";
            connCfg.Port = 15555;
            Connection conn = new Connection(connCfg);

            WorkerConfig workerCfg = new WorkerConfig();
            workerCfg.Service = "helloworld";

            Worker worker = new Worker(conn, workerCfg);
            while (true)
            {
                try
                {
                    ZMsg msg = worker.Recv();
                    msg.Dump();
                    msg.PushBack("from C#");
                    worker.Send(msg);
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
