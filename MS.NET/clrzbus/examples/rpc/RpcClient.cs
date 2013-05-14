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
            //1) 配置RPC链接
            RpcConfig config = new RpcConfig();
            config.Service = "MyRpc";  
            Rpc rpc = new Rpc(config);


            //2) 远程方法调用
            object res = rpc.Invoke("user", "前海股权交易中心");  
            IDictionary<string, object> user = (Dictionary<string, object>)res; 
            Console.WriteLine("name: " + user["name"]);
            Console.WriteLine("addr: " + user["addr"]);

            //3) 销毁链接
            rpc.Dispose();
            Console.ReadKey();
        } 
    }
}
