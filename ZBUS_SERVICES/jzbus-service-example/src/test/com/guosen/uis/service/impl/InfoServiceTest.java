package com.guosen.uis.service.impl;

import java.util.ArrayList;
import java.util.List;

import sun.misc.BASE64Encoder;

import com.guosen.uis.SpringAwareTestCase;
import com.guosen.uis.dto.InfoAttachDto;
import com.guosen.uis.dto.InfoDto;
import com.guosen.uis.dto.LatestInfoCondition;
import com.guosen.uis.dto.PageResult;
import com.guosen.uis.dto.SecuMainDto;
import com.guosen.uis.dto.SimpleResult;
import com.guosen.uis.service.InfoService;
import com.guosen.uis.util.CommonFunc;
import com.guosen.uis.util.ConstValues;

public class InfoServiceTest extends SpringAwareTestCase {

	private InfoService infoService ;

	public InfoService getInfoService() {
		return infoService;
	}

	public void setInfoService(InfoService infoService) {
		this.infoService = infoService;
	}
	public void passCreateInfo2()
	{
		//"clientMsgSend":false,"contentType":"2","draft":false,"infoTypeCode":"1",
		//"opOwner":"admin_beijing","prdLmId":"2462","relImagePath":"e:/uis/uploadimage/12332434","showShare":0,
		//"title":"文章标题啊","totalCount":0
		InfoDto dto = new InfoDto() ;
		dto.setClientMsgSend(false) ;
		dto.setContentType("2") ;
		dto.setDraftFlag(false);
		dto.setInfoTypeCode("1") ;
		dto.setOpOwner("admin_beijing") ;
		dto.setPrdLmId("2462") ;
		dto.setRelImagePath("e:/uis/uploadimage/12332434");
		dto.setShowShare(0) ;
		dto.setTitle("文章标题啊") ;
		dto.setTotalCount(0);
		
		SimpleResult<Integer> res = this.infoService.createInfo(dto) ;
		
		System.out.println(res.getR());
		System.out.println(res.getRtnMsg()) ;
	}

	public void passCreateInfo()
	{
		InfoDto dto = new InfoDto() ;
		dto.setTitle("这个是一条测试用的资讯.") ;
		dto.setContent("测试资讯内容为空。") ;
		dto.setPubDate("20130108") ;
		dto.setPubTime("15:47:03") ;
		dto.setDesc("测试摘要") ;
		List<InfoAttachDto> att = new ArrayList<InfoAttachDto>() ;
		
		dto.setInfoAttaches(att) ;
		dto.setInfoPrioCode("2") ;
		dto.setInfoTypeCode("4") ;
		dto.setOpOwner("zhangsan") ;
		dto.setOrigSource("网上摘录过来的.");
		dto.setPrdLmId("19") ;
		dto.setRelImagePath("/apps/dev/filedata/download/avatar/026b01b5-d83d-41ca-846f-6f425ea970ad.gif") ;
		dto.setShowShare(1) ;
		dto.setTitleStyle("1") ;
		dto.setClientMsgSend(true) ;
		
		List<SecuMainDto> secuIds = new ArrayList<SecuMainDto>() ;

		SecuMainDto secuDto1 = new SecuMainDto() ;
		secuDto1.setSid(19) ;
		
		SecuMainDto secuDto2 = new SecuMainDto() ;
		secuDto2.setSid(20) ;
		
		secuIds.add(secuDto1) ;
		secuIds.add(secuDto2) ;
		
		List<String> keyWords = new ArrayList<String>() ;
		keyWords.add("国家") ;
		keyWords.add("大学");
		
		List<InfoAttachDto> infoAttaches = new ArrayList<InfoAttachDto>() ;
		
		InfoAttachDto attaDto = new InfoAttachDto() ;
		attaDto.setDesc("测试TXT") ;
		attaDto.setExt(".txt") ;
		attaDto.setOrigFileName("fe_user.txt") ;
		byte[] txtContent = CommonFunc.stream2Byte("D:\\fe_user.txt") ;
		BASE64Encoder encoder = new BASE64Encoder() ;
		String fr = encoder.encode(txtContent) ;
		
		attaDto.setBase64Content(fr) ;
		
		infoAttaches.add(attaDto) ;
		dto.setInfoAttaches(infoAttaches) ;
		
		List<InfoDto> relInfoIds = new ArrayList<InfoDto>() ;

		InfoDto dto1 = new InfoDto() ;
		dto1.setInfoID("12") ;
		
		InfoDto dto2 = new InfoDto() ;
		dto2.setInfoID("13") ;
		
		relInfoIds.add(dto1) ;
		relInfoIds.add(dto2) ;

		dto.setKeyWords(keyWords) ;
		dto.setSecuIds(secuIds) ;
		dto.setRelInfoIds(relInfoIds) ;
		
		SimpleResult<Integer> res = this.infoService.createInfo(dto) ;
		
		System.out.println(res.getR());
		System.out.println(res.getRtnMsg()) ;
	}
	
