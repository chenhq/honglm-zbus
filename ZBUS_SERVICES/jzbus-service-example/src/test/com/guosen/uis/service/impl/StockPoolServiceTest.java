package com.guosen.uis.service.impl;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import com.guosen.uis.SpringAwareTestCase;
import com.guosen.uis.cond.StockPoolQuery;
import com.guosen.uis.dto.PageResult;
import com.guosen.uis.dto.SimpleResult;
import com.guosen.uis.dto.StockRecordDto;
import com.guosen.uis.service.StockPoolService;
import com.guosen.uis.util.ConstValues;

public class StockPoolServiceTest extends SpringAwareTestCase {

	private StockPoolService stockPoolService ;

	public StockPoolService getStockPoolService() {
		return stockPoolService;
	}

	public void setStockPoolService(StockPoolService stockPoolService) {
		this.stockPoolService = stockPoolService;
	}
	
	@SuppressWarnings("rawtypes")
	public void testCreateStockPool()
	{
		StockRecordDto dto = new StockRecordDto();
		dto.setAdvice("TEST UPDATE!!!!!!!") ;
		dto.setBuyDate(new Date()) ;
		dto.setSellDate(new Date()) ;
		dto.setPrdLmId("3807") ;
		dto.setCreator("admin_beijing") ;
		dto.setClientMsgSend(true) ;
		dto.setStockStr("SZA-000031-中粮地产") ;
		
		SimpleResult rst = this.stockPoolService.createStockPool(dto) ;
		System.out.println(rst.getRtnMsg()) ;
    	System.out.println(rst.getRtnMsg()) ;
	}
	
	@SuppressWarnings("rawtypes")
	public void passUpdateStockPool()
	{
		StockRecordDto dto = new StockRecordDto();
		dto.setSid("872") ;
		dto.setAdvice("TEST UPDATE!!!!!!!") ;
		dto.setStockStr("SZA-000031-中粮地产") ;
		dto.setBuyDate(new Date()) ;
		dto.setSellDate(new Date()) ;
		dto.setPrdLmId("12") ;
		dto.setCreator("zhangjie") ;
		dto.setModifier("zhangsan") ;
		dto.setClientMsgSend(true) ;
		SimpleResult rst = this.stockPoolService.updateStockPool(dto) ;
		System.out.println(rst.getRtnMsg()) ;
    	System.out.println(rst.getRtnMsg()) ;
	}
	
	@SuppressWarnings("rawtypes")
	public void passDelStockPool()
	{
		List<String> ls = new ArrayList<String>() ;
		ls.add("872") ;
/*		ls.add("4") ;
		ls.add("5") ;*/
		String opUsername = "zhangsan" ;
		
		SimpleResult rst = this.stockPoolService.delStockPool(ls, opUsername) ;
		System.out.println(rst.getRtnMsg()) ;
    	System.out.println(rst.getRtnMsg()) ;
	}
	
	
	public void passGetStockPool()
	{
		
		SimpleResult<StockRecordDto> rst = this.stockPoolService.getStockPool("3") ;
		System.out.println(rst.getRtnMsg()) ;
    	System.out.println(rst.getRtnMsg()) ;
	}
	
	public void passFindStockPool()
	{
		StockPoolQuery cond = new StockPoolQuery() ;
		cond.setRecordId("7") ;
		cond.setPageNo(1) ;
		cond.setPageSize(5) ;
		
		PageResult<StockRecordDto> page = this.stockPoolService.findStockRecords(cond) ;
		System.out.println(page.getRtnMsg()) ;
    	System.out.println(page.getRtnMsg()) ;
	}
	
	@SuppressWarnings("rawtypes")
	public void passupdateStockPoolStatus()
	{
		String sid = "872" ;
		String statusCode = ConstValues.DICT_CODE_INFO_STATUS_TH ;
		String opUser ="zhangsan" ;
		List<String> sids = new ArrayList<String>() ;
		sids.add(sid) ;
		
		SimpleResult rst = this.stockPoolService.updateStockPoolStatus(sids, statusCode, opUser) ;
		
		System.out.println(rst.getRtnMsg()) ;
    	System.out.println(rst.getRtnMsg()) ;
	}
}
