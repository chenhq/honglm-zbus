using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using zbus;
namespace zbus
{
    class SyncClient
    {
        public static void Main(string[] args)
        {
            //1) 创建连接
            ConnectionConfig config = new ConnectionConfig();
            config.Host = "127.0.0.1";
            config.Port = 15555; 
            BusClient client = new BusClient(config);

            //2) 组装消息（消息帧数组） 
            ZMsg request = new ZMsg();
            request.PushBack("Frame1");     //消息帧1 -- 字符类型
            request.PushBack(new byte[10]); //消息帧2 -- 二进制串
            request.PushBack("request from C#"); //消息帧3 -- 字符类型

            //3) 向ZBUS总线发送请求
            ZMsg result = client.Request("MyService", "", request);
            result.Dump();


            //4) 销毁客户端
            client.Destroy();

            Console.ReadKey();
        } 
    }
}
