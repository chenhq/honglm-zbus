package com.guosen.uis.service.impl;

import com.guosen.uis.SpringAwareTestCase;
import com.guosen.uis.dto.SimpleResult;
import com.guosen.uis.service.ScreenWordService;

public class ScreenWordServiceImplTest extends SpringAwareTestCase {
	
	private ScreenWordService screenWordService ;

	public ScreenWordService getScreenWordService() {
		return screenWordService;
	}

	public void setScreenWordService(ScreenWordService screenWordService) {
		this.screenWordService = screenWordService;
	}
	
	public void testFindScreenWord()
	{
		String text = "张三在看《盗墓笔记》。";
		
		SimpleResult<Boolean> result = this.screenWordService.findScreenWord(text) ;
		
		System.out.println(result.getR()?"含有关键词":"没有关键词") ;
		
		SimpleResult<String> result2 = this.screenWordService.screenWord(text, true, true , false);
		
		System.out.println(result2.getR()) ;
		
		result2 = this.screenWordService.screenWord(text, true, false , false);
		
		System.out.println(result2.getR()) ;
		
	}
	
}
