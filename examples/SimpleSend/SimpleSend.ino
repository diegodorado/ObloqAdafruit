#include "SoftwareSerial.h"
#include "ObloqAdafruit.h"

#define WIFI_SSID         "myWifiSSID"
#define WIFI_PASSWORD     "myWIFI_PWD"
#define ADAFRUIT_USERNAME "adafruit_user"
#define ADAFRUIT_KEY      "adafruit_key"


SoftwareSerial softSerial(10,11);
ObloqAdafruit olq(&softSerial,WIFI_SSID,WIFI_PASSWORD,ADAFRUIT_USERNAME,ADAFRUIT_KEY);

static unsigned long currentTime = 0;
bool valueOn = true;

void setup()
{
    softSerial.begin(9600);
}

void loop()
{
    olq.update();

    // do not use delay!, check time with millis() instead
    if(millis() - currentTime > 5000)
    {
        valueOn = !valueOn;
        currentTime =  millis();
        olq.publish("lamp", valueOn ? "1" : "0");
    }

}
