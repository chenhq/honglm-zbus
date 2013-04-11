using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using zbus;
namespace zbus
{
    
    class RpcClient
    {
        public static void Main(string[] args)
        {
            RpcConfig config = new RpcConfig();
            config.Service = "rpc"; 

            Rpc rpc = new Rpc(config);
            object res = rpc.Invoke("user", "hong"); 

            IDictionary<string, object> user = (Dictionary<string, object>)res;
            Console.WriteLine(res.GetType());
            Console.WriteLine("name: " + user["Name"]);
            Console.WriteLine("addr: " + user["Addr"]);

            res = rpc.Invoke("plus", 1, 2);
            Console.WriteLine(res.GetType());
            Console.WriteLine(res);

            Console.ReadKey();
        } 
    }
}
