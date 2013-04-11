package com.guosen.uis.service.impl;

import com.alibaba.fastjson.JSON;
import com.guosen.uis.SpringAwareTestCase;
import com.guosen.uis.dto.ClientDto;
import com.guosen.uis.dto.PageResult;
import com.guosen.uis.service.ClientService;

public class ClientServiceImplTest extends SpringAwareTestCase {

	private ClientService clientService ;

	public ClientService getClientService() {
		return clientService;
	}

	public void setClientService(ClientService clientService) {
		this.clientService = clientService;
	}
	
	public void testFindClient()
	{
		ClientDto dto = new ClientDto() ;
		dto.setPageNo(1);
		dto.setPageSize(20);
		dto.setOrgId("6100") ;
		dto.setCustId("610000000058");
		
		PageResult<ClientDto> rst= this.clientService.findClientList(dto);
		
		System.out.println(JSON.toJSON(rst));
	}
}
