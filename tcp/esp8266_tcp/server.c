
#include <ESP8266WiFi.h>

#define MAX_SRV_CLIENTS 3   //���ͬʱ��������������Ҫ������豸������8266tcpserverֻ�ܽ����������

 

const char *ssid = "TP_LINK_6960"; ////�������ҵ�wifi����ʹ��ʱ�޸�Ϊ��Ҫ���ӵ�wifi ssid

const char *password = "123456789"; //��Ҫ���ӵ�wifi����

 

WiFiServer server(8266);//��Ҫ�Ķ˿ںţ������޸ģ���Χ0-65535

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

    server.setNoDelay(true);  //���Ϻ������Щ

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

                if (serverClients[i]) serverClients[i].stop();//δ����,���ͷ�

                serverClients[i] = server.available();//�����µ�

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

            digitalWrite(16, 0);//�����Ӵ���,��һֱ����

 

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

                serverClients[i].write(sbuf, len);  //�����пͻ��˷�������

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
