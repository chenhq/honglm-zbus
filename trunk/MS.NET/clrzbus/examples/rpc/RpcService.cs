using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using zbus;
namespace zbus
{
    class User
    {
        public string name;
        public string addr;
    }
    class MyService
    {
        [Remote]
        public string echo(string msg)
        {
            return msg;
        }

        [Remote]
        public int plus(int a, int b)
        {
            return a + b;
        }

        [Remote]
        public User user(string name)
        { 
            User user = new User();
            user.name = name;
            user.addr = "深圳";
            return user;
        }

    }
    class RpcService
    {
        public static void Main(string[] args)
        {
            //1) 配置服务信息
            WorkerPoolConfig config = new WorkerPoolConfig();
            config.Service = "MyRpc";
            config.Mode = WorkerConfig.MODE_LB;
            config.Brokers = new string[] { "127.0.0.1:15555" }; //总线地址（多总线注册）


            //2)以指定的线程数运行RPC实例
            int threadCount = 2;
            Console.WriteLine("Pooled RPC({0}) Run...", threadCount);
            WorkerPool pool = new WorkerPool(config);
            ServiceHandler handler = new JsonServiceHandler(new MyService());
            pool.Run(threadCount, handler);
        } 
    }
}
