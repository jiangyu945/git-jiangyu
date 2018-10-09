
#include <ESP8266WiFi.h>

#define MAX_SRV_CLIENTS 3   //最大同时连接数，即你想要接入的设备数量，8266tcpserver只能接入五个，哎

 

const char *ssid = "TP_LINK_6960"; ////这里是我的wifi，你使用时修改为你要连接的wifi ssid

const char *password = "123456789"; //你要连接的wifi密码

 

WiFiServer server(8266);//你要的端口号，随意修改，范围0-65535

WiFiClient serverClients[MAX_SRV_CLIENTS];

 

void setup()

{

    Serial.begin(115200);

    delay(10);

    pinMode(16, OUTPUT);

    digitalWrite(16, 0);

    WiFi.begin(ssid, password);

 

    while (WiFi.status() != WL_CONNECTED)

    {

        delay(500);

    }

    server.begin();

    server.setNoDelay(true);  //加上后才正常些

}

 

void loop()

{

    blink();

 

    uint8_t i;

    if (server.hasClient())

    {

        for (i = 0; i < MAX_SRV_CLIENTS; i++)

        {

            if (!serverClients[i] || !serverClients[i].connected())

            {

                if (serverClients[i]) serverClients[i].stop();//未联接,就释放

                serverClients[i] = server.available();//分配新的

                continue;

            }

 

        }

        WiFiClient serverClient = server.available();

        serverClient.stop();

    }

    for (i = 0; i < MAX_SRV_CLIENTS; i++)

    {

        if (serverClients[i] && serverClients[i].connected())

        {

            digitalWrite(16, 0);//有链接存在,就一直长亮

 

            if (serverClients[i].available())

            {

                while (serverClients[i].available()) 

                    Serial.write(serverClients[i].read());

            }

        }

    }

    if (Serial.available())

    {

        size_t len = Serial.available();

        uint8_t sbuf[len];

        Serial.readBytes(sbuf, len);

        //push UART data to all connected telnet clients

        for (i = 0; i < MAX_SRV_CLIENTS; i++)

        {

            if (serverClients[i] && serverClients[i].connected())

            {

                serverClients[i].write(sbuf, len);  //向所有客户端发送数据

                delay(1);

            }

        }

    }

}

 

 

void blink()

{

    static long previousMillis = 0;

    static int currstate = 0;

 

    if (millis() - previousMillis > 200)  //200ms

    {

        previousMillis = millis();

        currstate = 1 - currstate;

        digitalWrite(16, currstate);

    }

}
