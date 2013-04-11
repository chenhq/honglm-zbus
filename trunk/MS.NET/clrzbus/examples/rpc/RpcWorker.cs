using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using zbus;
namespace zbus
{
    class User
    {
        public string Name;
        public string Addr;
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
            user.Name = name;
            user.Addr = "深圳";
            return user;
        }

    }
    class RpcWorker
    {
        public static void Main(string[] args)
        {
            WorkerPoolConfig config = new WorkerPoolConfig();
            config.Service = "rpc";

            WorkerPool pool = new WorkerPool(config);
            ServiceHandler handler = new JsonServiceHandler(new MyService());
            pool.Run(2, handler); //run with 2 threads
        } 
    }
}
