#include <iostream>
#include <string>
#include "WXBizJsonMsgCrypt.h"

using namespace Tencent;
using namespace std;

/*
*read me
*��ʾ������Ӧ���˿�Դ��openssl��rapidJson�⣬�������32λ��64λ�ľ�̬������,
*ʾ��Makefile������������64λ�汾�ģ������Ի���Ϊ32λ�����Makefile������

*openssl�İ汾����openssl-1.0.1h��http://www.openssl.org/
*rapidjson�İ汾����rapidjson v1.1.0,    https://github.com/Tencent/rapidjson/releases/tag/v1.1, pure headers
*/

int main()
{
	// ������ҵ����ҵ΢�ź�̨���õĲ�������
	string sToken = "QDG6eK";
	string sCorpID = "wx5823bf96d3bd56c7";
	string sEncodingAESKey = "jWmYm7qr5nMoAUwZRjGtBxmz3KA1tkAj3ykkR6q2B2C";	

	/*
	------------ʹ��ʾ��һ����֤�ص�URL---------------
	*��ҵ�����ص�ģʽʱ����ҵ΢�Ż�����֤url����һ��get���� 
	��������֤ʱ����ҵ�յ���������
	* GET /cgi-bin/wxpush?msg_signature=5c45ff5e21c57e6ad56bac8758b79b1d9ac89fd3&timestamp=1409659589&nonce=263014780&echostr=P9nAzCzyDtyTWESHep1vC5X9xho%2FqYX3Zpb4yKa9SKld1DsH3Iyt3tP3zNdtp%2B4RPcs8TgAE7OaBO%2BFZXvnaqQ%3D%3D 
	* HTTP/1.1 Host: qy.weixin.qq.com

	���յ�������ʱ����ҵӦ	1.������Get����Ĳ�����������Ϣ��ǩ��(msg_signature)��ʱ���(timestamp)��������ִ�(nonce)�Լ���ҵ΢�����͹�������������ַ���(echostr),
	��һ��ע����URL���롣
	2.��֤��Ϣ��ǩ������ȷ�� 
	3. ���ܳ�echostrԭ�ģ���ԭ�ĵ���Get�����response�����ظ���ҵ΢��
	��2��3����������ҵ΢���ṩ�Ŀ⺯��VerifyURL��ʵ�֡�
	*/
    WXBizJsonMsgCrypt wxcpt(sToken,sEncodingAESKey,sCorpID);
	// string sVerifyMsgSig = HttpUtils.ParseUrl("msg_signature");
	string sVerifyMsgSig = "5c45ff5e21c57e6ad56bac8758b79b1d9ac89fd3";
	// string sVerifyTimeStamp = HttpUtils.ParseUrl("timestamp");
	string sVerifyTimeStamp = "1409659589";
	// string sVerifyNonce = HttpUtils.ParseUrl("nonce");
	string sVerifyNonce = "263014780";
	// string sVerifyEchoStr = HttpUtils.ParseUrl("echostr");
	string sVerifyEchoStr = "P9nAzCzyDtyTWESHep1vC5X9xho/qYX3Zpb4yKa9SKld1DsH3Iyt3tP3zNdtp+4RPcs8TgAE7OaBO+FZXvnaqQ==";
	int ret = 0;
	// ��Ҫ���ص�����
	string sEchoStr;
	ret = wxcpt.VerifyURL(sVerifyMsgSig, sVerifyTimeStamp, sVerifyNonce, sVerifyEchoStr, sEchoStr);
	if( ret!=0 )
	{
		cout<<"ERR: VerifyURL ret: "<< ret << endl;
		return ret;
	}
	// ret==0 ��ʾ��֤URL�ɹ�����Ҫ��sEchoStr���ظ���ҵ΢��
	// HttpUtils.SetResponse( sEchoStr );
	

	/*
	------------ʹ��ʾ���������û��ظ�����Ϣ����---------------
	�û��ظ���Ϣ���ߵ���¼���Ӧʱ����ҵ���յ��ص���Ϣ������Ϣ�Ǿ�����ҵ΢�ż���֮���������post��ʽ���͸���ҵ�����ĸ�ʽ��ο��ٷ��ĵ�
	������ҵ�յ���ҵ΢�ŵĻص���Ϣ���£�
	POST /cgi-bin/wxpush? msg_signature=477715d11cdb4164915debcba66cb864d751f3e6&timestamp=1409659813&nonce=1372623149 HTTP/1.1
	Host: qy.weixin.qq.com
	Content-Length:
	Content-Type:text/json
	{
		"tousername":"wx5823bf96d3bd56c7",
		"encrypt":"No8isRLoXqFMhLlpe7R/DA7UbJ88DKJxDhJH/UVG3o1ib0Fhzdd3qWYHH/KL1mITv5qOCp2FbyILqfI7zazrp/ARgSHR177OCrv8O9UrMHWdnOaMXaz+mLd5X5VWm5r2J3Qpm+NdTQRPhHbce88frKF3wqTaZunKW7ae87bRZUfaq5tLFnyTsf6aiy0su3SsQ06dQGKPcHfYHY3upB881008Q9t9xeAZ/uqfXpYQgSLQfaX+fk/K/FQEl4QpLk94eD1YjluFY3uLnKp40zDyxgeWRAmgTtmx1eLwediVqZ8=",
		"agentid":218
	}

	��ҵ�յ�post����֮��Ӧ��	1.������url�ϵĲ�����������Ϣ��ǩ��(msg_signature)��ʱ���(timestamp)�Լ�������ִ�(nonce)
	2.��֤��Ϣ��ǩ������ȷ�ԡ�	3.��post��������ݽ���json����������"encrypt"��ǩ�����ݽ��н��ܣ����ܳ��������ļ����û��ظ���Ϣ�����ģ����ĸ�ʽ��ο��ٷ��ĵ�
	��2��3����������ҵ΢���ṩ�Ŀ⺯��DecryptMsg��ʵ�֡�
	*/
	
	// string sReqMsgSig = HttpUtils.ParseUrl("msg_signature");
	string sReqMsgSig = "0a33ac8596e2271d03eded037d04383a28e6f133";
	// string sReqTimeStamp = HttpUtils.ParseUrl("timestamp");
	string sReqTimeStamp = "1409659813";
	// string sReqNonce = HttpUtils.ParseUrl("nonce");
	string sReqNonce = "1372623149";
	// post�������������
	string sReqData = "{\"tousername\":\"wx5823bf96d3bd56c7\",\"encrypt\":\"No8isRLoXqFMhLlpe7R/DA7UbJ88DKJxDhJH/UVG3o1ib0Fhzdd3qWYHH/KL1mITv5qOCp2FbyILqfI7zazrp/ARgSHR177OCrv8O9UrMHWdnOaMXaz+mLd5X5VWm5r2J3Qpm+NdTQRPhHbce88frKF3wqTaZunKW7ae87bRZUfaq5tLFnyTsf6aiy0su3SsQ06dQGKPcHfYHY3upB881008Q9t9xeAZ/uqfXpYQgSLQfaX+fk/K/FQEl4QpLk94eD1YjluFY3uLnKp40zDyxgeWRAmgTtmx1eLwediVqZ8=\",\"agentid\":\"218\"}";
	string sMsg;  // ����֮�������
	ret = wxcpt.DecryptMsg(sReqMsgSig, sReqTimeStamp, sReqNonce, sReqData, sMsg);
	if( ret!=0 )
	{
		cout << "ERR: DecryptMsg ret: " << ret << endl;
		return ret;
	}

	// ���ܳɹ���sMsg��Ϊjson��ʽ������
	// TODO: �����ĵĴ���
	// For example:
	string content;

	//�����ṩ��json�н����õ�string���͵�value�⣬��sdk���ṩ��uint_32��uint64_t���͵Ľӿڣ�
	//�û�Ҳ�ɽ����ľ���DecryptMsg����õ���Ϣ���ĺ�ֱ������rapaidjson�ṩ�Ľӿ���ȡ��ֵ�������ͬһjson������ȡֵ������ֱ��ʹ��rapaidjson�ӿ�
	if( 0!= wxcpt.GetJsonField(sMsg, "Content", content) )
    {
		cout<<"Post data Format ERR"<<endl;
        return WXBizMsgCrypt_ParseJson_Error;
    }
	cout<<"content: "<<content<<endl;

	uint32_t timeStamp = 0;
	if( 0!= wxcpt.GetJsonField(sMsg, "CreateTime", timeStamp) )
    {
		cout<<"Post data Format ERR"<<endl;
        return WXBizMsgCrypt_ParseJson_Error;
    }
	cout<<"timestamp: "<<timeStamp<<endl;

	uint64_t msgId = 0;
	if( 0!= wxcpt.GetJsonField(sMsg, "MsgId", msgId) )
    {
		cout<<"Post data Format ERR"<<endl;
        return WXBizMsgCrypt_ParseJson_Error;
    }
	cout<<"MsgId: "<<msgId<<endl;
	// ...
	// ...
	// ...
	
	
	/*
	------------ʹ��ʾ��������ҵ�ظ��û���Ϣ�ļ���---------------
	��ҵ�����ظ��û�����ϢҲ��Ҫ���м��ܣ�����ƴ�ӳ����ĸ�ʽ��json����
	������ҵ��Ҫ�ظ��û����������£�

	{ 
		"ToUserName": "mycreate",
		"FromUserName":"wx5823bf96d3bd56c7",
		"CreateTime": 1348831860,
		"MsgType": "text",
		"Content": "this is a test",
		"MsgId": 1234567890123456,
		"AgentID": 128
    }

	Ϊ�˽��˶����Ļظ����û�����ҵӦ��	1.�Լ�����ʱ��ʱ���(timestamp),������ִ�(nonce)�Ա�������Ϣ��ǩ����Ҳ����ֱ���ô���ҵ΢�ŵ�post url�Ͻ������Ķ�Ӧֵ��
	2.�����ļ��ܵõ����ġ�
	3.�����ģ�����1���ɵ�timestamp,nonce����ҵ����ҵ΢���趨��token������Ϣ��ǩ����
	4.�����ģ���Ϣ��ǩ����ʱ�����������ִ�ƴ�ӳ�json��ʽ���ַ��������͸���ҵ�š�
	����2��3��4����������ҵ΢���ṩ�Ŀ⺯��EncryptMsg��ʵ�֡�
	*/
	//��Ҫ���͵�����ΪsRespData
	string sRespData = "{\"ToUserName\":\"wx5823bf96d3bd56c7\",\"FromUserName\":\"mycreate\",\"CreateTime\": 1409659813,\"MsgType\":\"text\",\"Content\":\"hello\",\"MsgId\":4561255354251345929,\"AgentID\": 218}";
	string sEncryptMsg; 
	ret = wxcpt.EncryptMsg(sRespData, sReqTimeStamp, sReqNonce, sEncryptMsg);
	if( ret!=0 )
	{
		cout<<"ERR: EncryptMsg ret: "<<ret<<endl;
		return ret;
	}
	//TODO:
	// ret==0��ʾ���ܳɹ�����ҵ��Ҫ��sEncryptMsg����
	// HttpUtils.SetResponse(sEncryptMsg);
	cout<<sEncryptMsg<<endl;

    return 0;
}
