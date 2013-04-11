using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using zbus;
namespace zbus
{
    class ClientRecv
    {
        public static void Main(string[] args)
        {
            ConnectionConfig connCfg = new ConnectionConfig();
            connCfg.Host = "127.0.0.1";
            connCfg.Port = 15555;
            connCfg.Id = "local_mq";

            BusClient client = new BusClient(connCfg);

            while (true)
            {
                try
                { 
                    ZMsg msg = client.Recv(4000);//4s for ping
                    msg.Dump();
                }
                catch (Exception e)
                {
                    Console.WriteLine(e);
                    break;
                }
            }

            client.Destroy();
           
        } 
    }
}
