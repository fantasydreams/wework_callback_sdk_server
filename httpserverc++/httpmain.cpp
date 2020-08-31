#include <iostream>
#include <memory>
#include "http_server.h"
#include "WXBizJsonMsgCrypt.h"
#include <string>
#include <ctime>


mg_serve_http_opts HttpServer::s_server_option;
std::string HttpServer::s_web_dir = "./";
std::unordered_map<std::string, ReqHandler> HttpServer::s_handler_map;
std::unordered_set<mg_connection *> HttpServer::s_websocket_session_set;


static std::string sToken = "gY1AGR3***hzy";
static std::string sEncodingAESKey = "g8VGfQEqluUhoKOlyjmmll8Q9C5***UTX5T2qkmI9Sv";
static std::string sCorpID = "wwabf***ec7171e769";
static std::string port = "8893";

unsigned char FromHex(unsigned char x)
{
	unsigned char y;
	if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
	else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
	else if (x >= '0' && x <= '9') y = x - '0';
	else assert(0);
	return y;
}

int TrimString ( std::string & str,const std::string drop )
{

	str  = str.erase( str.find_last_not_of(drop)+1 );

	str.erase( 0, str.find_first_not_of(drop) );

	return 0;
}

std::string UrlDecode(const std::string& str)
{
	std::string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; i++)
	{
		if (str[i] == '+') strTemp += ' ';
		else if (str[i] == '%')
		{
			assert(i + 2 < length);
			unsigned char high = FromHex((unsigned char)str[++i]);
			unsigned char low = FromHex((unsigned char)str[++i]);
			strTemp += high*16 + low;
		}
		else strTemp += str[i];
	}
	return strTemp;
}

size_t GetSubString(const std::string &msg, std::string & substr,const size_t start, const std::string & endSubstr)
{
	size_t end = msg.find(endSubstr,start);
	if(end == std::string::npos){
			printf("error end\n");
			return -1;
	}
	substr = msg.substr(start,end-start);
	return end + endSubstr.length();
}
bool URLverifyHandler(const std::string &msg, const std::string &body, mg_connection *c, const OnRspCallback rsp_callback);
bool JsonHandler(const std::string &_msg, const std::string &body, mg_connection *c, const OnRspCallback rsp_callback);
bool XmlHandler(const std::string &_msg, const std::string &body, mg_connection *c, const OnRspCallback rsp_callback);
bool CallbackHandler(const std::string &_msg, const std::string &body, mg_connection *c, const OnRspCallback rsp_callback)
{
	size_t start = _msg.find("Content-Type: ");
	if(start == std::string::npos)
	{
			printf("url not specify mes format(xml/json)\n");
			rsp_callback(c,"error format");
	}
	start += strlen("Content-Type: ");
	std::string contentType;
	size_t next = GetSubString(_msg,contentType,start,"\r\n");
	if(next == -1)
	{
			printf("error format\n");
			rsp_callback(c,"error format");
	}

	printf("msg format %s\n",contentType.c_str());

	if(contentType == "application/json")
	{
			return JsonHandler(_msg, body, c, rsp_callback);
	}
	else if(contentType == "text/xml")
	{
			return XmlHandler(_msg, body, c, rsp_callback);
	}
	else
	{
			rsp_callback(c, "unsport format");
			return false;
	}
}


bool DisPatcher(const std::string &msg, const std::string &body, mg_connection *c, const OnRspCallback rsp_callback)
{
	size_t echo = msg.find("echostr",0);
	if(echo != std::string::npos)
	{
			return URLverifyHandler(msg,body,c,rsp_callback);
	}
	else
	{
			return CallbackHandler(msg,body,c,rsp_callback);
	}

	return true;
}
bool JsonTextMsgHandler(Tencent::WXBizJsonMsgCrypt & wxcpt, const std::string &json_msg, mg_connection *c, const OnRspCallback rsp_callback);
bool JsonImageMsgHandler(Tencent::WXBizJsonMsgCrypt & wxcpt, const std::string &json_msg, mg_connection *c, const OnRspCallback rsp_callback);

