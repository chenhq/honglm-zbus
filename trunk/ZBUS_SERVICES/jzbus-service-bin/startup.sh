JAVA_HOME=/apps/dev/jdk
JAVA_OPTS="-Dfile.encoding=UTF-8 -server -Xms64m -Xmx512m -XX:PermSize=32m -XX:+UseParallelGC"
MAIN_CLASS=net.zbus.reqrep.DefaultWorker
nohup $JAVA_HOME/bin/java $JAVA_OPTS -cp "classes:lib/*" $MAIN_CLASS &