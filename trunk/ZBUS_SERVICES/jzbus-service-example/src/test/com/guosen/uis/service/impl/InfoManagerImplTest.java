package com.guosen.uis.service.impl;

import java.util.ArrayList;
import java.util.List;

import com.guosen.uis.SpringAwareTestCase;
import com.guosen.uis.dto.SecuMainDto;
import com.guosen.uis.manager.InfoManager;

public class InfoManagerImplTest extends SpringAwareTestCase {

    private InfoManager infoManager;

    public InfoManager getInfoManager() {
        return infoManager;
    }

    public void setInfoManager(InfoManager infoManager) {
        this.infoManager = infoManager;
    }
    
    public void testsaveInfoStockRel() {
        List<SecuMainDto> param = new ArrayList<SecuMainDto>();
        SecuMainDto dto = new SecuMainDto();
        dto.setMarketCodeEn("SZA");
        dto.setTradingCode("000008");
        param.add(dto);
        infoManager.saveInfoStockRel(4122065,param);
    }
    
}
