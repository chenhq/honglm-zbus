REM -a	 apex��ʶ , ���� -a127.0.0.1@9101/tcp  
REM -r	 apex������������룩, ���� -r1000 (ms) 
REM -u	 apex��½�û���, ����-uapex
REM -p	 apex��½����, ���� -p1111 

REM -b	 zbus�������ߵ�ַ��ip:port��ʽ��, ���� -b172.24.180.42:15555
REM -s	 ������ע�ᵽzbus���ߵı�ʶ����, ���� -sapex 
REM -c	 zbus-apex ��������߳���, ���� -c2 
REM -kreg	 zbusע����֤��, ���� -kregxyz 
REM -kacc	 ���ñ���������֤��, ���� -kaccxyz 
REM -log ������־�ļ���·��, ���� -logC:\logs 
REM -v	 �Ƿ����־, ���� -v1

zbus-apex -blocalhost:15555 -a10.50.71.12@9101/tcp  -v1 -c2 -r5000 -sAPEX -loglogs