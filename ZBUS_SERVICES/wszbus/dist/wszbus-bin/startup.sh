JAVA_HOME=/apps/dev/jdk
JAVA_OPTS="-server -Xms64m -Xmx512m -XX:PermSize=32m -XX:+UseParallelGC"
MAIN_CLASS=net.zbus.ws.Publisher
$JAVA_HOME/bin/java $JAVA_OPTS -cp "classes:lib/*" $MAIN_CLASS