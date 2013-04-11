package net.zbus;

import java.util.ArrayDeque;
import java.util.Iterator;
 
public class ZMsg{ 
	private ArrayDeque<byte[]> frames = new ArrayDeque<byte[]>();
	private long contentSize = 0;
	
	public void pushBack(byte[] frame){ 
		this.frames.add(frame);
		this.contentSize += frame.length;
	}
	
	public void pushBack(String frame){
		this.pushBack(frame.getBytes());
	}
	
	public void pushFront(byte[] frame){
		this.frames.push(frame);
		this.contentSize += frame.length;
	}
	
	public void pushFront(String frame){
		this.pushFront(frame.getBytes());
	}
	
	public byte[] popFront(){
		byte[] data = this.frames.pollFirst();
		if(data == null) return null;
		
		this.contentSize -= data.length;
		return data;
	}
	
	public byte[] popBack(){
		byte[] data = this.frames.pollLast();
		if(data == null) return null;
		
		this.contentSize -= data.length;
		return data;
	}
	
	public String popFrontStr(){
		byte[] data = this.popFront();
		if(data == null) return null;
		return new String(data);
	}
	
	public String popBackStr(){
		byte[] data = this.popBack();
		if(data == null) return null;
		return new String(data); 
	}
	
	public int frameSize(){
		return this.frames.size();
	}
	
	public long contentSize(){
		return this.contentSize;
	}
	
	private String strhex(byte[] data, int offset, int len) {		
		String hexChar = "0123456789ABCDEF";
		StringBuilder b = new StringBuilder();
		for (int nbr = offset;nbr<len;nbr++) {
			int b1 = data[nbr] >>> 4 & 0xf;
			int b2 = data[nbr] & 0xf;
			b.append(hexChar.charAt(b1));
			b.append(hexChar.charAt(b2));
		}
		return b.toString();
	}
	
	private String frameString(byte[] data){
		boolean isText = true;
		int maxSize = data.length;
		if(maxSize > 128) maxSize = 128;
		for (int i = 0;i<maxSize;i++) {
			if (data[i] < 32 || data[i] > 127)
				isText = false;
		}
		if (isText) 
			return new String(data, 0, maxSize);
		else
			return strhex(data, 0, maxSize);
	}
	
	public void dump(){
		System.out.println("---------------------------------");
		for(byte[] data : this.frames){
			System.out.printf("[%03d] %s\n", data.length, frameString(data));
		}
	}
	
	public Iterator<byte[]> iterator(){
		return this.frames.iterator();
	}
	
	public void clear(){
		this.frames.clear();
		this.contentSize = 0;
	}
	public byte[] getLast(){
		return this.frames.getLast();
	}
	public byte[] getFirst(){
		return this.frames.getFirst();
	}
}
