using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using zbus;
namespace zbus
{
    class ClientSend
    {
        public static void Main(string[] args)
        {

            ConnectionConfig config = new ConnectionConfig();
            config.Host = "127.0.0.1";
            config.Port = 15555;
            //config.Verbose = true;

            BusClient client = new BusClient(config);

            AsynCtrl ctrl = new AsynCtrl();
            ctrl.Service = "helloworld";
            ctrl.PeerId = "local_mq";
            ctrl.MessageId = "10000";

            ZMsg msg = new ZMsg();
            msg.PushBack("C# asyn call");
            msg = client.Send(ctrl, msg);
            msg.Dump();

            client.Destroy();

            Console.ReadKey();
        } 
    }
}
