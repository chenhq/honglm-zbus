REM SET JAVA_HOME=%JAVA_HOME%\bin\
SET JAVA_OPTS=-Dfile.encoding=UTF-8 -server -Xms64m -Xmx512m -XX:PermSize=32m -XX:+UseParallelGC
SET MAIN_CLASS=net.zbus.ws.Publisher

java %JAVA_OPTS% -cp "classes;lib/*" %MAIN_CLASS%  
 