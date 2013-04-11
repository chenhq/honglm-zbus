using System;
using System.Collections.Generic;
using System.Diagnostics;
 
namespace zbus { 
    public class ZBusException : Exception{
        public ZBusException(string message): base(message){ }
        public ZBusException(): base(){}
        public ZBusException(string message, Exception ex): base(message, ex){}
    }
    /// <summary>
    /// ZMQ Context
    /// </summary>
    public class ZContext : IDisposable {
        private static ZContext instance;

        private static int _defaultIOThreads = 1;
        private IntPtr _handle; 
        
        /// <summary>
        /// Create ZMQ Context
        /// </summary>
        /// <param name="io_threads">Thread pool size</param>
        public ZContext(int io_threads) { 
            _handle = C.zmq_ctx_new(io_threads);
            if (Handle == IntPtr.Zero)
                throw new Exception("zmq context init failed");
        }

        public ZContext() {

            _handle = C.zmq_ctx_new(_defaultIOThreads);
            if (_handle == IntPtr.Zero)
                throw new Exception("zmq context init failed");
        }

        ~ZContext() {
            Dispose(false);
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (_handle != IntPtr.Zero)
            {
                int rc = C.zmq_ctx_destroy(_handle);
                _handle = IntPtr.Zero;
                if (rc != 0)
                    throw new Exception("zmq context term fialed");
            }
        }

        public static int DefaultIOThreads
        {
            get { return _defaultIOThreads; }
            set { _defaultIOThreads = value; }
        }
        public IntPtr Handle { get { return _handle; } }

        public static ZContext Instance(int ioThreads){
            if(instance == null){
                instance = new ZContext(ioThreads);
            }
            return instance;
        }

        public static void ZLogUseStdout(){
            C.zlog_use_stdout();
        }

        public static void ZLogUseFile(string path)
        { 
            C.zlog_use_file(path);
        }
    }
}
