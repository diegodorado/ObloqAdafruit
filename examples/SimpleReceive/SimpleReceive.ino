#include "SoftwareSerial.h"
#include "ObloqAdafruit.h"

#define WIFI_SSID         "myWifiSSID"
#define WIFI_PASSWORD     "myWIFI_PWD"
#define ADAFRUIT_USERNAME "adafruit_user"
#define ADAFRUIT_KEY      "adafruit_key"


SoftwareSerial softSerial(10,11);
ObloqAdafruit olq(&softSerial,WIFI_SSID,WIFI_PASSWORD,ADAFRUIT_USERNAME,ADAFRUIT_KEY);

void msgHandle(const String& topic,const String& msg)
{
    if(topic=="lamp"){
      if(msg=="ON"){
        digitalWrite(13,1);
      }else{
        digitalWrite(13,0);
      }
    }
}

void setup()
{
    pinMode(13,OUTPUT);
    softSerial.begin(9600);
    olq.setMsgHandle(msgHandle);
    olq.subscribe("lamp");
}

void loop()
{
    olq.update();
}
