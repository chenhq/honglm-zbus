using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Runtime.InteropServices;
 
namespace zbus {  
 
    public class ZMsg{
        private LinkedList<byte[]> frames = new LinkedList<byte[]>();
        private long contentSize = 0;

        public ZMsg()
        { 
        }
 
        public void PushBack(byte[] frame){
            frames.AddLast(frame);
            this.contentSize += frame.Length;
        }

        public void PushBack(string frame, Encoding encoding)
        {
            PushBack(encoding.GetBytes(frame));
        }

        public void PushBack(string frame)
        {
            PushBack(frame, Encoding.UTF8);
        }

        public void PushFront(byte[] frame)
        {
            frames.AddFirst(frame);
            this.contentSize += frame.Length;
        }

        public void PushFront(string frame, Encoding encoding)
        {
            PushFront(encoding.GetBytes(frame));
        }

        public void PushFront(string frame)
        {
            PushFront(frame, Encoding.UTF8);
        } 

        public byte[] PopFront(){
            if(frames.First == null){
                return null;
            }
            byte[] frame = frames.First.Value;
            frames.RemoveFirst();
            this.contentSize -= frame.Length;
            return frame;
        }
        public string PopFrontStr(Encoding encoding)
        {
            byte[] frame = PopFront();
            if (frame == null) return null;
            return encoding.GetString(frame); 
        }

        public string PopFrontStr()
        {
            return PopFrontStr(Encoding.UTF8);
        }


        public byte[] PopBack()
        {
            if (frames.Last == null)
            {
                return null;
            }
            byte[] frame = frames.Last.Value;
            frames.RemoveLast();
            this.contentSize -= frame.Length;
            return frame;
        }
        public string PopBackStr(Encoding encoding)
        {
            byte[] frame = PopBack();
            if (frame == null) return null;
            return encoding.GetString(frame); 
        }

        public string PopBackStr()
        { 
            return PopFrontStr(Encoding.UTF8);
        }


        public IEnumerator<byte[]> GetEnumerator()
        {
            return frames.GetEnumerator();
        }

        public int FrameSize {
            get { return this.frames.Count; }
        }

        public long ContentSize{
            get { return this.contentSize; }
        }

        public void Clear(){
            frames.Clear();
            this.contentSize = 0;
        }
        public byte[] First{
            get{
                if (this.frames.First == null) return null;

                return this.frames.First.Value;
            }
        }

        public byte[] Last{
            get{
                if (this.frames.Last == null) return null;

                return this.frames.Last.Value;
            }
        }


        private String strhex(byte[] data, int offset, int len) {		
		    String hexChar = "0123456789ABCDEF";
		    StringBuilder b = new StringBuilder();
		    for (int nbr = offset; nbr<len; nbr++) {
			    int b1 = (data[nbr]>>4) & 0xf;
			    int b2 = data[nbr] & 0xf;
			    b.Append(hexChar[b1]);
                b.Append(hexChar[b2]);
		    }
            return b.ToString();
	    }

        private String frameString(byte[] data)
        {
            bool isText = true;
            int maxSize = data.Length;
            string elipsis = "";
            if (maxSize > 128) {
                maxSize = 128;
                elipsis = "...";
            }
            for (int i = 0; i < maxSize; i++)
            {
                if (data[i] < 32 || data[i] > 127)
                    isText = false;
            }
            if (isText)
                return Encoding.UTF8.GetString(data, 0, maxSize) + elipsis;
            else
                return strhex(data, 0, maxSize)+elipsis;
        }

        public void Dump(){
		    Console.WriteLine("---------------------------------");
		    foreach(byte[] data in this.frames){
			    Console.WriteLine("[{0,3:D3}] {1}", data.Length, frameString(data));
		    }
	    }

        public static byte[] ToZFrame(IntPtr frame)
        {
            int frameSize = C.zmq_msg_size(frame);
            byte[] data = new byte[frameSize];
            Marshal.Copy(C.zmq_msg_data(frame), data, 0, frameSize);

            C.zframe_destroy(out frame);

            return data;
        }


        public static IntPtr ToZFramePtr(byte[] data)
        {
            IntPtr pdata = C.zmalloc(data.Length);

            Marshal.Copy(data, 0, pdata, data.Length);
            IntPtr frame = C.zframe_new(pdata, data.Length);

            C.zfree(pdata);

            return frame;
        }

        public static ZMsg ToZMsg(IntPtr msg)
        {
            ZMsg zmsg = new ZMsg();
            IntPtr frame = C.zmsg_pop_front(msg);
            while (frame != IntPtr.Zero) {
                int frameSize = C.zmq_msg_size(frame);
                byte[] data = new byte[frameSize];
                Marshal.Copy(C.zmq_msg_data(frame), data, 0, frameSize);
                zmsg.PushBack(data);

                C.zframe_destroy(out frame);
                frame = C.zmsg_pop_front(msg);
            }
            C.zmsg_destroy(out msg);
            return zmsg;
        } 

        public static IntPtr ToZMsgPtr(ZMsg msg){
            IntPtr zmsg = C.zmsg_new();
            IEnumerator<byte[]> fe = msg.GetEnumerator();
            
            while(fe.MoveNext()){
                IntPtr framePtr = ToZFramePtr(fe.Current);
                C.zmsg_push_back(zmsg, framePtr);
            }

            return zmsg;
        }
    }
}
