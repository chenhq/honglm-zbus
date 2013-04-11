package com.guosen.uis.service.impl;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import com.guosen.uis.SpringAwareTestCase;
import com.guosen.uis.dto.SimpleResult;
import com.guosen.uis.dto.SmsDto;
import com.guosen.uis.service.SmsService;
import com.guosen.uis.util.ConstValues;

public class SmsServiceTest extends SpringAwareTestCase {

	private SmsService smsService ;
	
	@SuppressWarnings("rawtypes")
	public void passCreateSms()
	{
		SmsDto dto = new SmsDto() ;
		dto.setChannel("1") ;
		dto.setChargeOrgId("4100") ;
		dto.setColumnId(19) ;
		dto.setContent("测试用的短信资讯") ;
		dto.setCreateTime(new Date()) ;
		dto.setDraftFlag(true) ;
		dto.setImportance("2") ;
		dto.setSender("zhangbanxian") ;
		dto.setSendOrgId("4100") ;
		dto.setSendTimeType("1") ;
		
		SimpleResult res = this.smsService.createSms(dto) ;
		System.out.println(res.getRtnMsg());
		
	}

	public void passGetSms()
	{
		SimpleResult<SmsDto> res = this.smsService.getSmsInfoById("123") ;
		System.out.println(res.getRtnMsg());
		System.out.println(res.getR());
	}
	
	@SuppressWarnings("rawtypes")
	public void passUpdatestatus()
	{
		List<String> infoIds = new ArrayList<String>() ;
		infoIds.add("125") ;
		
		String infoStatusCode = ConstValues.DICT_CODE_INFO_STATUS_DFS ;
		
		SimpleResult res = this.smsService.updateSmsStatus(infoIds, infoStatusCode , "zhangjie") ;
		
		System.out.println(res.getRtnMsg());
	}
	
	@SuppressWarnings("rawtypes")
	public void testDelSms()
	{
		SimpleResult res = this.smsService.delSms("125" , "zhangjie") ;
		
		System.out.println(res.getRtnMsg());
	}
	
	@SuppressWarnings("rawtypes")
	public void passUpdateSms()
	{
		SmsDto dto = new SmsDto() ;
		dto.setInfoId(123) ;
		dto.setChannel("1") ;
		dto.setChargeOrgId("4100") ;
		dto.setColumnId(19) ;
		dto.setContent("测试用的短信资讯-更新") ;
		dto.setCreateTime(new Date()) ;
		dto.setDraftFlag(true) ;
		dto.setImportance("2") ;
		dto.setSender("zhangbanxian") ;
		dto.setSendOrgId("4100") ;
		dto.setSendTimeType("1") ;
		
		SimpleResult res = this.smsService.updateSms(dto) ;
		System.out.println(res.getRtnMsg());
	}
	public SmsService getSmsService() {
		return smsService;
	}

	public void setSmsService(SmsService smsService) {
		this.smsService = smsService;
	}
}
