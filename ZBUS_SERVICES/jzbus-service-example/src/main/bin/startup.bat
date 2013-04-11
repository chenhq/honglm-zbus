@echo off

if '%1=='## goto ENVSET

rem 库文件所在的目录，相对于当前路径
SET LIBDIR=lib

rem 设定CLSPATH
SET CLSPATH=classes
FOR %%c IN (%LIBDIR%\*.jar) DO CALL %0 ## %%c

rem 运行
GOTO RUN

:RUN
rem JVM参数
SET JAVA_OPTS="-server"

rem 要启动的类名
SET CLSNAME=com.guosen.uis.proxy.ZBusUISProxy

java -Dfile.encoding=utf-8 -cp %CLSPATH% %CLSNAME% 
goto END

:ENVSET
@echo off
set CLSPATH=%CLSPATH%;%2
goto END
:END