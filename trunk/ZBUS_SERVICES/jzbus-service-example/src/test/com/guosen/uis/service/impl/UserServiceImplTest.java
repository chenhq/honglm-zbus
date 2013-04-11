package com.guosen.uis.service.impl;

import com.alibaba.fastjson.JSONObject;
import com.guosen.uis.SpringAwareTestCase;
import com.guosen.uis.cond.InfoQuery;
import com.guosen.uis.dto.InfoAbbrDto;
import com.guosen.uis.dto.PageResult;
import com.guosen.uis.service.UserService;

public class UserServiceImplTest extends SpringAwareTestCase {

    private UserService userService;

    public UserService getUserService() {
        return userService;
    }

    public void setUserService(UserService userService) {
        this.userService = userService;
    }

    // public void testCountInfo() {
    // SimpleResult<Integer> r =
    // userService.countInfo(ConstValues.DICT_CODE_INFO_STATUS_TH,
    // "admin_info");
    // System.out.println(JSONObject.toJSON(r));
    // }
    //    
//     public void testCountToAuth() {
//     SimpleResult<Integer> r = userService.countToAuth("beijing_caifu");
//     System.out.println(JSONObject.toJSON(r));
//     }
    //    
//     public void testGetUserInfo() {
//     SimpleResult<UserInfo> r = userService.getUserInfo("admin_beijing");
//     System.out.println(JSONObject.toJSON(r));
//     }
        
//     public void testFindAuthInfoList() {
//     InfoQuery cond = new InfoQuery();
//     cond.setUsername("admin_beijing");
//     cond.setColumnID(1578);
//     cond.setStartDate(Calendar.getInstance().getTime());
//     PageResult<InfoAbbrDto> r = userService.findAuthInfoList(cond);
//     System.out.println(JSONObject.toJSON(r));
//     }

    public void testgetUserInfoList() {
        InfoQuery cond = new InfoQuery();
        cond.setUsername("beijing_caifu");

        PageResult<InfoAbbrDto> r = userService.getUserInfoList(cond);
        System.out.println(JSONObject.toJSON(r));
    }
//    
//    public void testauth() {
//        List<String> l = new ArrayList<String>();
//        l.add("4121989");
//        SimpleResult r = userService.authInfo("admin_beijing", l, "2", "2");
//        System.out.println(JSONObject.toJSON(r));
//    }
}
