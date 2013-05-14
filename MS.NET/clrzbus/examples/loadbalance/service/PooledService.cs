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

            ZMsg result = new ZMsg();
            result.PushBack("this is from pooled c# worker");

            return result;
        }
    }

    class PooledService
    {
        public static void Main(string[] args)
        {
            //1) 配置服务信息
            WorkerPoolConfig config = new WorkerPoolConfig();
            config.Service = "MyService";
            config.Mode = WorkerConfig.MODE_LB;

            
            int threadCount = 2;
            Console.WriteLine("Pooled Workers({0}) Run...", threadCount);
            WorkerPool pool = new WorkerPool(config);
            //2)以指定的线程数运行服务处理业务逻辑代码
            pool.Run(threadCount, new MyServiceHandler()); 
             
        } 
    }
}
