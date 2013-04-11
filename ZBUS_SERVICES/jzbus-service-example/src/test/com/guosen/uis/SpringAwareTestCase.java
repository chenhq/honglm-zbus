package com.guosen.uis;

import org.springframework.test.AbstractDependencyInjectionSpringContextTests;


/**
 * <p>
 *   TODO	
 * </p> 
 * @author yuanxiaocheng
 * @create 2009-7-22
 */
public class SpringAwareTestCase extends AbstractDependencyInjectionSpringContextTests {

	@Override
	protected String[] getConfigLocations() {
		return new String[] {"classpath:applicationContext-test.xml"};
	}
	
	
}