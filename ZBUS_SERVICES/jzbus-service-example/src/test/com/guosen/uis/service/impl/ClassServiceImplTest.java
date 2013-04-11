package com.guosen.uis.service.impl;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import com.guosen.uis.SpringAwareTestCase;
import com.guosen.uis.cond.CclassQuery;
import com.guosen.uis.dto.ClassDto;
import com.guosen.uis.dto.ListResult;
import com.guosen.uis.dto.PageResult;
import com.guosen.uis.dto.SimpleResult;
import com.guosen.uis.dto.UsedColumnDto;
import com.guosen.uis.dto.UserInfo;
import com.guosen.uis.service.ClassService;

public class ClassServiceImplTest extends SpringAwareTestCase {

    private ClassService classService;

    public ClassService getClassService() {
        return classService;
    }

    public void setClassService(ClassService classService) {
        this.classService = classService;
    }
    
  public void passFindUserPubCol() {
	  CclassQuery cond = new CclassQuery();
	  cond.setUsername("admin_beijing");
	  cond.setClassID(0);
	  cond.setPageNo(1);
	  cond.setPageSize(10);
	  
	  PageResult<UsedColumnDto> r = classService.findUserPubCol(cond);
	  System.out.println(JSONObject.toJSON(r));
}
    
//    public void testfindAdminPubCol() {
//        CclassQuery cond = new CclassQuery();
//        cond.setOrgId("F010");
//        cond.setUsername("admin_beijing");
//        
//       PageResult<UsedColumnDto> r = classService.findAdminPubCol(cond);
//       System.out.println(JSONObject.toJSON(r));
//    }
    
    public void passQueryClass() {
        CclassQuery cond = new CclassQuery();
//        cond.setClassID(1511);
        cond.setDefaultOrg("F010");
//        cond.setClassID(10);       
//        cond.setClassName("基金投资");
//        cond.setClassStatus("0");
//        cond.setClassTypeCode("1");
//        cond.setInvSug("0");
//        cond.setStartDate("2012-12-04");
//        cond.setConfirmer("beijing_caifu");

        cond.setClassPubTypeCode("9");
        cond.setPublisher("admin_beijing");
        PageResult<ClassDto> r = classService.queryClass(cond);
        System.out.println(JSONObject.toJSON(r));
        
    }
    
    public void passAddClass() {
        ClassDto dto = new ClassDto();
        dto.setClassName("测试栏目发布人1");
        dto.setStatus("0");
        dto.setDesc("测试栏目发布人1");
        dto.setClassPubTypeCode("10");
        dto.setClassPubTypeID(0) ;
        dto.setClassTypeCode("4");
        dto.setClassTypeID(0) ;
        dto.setConfirmer("admin_beijing,beijing_caifu,beijing_kefu") ;
        dto.setCreator("admin_beijing") ;
        dto.setInvestAdvice("0") ;
        dto.setOrgCode("F010");
        dto.setPublisher("admin_beijing,beijing_caifu,beijing_kefu") ;
        dto.setTotalCount(0) ;
        
        @SuppressWarnings("rawtypes")
		SimpleResult r = classService.addClass(dto);
        
        System.out.println(JSONObject.toJSON(r));
        
    }
    
  public void passAddUsedCol() {
	  SimpleResult r = classService.addUsedCol("admin_shanghai","1204");
	  System.out.println(JSONObject.toJSON(r));
}
    public void testGetConfirmers()
    {
    	int prdLmId = 1879 ;
    	
    	ListResult<UserInfo> rs = this.classService.getClassConfirmers(prdLmId) ;
    	
    	System.out.println(JSON.toJSON(rs));
    }
//    
//    public void testDeleteClass() {
//      SimpleResult r = classService.deleteClass(3726);
//      
//      System.out.println(JSONObject.toJSON(r));
//    }
//    

//    
//    public void testGetUsedColumn() {
//        CclassQuery cond = new CclassQuery();
//        cond.setUsername("admin_shanghai1");
//        cond.setPageSize(1);
//        PageResult<UsedColumnDto> r = classService.getUsedColumn(cond);
//        System.out.println(JSONObject.toJSON(r));
//    }
//    
//    public void testCancalUsedCol() {
//        SimpleResult r = classService.cancalUsedCol("2");
//        System.out.println(JSONObject.toJSON(r));
//    }
//    
//    public void testFindUserPubCol() {
//        CclassQuery cond = new CclassQuery();
//        cond.setUsername("beijing_caifu");
//        PageResult<UsedColumnDto> r = classService.findUserPubCol(cond);
//        System.out.println(JSONObject.toJSON(r));
//    }
//    
//    public void testUpDownUsedCol() {
//        SimpleResult r = classService.upDownUsedCol("56", "1");
//        System.out.println(JSONObject.toJSON(r));
//    }
//    
//    public void testHasConfirmer() {
//        System.out.println(classService.hasConfirmer(660));
//    }
//    
    public void passmodifyClass() {
        ClassDto dto = new ClassDto();
        dto.setClassID(3726);
        dto.setClassName("测试栏目发布人1_modify");
        dto.setStatus("0");
        dto.setDesc("测试栏目发布人1");
        dto.setClassPubTypeCode("10");
        dto.setClassPubTypeID(0) ;
        dto.setClassTypeCode("4");
        dto.setClassTypeID(0) ;
        dto.setConfirmer("admin_beijing,beijing_caifu,beijing_kefu") ;
        dto.setCreator("admin_beijing") ;
        dto.setInvestAdvice("0") ;
        dto.setOrgCode("F010");
        dto.setPublisher("admin_beijing,beijing_caifu,beijing_kefu") ;
        dto.setTotalCount(0) ;
        
        SimpleResult r = classService.modifyClass(dto);
        System.out.println(JSONObject.toJSON(r));
    }
     
    
}
