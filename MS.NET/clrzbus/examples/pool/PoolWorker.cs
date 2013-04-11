using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using zbus;
namespace zbus
{
    class MyServiceHandler : ServiceHandler
    {
        public ZMsg HandleRequest(ZMsg request)
        {
            request.Dump();
            request.PushBack("from C# pool worker");

            return request;
        }
    }

    class PoolWorker
    {
        public static void Main(string[] args)
        {
            WorkerPoolConfig config = new WorkerPoolConfig();
            config.Service = "helloworld"; 

            WorkerPool pool = new WorkerPool(config);
            pool.Run(2, new MyServiceHandler()); //run with 2 threads
             
        } 
    }
}