bool JsonMsgHandler(Tencent::WXBizJsonMsgCrypt & wxcpt, const std::string& json_msg, mg_connection *c, const OnRspCallback rsp_callback)
{
	std::string msgType;
	wxcpt.GetJsonField(json_msg,"MsgType",msgType);
	if(msgType == "text"){
			return JsonTextMsgHandler(wxcpt, json_msg,c,rsp_callback);
	}
	else if(msgType == "image"){
			return JsonImageMsgHandler(wxcpt, json_msg,c,rsp_callback);
	}
	else{
			rsp_callback(c,"unspport format msg");
	}

}
bool JsonImageMsgHandler(Tencent::WXBizJsonMsgCrypt & wxcpt, const std::string &json_msg, mg_connection *c, const OnRspCallback rsp_callback)
{
	std::string toUser;
	wxcpt.GetJsonField(json_msg,"FromUserName",toUser);
	std::string FromUserName;
	wxcpt.GetJsonField(json_msg,"ToUserName",FromUserName);

	int CreateTime = time(NULL);
	std::string msg_type;
	wxcpt.GetJsonField(json_msg,"MsgType",msg_type);
	std::string content;
	wxcpt.GetJsonField(json_msg,"Content",content);

	rapidjson::Document  Doc;
	rapidjson::Value fieldValue(rapidjson::kStringType);
	Doc.SetObject();

	fieldValue.SetString(toUser.c_str(), strlen(toUser.c_str()));
	Doc.AddMember("ToUserName",fieldValue , Doc.GetAllocator());

	fieldValue.SetString(FromUserName.c_str(), strlen(FromUserName.c_str()));
	Doc.AddMember("FromUserName",fieldValue , Doc.GetAllocator());

	Doc.AddMember("CreateTime",CreateTime , Doc.GetAllocator());

	fieldValue.SetString(msg_type.c_str(), strlen(msg_type.c_str()));
	Doc.AddMember("MsgType",fieldValue , Doc.GetAllocator());

	rapidjson::Document  DocImage;
	DocImage.SetObject();
	std::string imageid;
	wxcpt.GetJsonField(json_msg,"MediaId",imageid);
	fieldValue.SetString(imageid.c_str(), strlen(imageid.c_str()));
	DocImage.AddMember("MediaId",fieldValue,DocImage.GetAllocator());

	Doc.AddMember("Image",DocImage,Doc.GetAllocator());

	rapidjson::StringBuffer strBuf;
	rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
	Doc.Accept(writer);

	std::string sResult = strBuf.GetString();
	printf("\n\nreplay jsom msg: %s\n\n",sResult.c_str());

	std::string rePlay;
	int ret = wxcpt.EncryptMsg(sResult,std::to_string(CreateTime),std::to_string(time(NULL)),rePlay);
	rsp_callback(c,rePlay);


}

bool JsonTextMsgHandler(Tencent::WXBizJsonMsgCrypt & wxcpt, const std::string &json_msg, mg_connection *c, const OnRspCallback rsp_callback)
{
	std::string toUser;
	wxcpt.GetJsonField(json_msg,"FromUserName",toUser);
	std::string FromUserName;
	wxcpt.GetJsonField(json_msg,"ToUserName",FromUserName);

	int CreateTime = time(NULL);
	/*
	std::string msg_type;
	wxcpt.GetJsonField(json_msg,"MsgType",msg_type);
	std::string content;
	wxcpt.GetJsonField(json_msg,"Content",content);
	
	rapidjson::Document  Doc;
	rapidjson::Value fieldValue(rapidjson::kStringType);
	Doc.SetObject();

	fieldValue.SetString(toUser.c_str(), strlen(toUser.c_str()));
	Doc.AddMember("ToUserName",fieldValue , Doc.GetAllocator());

	fieldValue.SetString(FromUserName.c_str(), strlen(FromUserName.c_str()));
	Doc.AddMember("FromUserName",fieldValue , Doc.GetAllocator());

	Doc.AddMember("CreateTime",CreateTime , Doc.GetAllocator());
	
	fieldValue.SetString(msg_type.c_str(), strlen(msg_type.c_str()));
	Doc.AddMember("MsgType",fieldValue , Doc.GetAllocator());

	fieldValue.SetString(content.c_str(), strlen(content.c_str()));
	Doc.AddMember("Content",fieldValue , Doc.GetAllocator());
	

	rapidjson::StringBuffer strBuf;
	rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
	Doc.Accept(writer);

	std::string sResult = strBuf.GetString();
	printf("\n\nreplay jsom msg: %s\n\n",sResult.c_str());
	*/
	std::string reMsg = json_msg;
	size_t start = reMsg.find(toUser,0);
	reMsg.replace(start,toUser.length(),FromUserName);
	start = reMsg.find(FromUserName,0);
	reMsg.replace(start,FromUserName.length(),toUser);

	std::string nonce = std::to_string(time(NULL));
	std::string encryptMsg;
	int ret = wxcpt.EncryptMsg(reMsg, std::to_string(CreateTime), nonce, encryptMsg);
	if(ret !=0 )
	{
			printf("Encrypt fail\n");
			rsp_callback(c,"Encrypt fail");
	}

	printf("replay msg: %s\n",reMsg.c_str());

	rsp_callback(c,encryptMsg);

}

bool JsonEventHandler(const std::string &_msg, const std::string &body, mg_connection *c, const OnRspCallback rsp_callback)
{
	rsp_callback(c,"success");
}

