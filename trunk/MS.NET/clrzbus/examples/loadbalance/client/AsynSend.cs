using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using zbus;
namespace zbus
{
    class AsynSend
    {
        public static void Main(string[] args)
        {
            //1) 创建连接
            ConnectionConfig config = new ConnectionConfig();
            config.Host = "127.0.0.1";
            config.Port = 15555;  
            BusClient client = new BusClient(config);

            //2) 异步控制结构
            AsynCtrl ctrl = new AsynCtrl();
            ctrl.Service = "MyService";
            ctrl.PeerId = "local_mq";//消息投递目标ID，与接收方协商
            ctrl.MessageId = "10000"; 
            ctrl.Timeout = 2500; ////异步投递失败提示最长时间(ms)


            //3) 消息体（帧组成）
            ZMsg msg = new ZMsg();
            msg.PushBack("asyn call frame1");
            msg.PushBack("asyn call frame2");

            //4) 发送异步消息
            msg = client.Send(ctrl, msg);
            msg.Dump();

            //5）销毁链接
            client.Destroy();

            Console.ReadKey();
        } 
    }
}
