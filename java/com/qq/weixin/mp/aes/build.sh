# /bin/sh
javac -encoding UTF-8 -classpath ../../../../../lib/commons-codec-1.9.jar:../../../../../lib/json-20200518.jar -d . *.java
mv com/qq/weixin/mp/aes/* ./
rm -rf com