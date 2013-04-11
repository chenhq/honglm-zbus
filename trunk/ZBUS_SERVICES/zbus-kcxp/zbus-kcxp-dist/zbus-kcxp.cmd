REM -x	 kcxp标识 , 例如 -xzbus_kcxp
REM -h	 kcxp主机IP, 例如 -h172.24.180.134 
REM -p	 kcxp端口号, 例如 -p21000 
REM -r	 kcxp重连间隔（毫秒）, 例如 -r1000 (ms)
REM -qs	 kcxp发送队列名字（sendQ）, 例如 -qsreq_zb 
REM -qr	 kcxp接收队列名字（recvQ）, 例如 -qrans_zb 
REM -u	 kcxp登陆用户名, 例如-uKCXP00 
REM -P	 kcxp登陆密码, 例如 -P888888 

REM -b	 zbus服务总线地址（ip:port格式）, 例如 -b172.24.180.42:15555
REM -s	 本服务注册到zbus总线的标识名称, 例如 -sKCXP 
REM -c	 zbus-kcxp 代理服务线程数, 例如 -c2 
REM -kreg	 zbus注册认证码, 例如 -kregxyz 
REM -kacc	 设置本服访问认证码, 例如 -kaccxyz 
REM -log 滚动日志文件夹路径, 例如 -logC:\logs 
REM -v	 是否打开日志, 例如 -v1

zbus-kcxp -blocalhost:15555 -h172.24.181.216 -v1 -c2 -r5000 -sKCXP -qsreqacct_sql -qransacct_sql -xvm216-181 -loglogs