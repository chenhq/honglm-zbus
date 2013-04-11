package com.guosen.uis.service.impl;

import com.alibaba.fastjson.JSONObject;
import com.guosen.uis.SpringAwareTestCase;
import com.guosen.uis.dto.GroupDto;
import com.guosen.uis.dto.PageResult;
import com.guosen.uis.dto.SimpleResult;
import com.guosen.uis.service.GroupService;

public class GroupServiceImplTest extends SpringAwareTestCase {

    private GroupService groupService;

    public GroupService getGroupService() {
        return groupService;
    }

    public void setGroupService(GroupService groupService) {
        this.groupService = groupService;
    }
    
//    public void testcreateGroup() {
//        GroupDto dto = new GroupDto();
//        dto.setGroupName("测试分组");
//        dto.setGroupDesc("描述");
//        dto.setGroupMembers("13317456874,1377432324,15902038271");
//        dto.setAuthUsers("admin_beijing,admin_nanjing");
//        dto.setOwner("admin_dalian");
//        SimpleResult r = groupService.createGroup(dto);
//        System.out.println(JSONObject.toJSON(r));
//    }
//    
//    public void testdeleteGroup() {
//        SimpleResult r = groupService.deleteGroup("5");
//        System.out.println(JSONObject.toJSON(r));
//    }
//    
//    public void testfindOwnerGroup() {
//        PageResult<GroupDto> r = groupService.findOwnerGroup("admin_dalian",1,10);
//        System.out.println(JSONObject.toJSON(r));
//    }
//    
    public void testfindAuthGroup() {
        PageResult<GroupDto> r = groupService.findAuthGroup("admin_beijing",1,10);
        System.out.println(JSONObject.toJSON(r));
    }
//    
//    public void testmodifyGroup() {
//        GroupDto dto = new GroupDto();
//        dto.setGroupId("6");
//        dto.setGroupName("测试分组1");
//        dto.setGroupDesc("描述1");
//        dto.setGroupMembers("1,2,2");
//        dto.setAuthUsers("a,b");
//        dto.setOwner("admin_dalian1");
//        SimpleResult r = groupService.modifyGroup(dto);
//        System.out.println(JSONObject.toJSON(r));
//        
//    }
}