bool JsonHandler(const std::string &_msg, const std::string &body, mg_connection *c, const OnRspCallback rsp_callback)
{
	printf("json handler\n msg:%s \n body: %s\n",_msg.c_str(), body.c_str());

	std::string sReqMsgSig;
	size_t start = _msg.find("msg_signature=",0);
	if(start == std::string::npos){
			printf("error para \n");
			rsp_callback(c, "para error");
	}

	start = start + strlen("msg_signature=");

	size_t next_start = GetSubString(_msg,sReqMsgSig,start,"&timestamp=");

	if(next_start == -1){
			printf("error para \n");
			rsp_callback(c, "para error");
	}

	std::string sReqTimeStamp;
	next_start = GetSubString(_msg,sReqTimeStamp,next_start,"&nonce=");
	if(next_start == -1){
			printf("error para \n");
			rsp_callback(c, "para error");
	}

	std::string sReqNonce;
	next_start = GetSubString(_msg,sReqNonce,next_start," HTTP");
	if(next_start == -1){
			printf("error para \n");
			rsp_callback(c, "para error");
	}
	printf("para...\nsReqMsgSig: %s\nsReqTimeStamp:%s\nsReqNonce: %s\n",
			sReqMsgSig.c_str(),sReqTimeStamp.c_str(),
			sReqNonce.c_str());

	std::string sReqData = body;
	TrimString(sReqData," ");

	std::string sMsg;
	Tencent::WXBizJsonMsgCrypt wxcpt(sToken,sEncodingAESKey,sCorpID);
	int ret = wxcpt.DecryptMsg(sReqMsgSig, sReqTimeStamp, sReqNonce, sReqData, sMsg);
	if( ret!=0 )
	{
			printf("ERR: DecryptMsg ret: %d\n", ret);
			rsp_callback(c, "json data error");
			return ret;
	}else
	{
			printf("msg after decrypt: %s\n",sMsg.c_str());
	}

	std::string msg_type;
	ret = wxcpt.GetJsonField(sMsg,"MsgType",msg_type);
	if(ret != 0)
	{
			printf("Post data Format ERR\n");
			rsp_callback(c, "json data parse error");
			return Tencent::WXBizMsgCrypt_ParseJson_Error;

	}

	if(msg_type != "event")
	{
			return JsonMsgHandler(wxcpt,sMsg,c,rsp_callback);
	}
	else
	{
			return JsonEventHandler(sMsg,body,c,rsp_callback);
	}

	return true;
}

bool XmlHandler(const std::string &_msg, const std::string &body, mg_connection *c, const OnRspCallback rsp_callback)
{
	return true;
}



bool URLverifyHandler(const std::string &_msg, const std::string &body, mg_connection *c, const OnRspCallback rsp_callback)
{
	// do sth
	Tencent::WXBizJsonMsgCrypt wxcpt(sToken,sEncodingAESKey,sCorpID);
	std::string msg = UrlDecode(_msg);

	printf("\n------message--------: %s",msg.c_str());

	std::string sVerifyMsgSig;
	size_t start = msg.find("msg_signature=",0);
	if(start == std::string::npos){
			printf("error para \n");
			rsp_callback(c, "para error");
	}

	start = start + strlen("msg_signature=");

	size_t next_start = GetSubString(msg,sVerifyMsgSig,start,"&timestamp=");

	if(next_start == -1){
			printf("error para \n");
			rsp_callback(c, "para error");
	}

	std::string sVerifyTimeStamp;
	next_start = GetSubString(msg,sVerifyTimeStamp,next_start,"&nonce=");
	if(next_start == -1){
			printf("error para \n");
			rsp_callback(c, "para error");
	}

	std::string sVerifyNonce;
	next_start = GetSubString(msg,sVerifyNonce,next_start,"&echostr=");
	if(next_start == -1){
			printf("error para \n");
			rsp_callback(c, "para error");
	}

	std::string sVerifyEchoStr;
	next_start = GetSubString(msg,sVerifyEchoStr,next_start," HTTP");
	if(next_start == -1){
			printf("error para \n");
			rsp_callback(c, "para error");
	}

	printf("para...\nsVerifyMsgSig: %s\nsVerifyTimeStamp:%s\nsVerifyNonce: %s\nsVerifyEchoStr:%s\n",
			sVerifyMsgSig.c_str(),sVerifyTimeStamp.c_str(),
			sVerifyNonce.c_str(),sVerifyEchoStr.c_str());


	std::string sEchoStr;
	int ret = wxcpt.VerifyURL(sVerifyMsgSig, sVerifyTimeStamp, sVerifyNonce, sVerifyEchoStr, sEchoStr);
	if( ret!=0 )
	{
			printf("ERR: VerifyURL ret: %d",ret);
			rsp_callback(c, "verify error");
			return ret;
	}

	printf("echo str: %s\n",sEchoStr.c_str());
	rsp_callback(c, sEchoStr);
	return true;
}

int main(int argc, char *argv[])
{
	auto http_server = std::shared_ptr<HttpServer>(new HttpServer);
	http_server->Init(port);
	// add handler
	http_server->AddHandler("/", DisPatcher);
	http_server->Start();


	return 0;
}