using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;
using System.Threading;
using System.Runtime.InteropServices;
 
namespace zbus
{
    public class ConnectionConfig{
        public string Host = "127.0.0.1"; 
        public int Port = 15555; 
        public bool Verbose = false;
        public string Id = "";
        public ZContext Ctx;
    }

    public class AsynCtrl
    {
        public string Service;
        public string Token = "";
        public int Timeout = 2500; //ms
        public string PeerId = "";
        public string MessageId = "";
    }


    public class Connection : IDisposable
    {
        private ThreadLocal<ConnectionUnsafe> connection;

        public Connection(ConnectionConfig config)
        {
            this.connection = new ThreadLocal<ConnectionUnsafe>(() =>
            {
                return new ConnectionUnsafe(config); 
            });
        }

        public int Reconnect()
        {
            return this.connection.Value.Reconnect();
        }

        public int Send(ZMsg msg)
        {
            return this.connection.Value.Send(msg);
        }

        public ZMsg Recv(int timeout)
        {
            return this.connection.Value.Recv(timeout);
        }

        public int Route(byte[] sockId, ZMsg msg)
        {
            return this.connection.Value.Route(sockId, msg);
        }

        public int Probe(int timeout = 2500)
        {
            return this.connection.Value.Probe(timeout);
        }

        ~Connection()
        {
            Dispose();
        }

        public void Dispose()
        {
            this.Destroy();
        }

        public void Destroy()
        {
            if (this.connection != null)
            {
                this.connection.Dispose();
            } 
        }

        public IntPtr Handle { get { return this.connection.Value.Handle; } }
    }

    public class BusClient : IDisposable
    {
        private Connection connection;
        private bool ownConnection = false;

        public BusClient(Connection connection)
        {
            this.connection = connection;
            this.ownConnection = false;
        }

        public BusClient(ConnectionConfig config)
        {
            this.connection = new Connection(config);
            this.ownConnection = true;
        }


        public ZMsg Request(string service, string token, ZMsg msg, int timeout=2500)
        {
            IntPtr c_msg = C.zmsg_new();
            IEnumerator<byte[]> fe = msg.GetEnumerator();
            while (fe.MoveNext())
            {
                IntPtr c_frame = ZMsg.ToZFramePtr(fe.Current);
                C.zmsg_push_back(c_msg, c_frame);
            }

            IntPtr res = C.zbuscli_request(this.connection.Handle, service, token, c_msg, timeout);
            if (res == IntPtr.Zero)
            {
                throw new ZBusException("request timeout");
            }

            ZMsg result = null;
            if (res != IntPtr.Zero)
            {
                result = ZMsg.ToZMsg(res);
            }

            return result;
        }

        public ZMsg Send(AsynCtrl ctrl, ZMsg msg)
        {    
            IntPtr c_msg = C.zmsg_new();
            IEnumerator<byte[]> fe = msg.GetEnumerator();
            while (fe.MoveNext())
            {
                IntPtr c_frame = ZMsg.ToZFramePtr(fe.Current);
                C.zmsg_push_back(c_msg, c_frame);
            }

            IntPtr c_ctrl = C.asyn_ctrl_new(ctrl.Service, ctrl.Token, ctrl.Timeout, ctrl.PeerId, ctrl.MessageId);
            IntPtr res = C.zbuscli_send(this.connection.Handle, c_ctrl, c_msg);
            C.asyn_ctrl_destroy(out c_ctrl);

            if (res == IntPtr.Zero)
            {
                throw new ZBusException("send timeout");
            }

            ZMsg result = null;
            if (res != IntPtr.Zero)
            {
                result = ZMsg.ToZMsg(res);
            }

            return result;
        }

        public ZMsg Recv(int pingInterval)
        {
            IntPtr res = C.zbuscli_recv(this.connection.Handle, pingInterval);

            ZMsg result = null;
            if (res != IntPtr.Zero)
            {
                result = ZMsg.ToZMsg(res);
            }

            return result;
        }

        public bool Publish(string service, string token, ZMsg msg, int timeout=2500)
        {
            AsynCtrl ctrl = new AsynCtrl();
            ctrl.Service = service;
            ctrl.Token = token;
            ctrl.Timeout = timeout;

            ZMsg res = this.Send(ctrl, msg);
            string status = res.PopFrontStr();
            if (status != null && status.Equals("200"))
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        ~BusClient()
        {
            Dispose();
        }

        public void Dispose()
        {
            this.Destroy();
        }
        public void Destroy()
        {
            if (this.ownConnection && this.connection != null)
            {
                this.connection.Dispose();
            }
        }

    }


    class ConnectionUnsafe : IDisposable 
    {
        private IntPtr _handle;
        public IntPtr Handle { get { return _handle; } }

        public ConnectionUnsafe(ConnectionConfig config)
        {
            ZContext ctx = config.Ctx;
            if(ctx == null){
                ctx = ZContext.Instance(1);
            }
            IntPtr c_cfg = C.zbusconn_cfg_new(config.Host, config.Port, config.Verbose ? 1 : 0, 
                ctx.Handle, config.Id);
            this._handle = C.zbusconn_new(c_cfg);
            C.zbusconn_cfg_destroy(out c_cfg);
        }

        public int Reconnect(){
            return C.zbusconn_reconnect(this._handle); 
        }

        public int Send(ZMsg msg){
            IntPtr c_msg = C.zmsg_new();
            IEnumerator<byte[]> fe = msg.GetEnumerator();
            while (fe.MoveNext())
            {
                IntPtr c_frame = ZMsg.ToZFramePtr(fe.Current);
                C.zmsg_push_back(c_msg, c_frame);
            }
            int res = C.zbusconn_send(this._handle, c_msg);
            return res;
        }

        public ZMsg Recv(int timeout){
            IntPtr res = C.zbusconn_recv(this._handle, timeout);

            ZMsg result = null;
            if (res != IntPtr.Zero)
            {
                result = ZMsg.ToZMsg(res);
            }

            return result;
        }

        public int Route(byte[] sockId, ZMsg msg){
            IntPtr c_sock = ZMsg.ToZFramePtr(sockId);
            IntPtr c_msg = C.zmsg_new();
            IEnumerator<byte[]> fe = msg.GetEnumerator();
            while (fe.MoveNext())
            {
                IntPtr frame = ZMsg.ToZFramePtr(fe.Current);
                C.zmsg_push_back(c_msg, frame);
            }
            int res = C.zbusconn_route(this._handle, c_sock, c_msg);
            return res;
        }

        public int Probe(int timeout = 2500)
        {
            return C.zbusconn_probe(this._handle, timeout);
        }

        ~ConnectionUnsafe()
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
                C.zbusconn_destroy(out this._handle);
            }
        }

    }
}
