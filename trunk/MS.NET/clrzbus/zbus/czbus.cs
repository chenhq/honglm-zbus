using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;

namespace zbus {
    internal static class C
    {

        private const CallingConvention PlatformConvention = CallingConvention.Cdecl;
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern IntPtr zmalloc(int size); 
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern void zfree(IntPtr ptr);

        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern IntPtr zmq_ctx_new(int io_threads); 
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern int zmq_ctx_destroy(IntPtr context);


        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern void zlog_use_stdout();
        [DllImport("libzbus", CallingConvention = PlatformConvention)] 
        public static extern void zlog_use_file([MarshalAs(UnmanagedType.LPStr)] string path);


        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern IntPtr zframe_new(IntPtr data, int size);
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern void zframe_destroy(out IntPtr self_p);
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern IntPtr zmq_msg_data(IntPtr self);
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern int zmq_msg_size(IntPtr self);

        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern IntPtr zmsg_new();
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern IntPtr zmsg_destroy(out IntPtr self_p);
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern void zmsg_push_back(IntPtr self, IntPtr frame);
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern void zmsg_push_front(IntPtr self, IntPtr frame);
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern IntPtr zmsg_pop_back(IntPtr self);
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern IntPtr zmsg_pop_front(IntPtr self);


        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern IntPtr zbusconn_new(IntPtr cfg);
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern void zbusconn_destroy(out IntPtr self_p);
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern int zbusconn_reconnect(IntPtr self);
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern int zbusconn_send(IntPtr self, IntPtr msg);
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern IntPtr zbusconn_recv(IntPtr self, int timeout);
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern int zbusconn_route(IntPtr self, IntPtr sock_id, IntPtr msg);
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern int zbusconn_probe(IntPtr self, int timeout);

        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern IntPtr zbuscli_request(IntPtr self, [MarshalAs(UnmanagedType.LPStr)] string service, 
            [MarshalAs(UnmanagedType.LPStr)] string token, IntPtr msg, int timeout);
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern IntPtr zbuscli_send(IntPtr self, IntPtr ctrl, IntPtr msg);
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern IntPtr zbuscli_recv(IntPtr self, int pingInterval);



        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern IntPtr zbuswrk_new(IntPtr cfg);
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern void zbuswrk_destroy(out IntPtr self_p);
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern int zbuswrk_send(IntPtr conn, IntPtr worker, IntPtr msg);
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern IntPtr zbuswrk_recv(IntPtr conn, IntPtr worker);
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern int zbuswrk_subscribe(IntPtr conn, IntPtr worker, [MarshalAs(UnmanagedType.LPStr)] string topic);
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern int zbuswrk_unsubscribe(IntPtr conn, IntPtr worker, [MarshalAs(UnmanagedType.LPStr)] string topic);
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern IntPtr zbuswrk_get_address(IntPtr worker, out IntPtr sock_id, out IntPtr msg_id);
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern IntPtr zbuswrk_set_address(IntPtr worker, IntPtr sock_id, IntPtr msg_id);


        //helper functions
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern IntPtr zbusconn_cfg_new([MarshalAs(UnmanagedType.LPStr)] string host,
            int port, int verbose, IntPtr ctx, [MarshalAs(UnmanagedType.LPStr)] string id);
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern IntPtr zbuswrk_cfg_new([MarshalAs(UnmanagedType.LPStr)] string service,
            [MarshalAs(UnmanagedType.LPStr)] string mode, [MarshalAs(UnmanagedType.LPStr)] string reg_token, 
            [MarshalAs(UnmanagedType.LPStr)] string acc_token);
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern IntPtr asyn_ctrl_new([MarshalAs(UnmanagedType.LPStr)] string service,
            [MarshalAs(UnmanagedType.LPStr)] string token, int timeout,
            [MarshalAs(UnmanagedType.LPStr)] string peer_id, [MarshalAs(UnmanagedType.LPStr)] string msg_id);
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern void zbusconn_cfg_destroy(out IntPtr self_p);
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern void zbuswrk_cfg_destroy(out IntPtr self_p);
        [DllImport("libzbus", CallingConvention = PlatformConvention)]
        public static extern void asyn_ctrl_destroy(out IntPtr self_p);
       
    }
}
