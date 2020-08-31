# /bin/sh
rm mongoose.o httpmain.o http_server.o
g++ -std=c++11 -c mongoose.c
g++ -std=c++11 -c http_server.cpp
g++ -std=c++11 -c httpmain.cpp
g++ -std=c++11 -c WXBizJsonMsgCrypt.cpp
ar -crv libWXBizJsonMsgCrypt.a WXBizJsonMsgCrypt.o
g++ -std=c++11 mongoose.o http_server.o httpmain.o -L ./ -lssl -lWXBizJsonMsgCrypt -lcrypto