using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using fastJSON;
using System.Reflection;
using System.Collections;

namespace zbus { 

    public class Remote : Attribute
    {
        private string id;

        public Remote(string id)
        {
            this.id = id;
        }

        public Remote()
        {
            this.id = null;
        }

        public string Id
        {
            get { return this.id; }
            set { this.id = value; }
        }
    }


    class MethodInstance
    {
        private MethodInfo method;
        private object instance;

        public MethodInstance(MethodInfo method, object instance)
        {
            this.method = method;
            this.instance = instance;
        }

        public MethodInfo Method
        {
            get { return this.method; }
            set { this.method = value; }
        }

        public object Instance
        {
            get { return this.instance; }
            set { this.instance = value; }
        }
    }

    public class JsonServiceHandler : ServiceHandler
    {

        private Encoding encoding;
        private Dictionary<string, MethodInstance> methods = new Dictionary<string, MethodInstance>();


        public JsonServiceHandler(params object[] services)
        {
            this.Init(Encoding.UTF8, services);
        }

        public JsonServiceHandler(Encoding encoding, params object[] services)
        {
            this.Init(encoding, services);
        }

        private void Init(Encoding encoding, params object[] services)
        {
            this.encoding = encoding;
            foreach (object service in services)
            {
                this.InitCommandTable(service);
            }
        }

        private void InitCommandTable(object service)
        {
            List<Type> types = new List<Type>();
            types.Add(service.GetType());
            foreach (Type type in service.GetType().GetInterfaces())
            {
                types.Add(type);
            }
            foreach (Type type in types)
            {
                foreach (MethodInfo info in type.GetMethods())
                {
                    foreach (Attribute attr in Attribute.GetCustomAttributes(info))
                    {
                        if (attr.GetType() == typeof(Remote))
                        {
                            Remote r = (Remote)attr;
                            string id = r.Id;
                            if (id == null)
                            {
                                id = info.Name;
                            }
                            if (this.methods.ContainsKey(id))
                            {
                                Console.WriteLine("{0} overridden", id);
                                break;
                            }

                            MethodInstance instance = new MethodInstance(info, service);
                            this.methods[id] = instance;
                            break;
                        }
                    }
                }
            }
        }

        public ZMsg HandleRequest(ZMsg request)
        {
            ZMsg reply = new ZMsg();
            reply.PushBack("200"); 
            try
            {
                IEnumerator<byte[]> fe = request.GetEnumerator();
                while(fe.MoveNext()){
                    string res = this.HandleJsonRequest(encoding.GetString(fe.Current));
                    reply.PushBack(encoding.GetBytes(res));
                } 
            }
            catch (System.Exception ex)
            {
                reply.Clear();
                reply.PushBack("500");
                string error = string.Format("Internal Error: {0}", ex.Message);
                reply.PushBack(encoding.GetBytes(error));
            }

            return reply;
        }


        private string HandleJsonRequest(string json)
        {
            System.Exception error = null;
            object result = null;

            string id = "";
            string method = null;
            ArrayList args = null;

            MethodInstance target = null;

            Dictionary<string, object> parsed = null;
            try
            {
                parsed = (Dictionary<string, object>)JSON.Instance.Parse(json);
            }
            catch (System.Exception ex)
            {
                error = ex;
            }
            if (error == null)
            {
                try
                {
                    id = (string)parsed["id"];
                    method = (string)parsed["method"];
                    args = (ArrayList)parsed["params"];
                }
                catch (System.Exception ex)
                {
                    error = ex;
                }
                if (id == null)
                {
                    error = new System.Exception("missing id");
                }
                if (method == null)
                {
                    error = new System.Exception("missing method name");
                }
            }

            if (error == null)
            {
                if (this.methods.ContainsKey(method))
                {
                    target = this.methods[method];
                }
                else
                {
                    error = new System.Exception(method + " not found");
                }
            }

            if (error == null)
            {
                try
                {
                    ParameterInfo[] pinfo = target.Method.GetParameters();
                    if (pinfo.Length == args.Count)
                    {
                        object[] paras = new object[args.Count];
                        for (int i = 0; i < pinfo.Length; i++)
                        {
                            paras[i] = System.Convert.ChangeType(args[i], pinfo[i].ParameterType);
                        }
                        result = target.Method.Invoke(target.Instance, paras);
                    }
                    else
                    {
                        error = new System.Exception("number of argument not match");
                    }
                }
                catch (System.Exception ex)
                {
                    error = ex;
                }
            }

            Dictionary<string, object> data = new Dictionary<string, object>();
            data["id"] = id;
            if (error == null)
            {
                data["error"] = null;
                data["result"] = result;
            }
            else
            {
                data["error"] = error.Message;
                data["result"] = null;
            }

            return JSON.Instance.ToJSON(data);
        }
    }

    public class RpcConfig : ConnectionConfig
    {
        public string Service;
        public string Token = "";
        public int Timeout = 2500;//ms
        public Encoding Encoding = Encoding.UTF8;
    }


    public class Rpc
    {
        private BusClient client;
        private bool ownClient = false;

        private string Service;
        private string Token = "";
        private int timeout = 2500;//ms
        private Encoding Encoding = Encoding.UTF8;

        public Rpc(RpcConfig config)
        {
            this.Encoding = Encoding.UTF8;
            this.Service = config.Service;
            this.Token = config.Token;
            this.client = new BusClient(config);
            this.ownClient = true;
        }

        public Rpc(BusClient client, string service, string token, Encoding encoding, int timeout)
        { 
            this.client = client;
            this.ownClient = false;
            this.Service = service;
            this.Token = token;
            this.Encoding = encoding;
            this.timeout = timeout;
        }


        /// <summary>
        /// Invoke remote procedure via zbus using JSON data format
        /// </summary>
        /// <param name="method">method name to invoke</param>
        /// <param name="args">method arguments</param>
        /// <returns>primitive type or Dictionary<string,object> instance</returns>
        public object Invoke(string method, params object[] args)
        {
            Dictionary<string, object> req = new Dictionary<string, object>();
            req["id"] = "jsonrpc";
            req["method"] = method;
            req["params"] = args;

            string json = JSON.Instance.ToJSON(req);

            ZMsg request = new ZMsg();
            request.PushBack(this.Encoding.GetBytes(json));  
            
            ZMsg result = this.client.Request(this.Service, this.Token, request, this.timeout);

            if (result == null) {
                throw new ZBusException("json rpc request timeout");
            }
            if (result.FrameSize != 2) {
                throw new ZBusException("json rpc result format error");
            }

            Dictionary<string, object> res = null;
            string status = result.PopFrontStr();
            if (status.Equals("200")) {
                try {
                    json = result.PopBackStr(this.Encoding);
                    res = (Dictionary<string, object>)JSON.Instance.Parse(json);
                    if (res.ContainsKey("result")) {
                        return res["result"];
                    } else {
                        throw new ZBusException((string)res["error"]);
                    }
                } catch (System.Exception ex) {
                    throw new ZBusException(ex.Message);
                }
            } else {
                throw new ZBusException(result.PopBackStr(this.Encoding));
            }
        }

        public void Dispose()
        {
            if (this.ownClient)
            {
                this.client.Dispose();
            }
        }
    }
}
