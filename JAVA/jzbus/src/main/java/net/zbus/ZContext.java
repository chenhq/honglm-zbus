package net.zbus;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;

public class ZContext {
	private static ZContext instance;
	
	private static String getCurrentPlatformIdentifier() {
		return "x"+System.getProperty("sun.arch.data.model");
	} 
	
	static {
		String ext = "so"; //default to linux/unix
		String osName = System.getProperty("os.name");
		if (osName.toLowerCase().indexOf("windows") > -1) {
			ext = "dll";
		}
		if (osName.toLowerCase().indexOf("mac") > -1) {
			ext = "dylib";
		}
		String absolutePath = ZContext.class.getProtectionDomain().getCodeSource().getLocation().getPath();
	    absolutePath = absolutePath.substring(0, absolutePath.lastIndexOf("/"));
		String libUrl = absolutePath + "/libjzbus." + ext;  
		File libFile = new File(libUrl); 
		if(libFile.exists()){
			try{
				System.load(libUrl); 
			}catch (Exception e) { 
				System.loadLibrary("libjzbus"); //try system lib
			}
		} else {
			StringBuilder url = new StringBuilder();
			url.append("/NATIVE/");
			url.append(getCurrentPlatformIdentifier());
			url.append("/libjzbus.");    	
			URL nativeLibraryUrl = ZContext.class.getResource(url.toString() + ext);
			if (nativeLibraryUrl != null) { 
				try { 
					final File libfile = File.createTempFile("libjzbus-", ".lib");
					libfile.deleteOnExit(); // just in case
	
					final InputStream in = nativeLibraryUrl.openStream();
					final OutputStream out = new BufferedOutputStream(new FileOutputStream(libfile));
	
					int len = 0;
					byte[] buffer = new byte[8192];
					while ((len = in.read(buffer)) > -1)
						out.write(buffer, 0, len);
					
					out.close();
					in.close();
	
					System.load(libfile.getAbsolutePath());
					
					libfile.delete(); 
					 
				} catch (IOException x) { 
					x.printStackTrace(); 
					System.loadLibrary("libjzbus");
				}  
			} else {
				System.loadLibrary("libjzbus"); //try system lib
			}
		}
	}

	/**
	 * Class constructor.
	 * 
	 * @param ioThreads  size of the threads pool to handle I/O operations.
	 */
	public ZContext(int ioThreads) {
		construct(ioThreads);
	}

	/**
	 * This is an explicit "destructor". It can be called to ensure the
	 * corresponding 0MQ Context has been disposed of.
	 */
	public void destroy() {
		finalize();
	}

	/** Initialize the JNI interface */
	protected native void construct(int ioThreads);

	/** Free all resources used by JNI interface. */
	@Override
	protected native void finalize();

	/**
	 * Get the underlying context handle. This is private because it is only
	 * accessed from JNI, where Java access controls are ignored.
	 * 
	 * @return the internal 0MQ context handle.
	 */
	private long getHandle() {
		return this.handle;
	}

	/** Opaque data used by JNI driver. */
	private long handle;
	
	
	/**
	 * make this method thread safe
	 * @param ioThreads
	 * @return
	 */
	public synchronized static ZContext getInstance(int ioThreads){
		if(instance == null){
			instance = new ZContext(ioThreads);
		}
		return instance;
	}

}
