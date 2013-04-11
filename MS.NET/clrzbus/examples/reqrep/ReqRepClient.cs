using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using zbus;
namespace zbus
{
    class ReqRepClient
    {
        public static void Main(string[] args)
        {
            ConnectionConfig config = new ConnectionConfig();
            config.Host = "127.0.0.1";
            config.Port = 15555;
            //config.Verbose = true;
         
            BusClient client = new BusClient(config);

            ZMsg msg = new ZMsg();
            msg.PushBack("hello");
            msg = client.Request("helloworld", "", msg);
            msg.Dump();

            client.Destroy();

            Console.ReadKey();
        } 
    }
}
