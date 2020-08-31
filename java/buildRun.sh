# /bin/sh
javac -encoding UTF-8 -classpath .:./lib/commons-codec-1.9.jar:./lib/json-20200518.jar javaHttpServer.java
java -Dfile.encoding=utf-8 -cp .:./lib/commons-codec-1.9.jar:./lib/json-20200518.jar javaHttpServer