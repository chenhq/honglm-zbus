package com.guosen.uis.service.impl;

import java.util.Date;

import com.guosen.uis.SpringAwareTestCase;
import com.guosen.uis.cond.StockRecordQuery;
import com.guosen.uis.dto.FinRecordDto;
import com.guosen.uis.dto.PageResult;
import com.guosen.uis.dto.SimpleResult;
import com.guosen.uis.service.FinancePrdService;

public class FinancePrdServiceImplTest extends SpringAwareTestCase {
	
	private FinancePrdService finService;

	public FinancePrdService getFinService() {
		return finService;
	}

	public void setFinService(FinancePrdService finService) {
		this.finService = finService;
	}

    @SuppressWarnings("rawtypes")
	public void passCreateFinRecord()
    {
    	FinRecordDto dto = new FinRecordDto() ;
    	dto.setApplyCode("TEST") ;
    	dto.setSecuId("JY11243") ;
    	dto.setName("TEST") ;
    	dto.setCreator("zhangjie") ;
    	dto.setRecommendDate(new Date()) ;
    	dto.setRecommendReason("TEST") ;
    	dto.setInfoId("134") ;
    	dto.setPrdLmId("12") ;
    	SimpleResult rst = this.finService.createFinRecord(dto) ;
    	System.out.println(rst.getRtnMsg()) ;
    }
    
    @SuppressWarnings("rawtypes")
	public void passUpdateFinRecord()
    {
    	FinRecordDto dto = new FinRecordDto() ;
    	dto.setSid("5") ;
    	dto.setApplyCode("TEST1") ;
    	dto.setSecuId("JY11243") ;
    	dto.setName("TEST1") ;
    	dto.setCreator("zhangjie") ;
    	dto.setModifier("zhangsan") ;
    	dto.setRecommendDate(new Date()) ;
    	dto.setRecommendReason("TEST,update") ;
    	dto.setInfoId("1345") ;
    	//dto.setPrdLmId("12") ;
    	SimpleResult rst = this.finService.updateFinRecord(dto) ;
    	System.out.println(rst.getRtnMsg()) ;
    }
    
    @SuppressWarnings("rawtypes")
    public void passRemoveFinRecord()
    {
    	SimpleResult rst = this.finService.removeFinRecord("5", "lisi") ;
    	System.out.println(rst.getRtnMsg()) ;
    	System.out.println(rst.getRtnMsg()) ;
    }
    
    public void passGetFinRecord()
    {
    	SimpleResult<FinRecordDto> rst = this.finService.getFinRecord("5") ;
    	System.out.println(rst.getRtnMsg()) ;
    	System.out.println(rst.getRtnMsg()) ;
    }
    
    public void passFindFinRecords()
    {
    	StockRecordQuery cond = new StockRecordQuery() ;
    	cond.setPageNo(2) ;
    	cond.setPageSize(5) ;
    	
    	PageResult<FinRecordDto> rst = this.finService.findFinRecords(cond) ;
    	
    	System.out.println(rst.getRtnMsg()) ;
    	System.out.println(rst.getRtnMsg()) ;
    	System.out.println(rst.getTotalCount()) ;
    	System.out.println(rst.getTotalPage()) ;
    	
    }
    @SuppressWarnings("rawtypes")
    public void testUpdateFinStatus()
    {
    	String sid = "6" ;
    	String opUser = "lisi" ;
    	String infostatuscode = "6" ;
    	
    	SimpleResult rst = this.finService.updateFinStatus(sid, infostatuscode, opUser) ;
    	System.out.println(rst.getRtnMsg()) ;
    	System.out.println(rst.getRtnMsg()) ;
    			
    			
    }
}