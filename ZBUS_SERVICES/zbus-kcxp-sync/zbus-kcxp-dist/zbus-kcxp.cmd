REM -x	 kcxp��ʶ , ���� -xzbus_kcxp
REM -h	 kcxp����IP, ���� -h172.24.180.134 
REM -p	 kcxp�˿ں�, ���� -p21000 
REM -r	 kcxp������������룩, ���� -r1000 (ms)
REM -qs	 kcxp���Ͷ������֣�sendQ��, ���� -qsreq_zb 
REM -qr	 kcxp���ն������֣�recvQ��, ���� -qrans_zb 
REM -u	 kcxp��½�û���, ����-uKCXP00 
REM -P	 kcxp��½����, ���� -P888888 

REM -b	 zbus�������ߵ�ַ��ip:port��ʽ��, ���� -b172.24.180.42:15555
REM -s	 ������ע�ᵽzbus���ߵı�ʶ����, ���� -sKCXP 
REM -c	 zbus-kcxp ��������߳���, ���� -c2 
REM -kreg	 zbusע����֤��, ���� -kregxyz 
REM -kacc	 ���ñ���������֤��, ���� -kaccxyz 
REM -log ������־�ļ���·��, ���� -logC:\logs 
REM -v	 �Ƿ����־, ���� -v1

zbus-kcxp -blocalhost:15555 -h172.24.181.216 -v1 -c2 -r5000 -sKCXP -qsreqacct_sql -qransacct_sql -xvm216-181 -loglogs