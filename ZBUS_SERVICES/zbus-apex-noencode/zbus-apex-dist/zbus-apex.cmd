REM -a	 apex标识 , 例如 -a127.0.0.1@9101/tcp  
REM -r	 apex重连间隔（毫秒）, 例如 -r1000 (ms) 
REM -u	 apex登陆用户名, 例如-uapex
REM -p	 apex登陆密码, 例如 -p1111 

REM -b	 zbus服务总线地址（ip:port格式）, 例如 -b172.24.180.42:15555
REM -s	 本服务注册到zbus总线的标识名称, 例如 -sapex 
REM -c	 zbus-apex 代理服务线程数, 例如 -c2 
REM -kreg	 zbus注册认证码, 例如 -kregxyz 
REM -kacc	 设置本服访问认证码, 例如 -kaccxyz 
REM -log 滚动日志文件夹路径, 例如 -logC:\logs 
REM -v	 是否打开日志, 例如 -v1

zbus-apex -blocalhost:15555 -a10.50.71.12@9101/tcp  -v1 -c2 -r5000 -sAPEX -loglogs