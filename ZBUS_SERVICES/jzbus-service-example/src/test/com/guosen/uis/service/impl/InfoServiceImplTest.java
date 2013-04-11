package com.guosen.uis.service.impl;

import java.util.ArrayList;
import java.util.List;

import com.guosen.uis.SpringAwareTestCase;
import com.guosen.uis.dto.InfoDto;
import com.guosen.uis.service.InfoService;

public class InfoServiceImplTest extends SpringAwareTestCase {

    private InfoService infoService;

    public InfoService getInfoService() {
        return infoService;
    }

    public void setInfoService(InfoService infoService) {
        this.infoService = infoService;
    }
    
    public void testCreateInfo() {
        InfoDto dto = new InfoDto();
        dto.setContent("斯坦福");
        dto.setDesc("1231");
        dto.setDraftFlag(false);
        dto.setInfoTypeCode("4");
        List<String> keywords = new ArrayList<String>();
        keywords.add("23");
        keywords.add("的");
        dto.setKeyWords(keywords);
        dto.setOpOwner("admin_beijing");
        dto.setPrdLmId("3745");
        dto.setPubDate("201318");
        dto.setPubTime("undefined");
        dto.setShowShare(1);
        dto.setTitle("四大");
        dto.setOrgCode("6100");
        infoService.createInfo(dto);
        		
    }
}
