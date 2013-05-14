using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using zbus;
namespace zbus
{
    class Publisher
    {
        public static void Main(string[] args)
        {
            ConnectionConfig config = new ConnectionConfig();
            config.Host = "127.0.0.1";
            config.Port = 15555;
            //config.Verbose = true;

            BusClient client = new BusClient(config);
            
            ZMsg msg = new ZMsg();
            msg.PushBack("publish from C#");

            bool result = client.Publish("MyPubSub", "", "topic1", msg);
           
            Console.WriteLine("publish result: "+result);

            client.Destroy();

            Console.ReadKey();
        } 
    }
}
