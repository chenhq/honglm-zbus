using System;
using System.Collections.Generic;
using System.Threading;
 
namespace zbus {
    
    public class WorkerConfig
    {
        public static readonly string MODE_LB = "1";
        public static readonly string MODE_PUBSUB = "2";
        public static readonly string MODE_BC = "3";

        public string Service;// required
        public string Mode = MODE_LB;
        public string RegisterToken = "";
        public string AccessToken = "";
    }



    public class Worker : IDisposable{  
        public byte[] RecvMsgId;
        public byte[] RecvSockId;


        private IntPtr _handle;
        private readonly Connection connection;
	    private readonly String service;
	    private readonly String mode;
	    private readonly String registerToken;
        private readonly String accessToken;

        public Worker(Connection connection, WorkerConfig config){
            if (config.Service == null || config.Service == "")
            {
                throw new ZBusException("WorkerConfig, missing service");
            }
            if (config.Mode != WorkerConfig.MODE_LB && config.Mode != WorkerConfig.MODE_BC
                && config.Mode != WorkerConfig.MODE_PUBSUB)
            {
                throw new ZBusException("worker mode wrong");
            }
            this.connection = connection;
            this.service = config.Service;
            this.mode = config.Mode;
            this.registerToken = config.RegisterToken;
            this.accessToken = config.AccessToken;

            IntPtr c_cfg = C.zbuswrk_cfg_new(this.service, this.mode, this.registerToken, this.accessToken);
            this._handle = C.zbuswrk_new(c_cfg);
            C.zbuswrk_cfg_destroy(out c_cfg);
        }

        public ZMsg Recv(){
            IntPtr res = C.zbuswrk_recv(this.connection.Handle, this._handle);

            IntPtr c_sock_id, c_msg_id;
            C.zbuswrk_get_address(this._handle, out c_sock_id, out c_msg_id);
            this.RecvSockId = ZMsg.ToZFrame(c_sock_id);
            this.RecvMsgId = ZMsg.ToZFrame(c_msg_id);

            ZMsg result = null;
            if (res != IntPtr.Zero)
            {
                result = ZMsg.ToZMsg(res);
            }

            return result;
        }


        public int Send(ZMsg msg){
            IntPtr c_msg = C.zmsg_new();
            IEnumerator<byte[]> fe = msg.GetEnumerator();
            while (fe.MoveNext())
            {
                IntPtr c_frame = ZMsg.ToZFramePtr(fe.Current);
                C.zmsg_push_back(c_msg, c_frame);
            }

            IntPtr c_sock_id, c_msg_id;
            c_sock_id = ZMsg.ToZFramePtr(this.RecvSockId);
            c_msg_id = ZMsg.ToZFramePtr(this.RecvMsgId);

            C.zbuswrk_set_address(this._handle, c_sock_id, c_msg_id);

            return C.zbuswrk_send(this.connection.Handle, this._handle, c_msg);
        }

        public int Subscribe(string topic)
        {
            return C.zbuswrk_subscribe(this.connection.Handle, this._handle, topic);
        }

        public int unsubscribe(string topic)
        {
            return C.zbuswrk_unsubscribe(this.connection.Handle, this._handle, topic);
        }

        ~Worker()
        {
            Dispose();
        }
         
        public void Dispose()
        {
            this.Destroy();
        }

        public void Destroy()
        {
            if (this._handle != IntPtr.Zero)
            {
                C.zbuswrk_destroy(out this._handle);
            }
        }
        
         
    }

    public interface ServiceHandler
    {
        ZMsg HandleRequest(ZMsg request);
    }

    public interface WorkerHandler
    {
        void HandleWorker(Worker worker);
    }


    class WorkerThreadStart
    { 
        private ConnectionConfig connCfg;
        private WorkerConfig workerCfg;
        private object handler;

        public WorkerThreadStart(ConnectionConfig connCfg, WorkerConfig workerCfg, object handler)
        {
            this.connCfg = connCfg;
            this.workerCfg = workerCfg;
            if (handler is ServiceHandler || handler is WorkerHandler)
            {
                this.handler = handler;
            }
            else
            {
                throw new ZBusException("handler invalid");
            }
            
        }
        
        public void Run()
        {
            Connection connection = new Connection(this.connCfg);
            Worker worker = new Worker(connection, this.workerCfg);

            if (this.handler is ServiceHandler)
            {
                ServiceHandler serviceHandler = this.handler as ServiceHandler;
                while (true)
                {
                    ZMsg request = worker.Recv();
                    if (request == null) break;//interrupted
                    ZMsg reply = serviceHandler.HandleRequest(request);
                    if (reply != null)
                    {
                        worker.Send(reply);
                    }
                }
            }
            else if (this.handler is WorkerHandler)
            {
                WorkerHandler workerHanlder = this.handler as WorkerHandler;
                workerHanlder.HandleWorker(worker);
            }
            else
            {
                throw new ZBusException("handler invalid");
            }
            worker.Dispose();
            connection.Dispose();
        }
    }

    public class WorkerPoolConfig { 
	    public static readonly String MODE_LB = "1";
        public static readonly string MODE_PUBSUB = "2";
        public static readonly String MODE_BC = "3";
	
	    //Connection config 
	    public string[] Brokers = {"127.0.0.1:15555"};
        public bool Verbose = false;
        public ZContext Ctx; //zeromq context, optional 

	    //Worker config
        public string Service;// required
        public string Mode = MODE_LB;
        public string RegisterToken = "";
        public string AccessToken = "";
	}


    public class WorkerPool{
        protected WorkerPoolConfig config;  
        public WorkerPool(WorkerPoolConfig config)
        {
            this.config = config;
        } 
         
        protected void RunHandler(int threadCount, object handler){
            if (config.Service == null) {
                throw new ZBusException("service required for config");
            }
            if( !(handler is ServiceHandler) && !(handler is WorkerHandler))
            { 
                throw new ZBusException("handler invalid");
            }
           
            ZContext ctx = null;
            if (config.Ctx == null)
            {
                ctx = new ZContext(1); 
                config.Ctx = ctx;
            }

            List<Thread> threads = new List<Thread>();
            foreach(string broker in this.config.Brokers){
                ConnectionConfig connCfg = new ConnectionConfig();
                WorkerConfig workerCfg = new WorkerConfig();
                string[] parts = broker.Split(':');
                if(parts.Length != 2){
                    string msg = string.Format("broker invalid: {0}", broker);
                    throw new ZBusException(msg);
                }
                connCfg.Host = parts[0];
                connCfg.Port = Convert.ToInt32(parts[1]);
                connCfg.Verbose = this.config.Verbose;
                connCfg.Ctx = ctx;

                workerCfg.Service = this.config.Service;
                workerCfg.Mode = this.config.Mode;
                workerCfg.RegisterToken = this.config.RegisterToken;
                workerCfg.AccessToken = this.config.AccessToken;


                for(int i=0; i<threadCount; i++){
                    WorkerThreadStart start = new WorkerThreadStart(connCfg, workerCfg, handler);
                    Thread thread = new Thread(new ThreadStart(start.Run));
                    threads.Add(thread);
                }
            }

            foreach (Thread thread in threads)
            {
                thread.Start();
            }
            foreach (Thread thread in threads)
            {
                thread.Join();
            }

            if (ctx != null)
            {
                ctx.Dispose();
            }
        }

        public void Run(int threadCount, ServiceHandler handler)
        {
            this.RunHandler(threadCount, handler);
        }

        public void Run(int threadCount, WorkerHandler handler)
        {
            this.RunHandler(threadCount, handler);
        }
    }
}
