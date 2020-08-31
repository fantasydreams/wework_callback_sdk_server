import org.json.JSONObject;
import com.qq.weixin.mp.aes.WXBizJsonMsgCrypt;
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.*;
import java.util.*;
import java.net.URLDecoder;

public class javaHttpServer {
    private static String sToken = "gY1AGR3mjBhz***";
    private static String sEncodingAESKey = "g8VGfQEqluU*****Olyjmmll8Q9C5tVFUTX5T2qkmI9Sv";
    private static String sCorpID = "wwabfd0cec7171****69";
    private static int port = 8893;

    public static Map<String, String> queryToMap(String query) {
        Map<String, String> result = new HashMap<>();
        for (String param : query.split("&")) {
            String[] entry = param.split("=");
            if (entry.length > 1) {
                result.put(entry[0], entry[1]);
            } else {
                result.put(entry[0], "");
            }
        }
        return result;
    }

    public static void verifyURL(Map<String, String> paraMap, HttpExchange httpExchange) throws Exception {
        String msg_signature = paraMap.get("msg_signature");
        String timestamp = paraMap.get("timestamp");
        String nonce = paraMap.get("nonce");
        String echostr = URLDecoder.decode(paraMap.get("echostr"), "UTF-8");
        System.out.println(echostr);
        WXBizJsonMsgCrypt wxcpt = new WXBizJsonMsgCrypt(sToken, sEncodingAESKey, sCorpID);
        String sEchoStr = "";
        try {
            sEchoStr = wxcpt.VerifyURL(msg_signature, timestamp, nonce, echostr);
            System.out.println("verifyurl echostr: " + sEchoStr);

        } catch (Exception e) {
            e.printStackTrace();
        }
        byte[] respContents = sEchoStr.getBytes("UTF-8");

        httpExchange.sendResponseHeaders(200, respContents.length);

        httpExchange.getResponseBody().write(respContents);

    }

    public static void msgHandler(Map<String, String> paraMap, String body, HttpExchange httpExchange)
            throws Exception {
        String msg_signature = paraMap.get("msg_signature");
        String timestamp = paraMap.get("timestamp");
        String nonce = paraMap.get("nonce");
        WXBizJsonMsgCrypt wxcpt = new WXBizJsonMsgCrypt(sToken, sEncodingAESKey, sCorpID);
        try {
            String sMsg = wxcpt.DecryptMsg(msg_signature, timestamp, nonce, body);
            System.out.println("after decrypt msg: " + sMsg);
            JSONObject json = new JSONObject(sMsg);
            String ToUserName = json.getString("ToUserName");
            String FromUserName = json.getString("FromUserName");
            sMsg = sMsg.replaceFirst(FromUserName, ToUserName);
            sMsg = sMsg.replaceFirst(ToUserName, FromUserName);
            System.out.println("replay msg" + sMsg);
            String sEncryptMsg = wxcpt.EncryptMsg(sMsg, timestamp, nonce);

            System.out.println("after encrypt msg: " + sEncryptMsg);

            byte[] respContents = sEncryptMsg.getBytes("UTF-8");

            httpExchange.sendResponseHeaders(200, respContents.length);

            httpExchange.getResponseBody().write(respContents);

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void main(String[] args) throws Exception {
        HttpServer httpServer = HttpServer.create(new InetSocketAddress(port), 0);
        httpServer.createContext("/", new HttpHandler() {
            @Override
            public void handle(HttpExchange httpExchange) throws IOException {

                System.out.println(httpExchange.getRequestURI().getQuery());

                Map<String, String> params = queryToMap(httpExchange.getRequestURI().getQuery());
                System.out.println("params:" + params);
                StringBuilder body = new StringBuilder();
                try (InputStreamReader reader = new InputStreamReader(httpExchange.getRequestBody(), "utf-8")) {
                    int read;
                    char[] buffer = new char[256];
                    while ((read = reader.read(buffer)) != -1) {
                        body.append(buffer, 0, read);
                    }
                }
                String sBody = body.toString();
                System.out.println("body:" + sBody);

                boolean flag = params.containsKey("echostr");
                if (flag) {
                    System.out.println("verify");
                    try {
                        verifyURL(params, httpExchange);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                } else {
                    System.out.println("msg");
                    try {
                        msgHandler(params, sBody, httpExchange);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }

                httpExchange.getRequestBody().close();

                /*
                 * 
                 * httpExchange.close();
                 */
            }
        });

        httpServer.start();
    }
}