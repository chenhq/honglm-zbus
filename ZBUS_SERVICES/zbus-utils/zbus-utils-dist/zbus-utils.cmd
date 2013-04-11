REM -b	 zbus服务总线地址（ip:port格式）支持多总线，逗号分隔, 例如 -b172.24.180.42:15555
REM -s	 本服务注册到zbus总线的标识名称, 例如 -sTrade 
REM -c	 本服务注册到zbus总线的并发线程数, 例如 -c2 
REM -kreg	 zbus注册认证码, 例如 -kregxyz 
REM -kacc	 设置本服访问认证码, 例如 -kaccxyz 
REM -log 滚动日志文件夹路径, 例如 -logC:\logs 
REM -v	 是否打开日志, 例如 -v1  
zbus-utils -blocalhost:15555 -v1 -c2 -sUtils -loglogs