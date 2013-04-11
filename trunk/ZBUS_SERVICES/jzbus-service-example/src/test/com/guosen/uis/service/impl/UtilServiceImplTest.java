package com.guosen.uis.service.impl;

import com.alibaba.fastjson.JSONObject;
import com.guosen.uis.SpringAwareTestCase;
import com.guosen.uis.dto.DictDto;
import com.guosen.uis.dto.ListResult;
import com.guosen.uis.dto.OrgDto;
import com.guosen.uis.dto.UserInfo;
import com.guosen.uis.service.UtilService;

public class UtilServiceImplTest extends SpringAwareTestCase {

    private UtilService utilService;

    public UtilService getUtilService() {
        return utilService;
    }

    public void setUtilService(UtilService utilService) {
        this.utilService = utilService;
    }
    
//    public void testgetColPubType() {
//        ListResult<DictDto> r = utilService.getColPubType();
//        System.out.println(JSONObject.toJSON(r));
//    }
//    
//    public void testgetManagedBranches() {
//        System.out.println(JSONObject.toJSON(utilService.getManagedBranches("4100")));
//    }
//    
//    
//    public void testgetColType() {
//        System.out.println(JSONObject.toJSON(utilService.getColType()));
//    }
//    
    public void testgetAuthBranches() {
    	ListResult<OrgDto> rst = utilService.getAuthBranches("S003") ;
    	System.out.println(rst.getList().size());
        System.out.println(JSONObject.toJSON(rst));
    }
    
    public void passgetOrgInfoAdmin() {
    	ListResult<UserInfo> result = this.utilService.getOrgInfoAdmin("6100") ;
    	
        System.out.println(JSONObject.toJSON(result));
    }
}
