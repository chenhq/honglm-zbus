package com.guosen.uis.service.impl;

import org.apache.log4j.Logger;

import com.guosen.uis.SpringAwareTestCase;
import com.guosen.uis.engine.PushEngine;

public class PushEngineImplTest extends SpringAwareTestCase {

	private static Logger logger = Logger.getLogger(PushEngineImplTest.class) ;
	private PushEngine pushEngine ;

	
	public void passGetUserMobileByColumnId()
	{
/*		String columnId = "3562" ;
		Set<String> result = this.pushEngine.getUserMobileByColumnId(columnId) ;
		
		System.out.println(result.size()) ;*/
		
	}


	public void testsendProductSmsInfo()
	{
		String infoId = "208" ;
		
		try{
			this.pushEngine.sendProductSmsInfo(infoId) ;
		}catch (Exception e) {
			logger.error(e.getMessage() , e
					 ) ;
		}
		
		
	}
	public PushEngine getPushEngine() {
		return pushEngine;
	}


	public void setPushEngine(PushEngine pushEngine) {
		this.pushEngine = pushEngine;
	}
	
}
