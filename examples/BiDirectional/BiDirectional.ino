#include "SoftwareSerial.h"
#include "ObloqAdafruit.h"

#define WIFI_SSID      "myWifiSSID"
#define WIFI_PASSWORD  "myWIFI_PWD"
#define IO_USERNAME    "adafruit_user"
#define IO_KEY         "adafruit_key"


SoftwareSerial softSerial(10,11);
ObloqAdafruit olq(&softSerial,WIFI_SSID,WIFI_PASSWORD,IO_USERNAME,IO_KEY);
/*
This example sketch will subscribe to a "please-log" topic
And upon a message is received, will publish "40" to "ana-log" topic
Just to demonstrate subscribing and publishing on the same sketch.
*/

void msgHandle(const String& topic,const String& msg)
{
    if(topic=="please-log"){
        olq.publish("ana-log","40");
    }
}

void setup()
{
    softSerial.begin(9600);
    olq.setMsgHandle(msgHandle);
    olq.subscribe("please-log");
}

void loop()
{
    olq.update();
}