	public void passGetInfo()
	{
		String sid = "4121904" ;
		SimpleResult<InfoDto> r = this.infoService.getInfo(sid) ;
		
		System.out.println(r.getR());
		System.out.println(r.getRtnMsg()) ;
	}
	
	public void testFindLatestInfo()
	{
		LatestInfoCondition cond = new LatestInfoCondition() ;
		
		cond.setPrdLmId("1484") ;
		
		PageResult<InfoDto> page = this.infoService.findLatestInfo(cond) ;
		
		System.out.println(page.getRtnMsg()) ;
		System.out.println(page.getTotalCount()) ;
		
	}
	@SuppressWarnings("rawtypes")
	public void passDelInfo()
	{
		List<String> deleteIds = new ArrayList<String>() ;
		deleteIds.add("4121911") ;
		deleteIds.add("4121910") ;
		deleteIds.add("4121909") ;
		
		SimpleResult rst = this.infoService.removeInfo(deleteIds , "zhangjie") ;
				
		System.out.println(rst.getRtnMsg()) ;
		
	}
	
	public void passUpdateInfo()
	{
		InfoDto dto = new InfoDto() ;
		dto.setInfoID("4121911") ;
		
		dto.setTitle("更新_这个是一条测试用的资讯.") ;
		dto.setContent("测试资讯内容为空。") ;
		dto.setPubDate("20121210") ;
		dto.setPubTime("15:47:03") ;
		dto.setDesc("更新_测试摘要") ;
		List<InfoAttachDto> att = new ArrayList<InfoAttachDto>() ;
		
		dto.setInfoAttaches(att) ;
		dto.setInfoPrioCode("2") ;
		dto.setInfoTypeCode("4") ;
		dto.setOpOwner("zhangsan") ;
		dto.setOrigSource("更新_网上摘录过来的.");
		dto.setPrdLmId("98") ;
		dto.setRelImagePath("/apps/dev/filedata/download/avatar/026b01b5-d83d-41ca-846f-6f425ea970ad.gif") ;
		dto.setShowShare(1) ;
		dto.setTitleStyle("1") ;
		
		List<SecuMainDto> secuIds = new ArrayList<SecuMainDto>() ;

		SecuMainDto secuDto1 = new SecuMainDto() ;
		secuDto1.setSid(19) ;
		
		SecuMainDto secuDto2 = new SecuMainDto() ;
		secuDto2.setSid(20) ;
		
		secuIds.add(secuDto1) ;
		secuIds.add(secuDto2) ;
		
		List<String> keyWords = new ArrayList<String>() ;
		keyWords.add("更新_国家") ;
		keyWords.add("更新_大学");
		
		List<InfoAttachDto> infoAttaches = new ArrayList<InfoAttachDto>() ;
		
		InfoAttachDto attaDto = new InfoAttachDto() ;
		attaDto.setDesc("测试TXT") ;
		attaDto.setExt(".txt") ;
		attaDto.setOrigFileName("fe_user_update.txt") ;

		byte[] txtContent = CommonFunc.stream2Byte("D:\\fe_user.txt") ;
		BASE64Encoder encoder = new BASE64Encoder() ;
		String fr = encoder.encode(txtContent) ;
		
		attaDto.setBase64Content(fr) ;
		
		infoAttaches.add(attaDto) ;
		dto.setInfoAttaches(infoAttaches) ;
		
		List<InfoDto> relInfoIds = new ArrayList<InfoDto>() ;

		InfoDto dto1 = new InfoDto() ;
		dto1.setInfoID("12") ;
		
		InfoDto dto2 = new InfoDto() ;
		dto2.setInfoID("13") ;
		
		relInfoIds.add(dto1) ;
		relInfoIds.add(dto2) ;

		dto.setKeyWords(keyWords) ;
		dto.setSecuIds(secuIds) ;
		dto.setRelInfoIds(relInfoIds) ;
		dto.setAttachIdsToDelete("1210874") ;
		
		SimpleResult<Integer> res = this.infoService.updateInfo(dto) ;
		
		System.out.println(res.getR());
		System.out.println(res.getRtnMsg()) ;
	}
	
	@SuppressWarnings("rawtypes")
	public void passUpdatestatus()
	{
		List<String> infoIds = new ArrayList<String>() ;
		infoIds.add("4121911") ;
		
		String infoStatusCode = ConstValues.DICT_CODE_INFO_STATUS_TH ;
		
		/*SimpleResult r = this.infoService.updateStatus(infoIds, infoStatusCode) ;
		
		System.out.println(r.getRtnMsg()) ;*/
	}
}
