package com.guosen.uis.service.impl;

import com.alibaba.fastjson.JSON;
import com.guosen.uis.SpringAwareTestCase;
import com.guosen.uis.cond.UserMsgQuery;
import com.guosen.uis.dto.InfoLogDto;
import com.guosen.uis.dto.ListResult;
import com.guosen.uis.dto.MsgDto;
import com.guosen.uis.dto.PageResult;
import com.guosen.uis.dto.SimpleResult;
import com.guosen.uis.service.SysService;
import com.guosen.uis.util.ConstValues;

public class SysServiceImplTest extends SpringAwareTestCase {

	private SysService sysService ;

	public SysService getSysService() {
		return sysService;
	}

	public void setSysService(SysService sysService) {
		this.sysService = sysService;
	}
	
	public void passFindUserMsg()
	{
		UserMsgQuery cond = new UserMsgQuery() ;
		cond.setUsername("admin_beijing") ;
		cond.setNewFlag("2") ;
		
		cond.setPageNo(1) ;
		cond.setPageSize(10) ;
		
		PageResult<MsgDto> result = this.sysService.findUserMsg(cond) ;
		System.out.println(result.getTotalCount());
	}
	
	public void passSaveInfoLog()
	{
		final String opUsername = "admin_beijing";
		final int infoId = 1111 ;
		final int prdLmId = 3830 ;
		String infoStatusCode = "6" ;
		
		SimpleResult<Integer> result = this.sysService.saveInfoLog(opUsername, infoId, prdLmId, ConstValues.DICT_CODE_INFO_STATUS_DSH
				 , infoStatusCode) ;
		
		System.out.println(JSON.toJSON(result)) ;
	}
	
	public void testQueryInfoLog()
	{
		ListResult<InfoLogDto> lst = this.sysService.queryInfoLog(1111) ;
		
		System.out.println(JSON.toJSON(lst)) ;
	}
}
