#include "SoftwareSerial.h"
#include "ObloqAdafruit.h"

#define WIFI_SSID         "Fibertel WiFi425"
#define WIFI_PASSWORD     "0143155948"
#define ADAFRUIT_USERNAME "diegodorado"
#define ADAFRUIT_KEY      "27edcfdd29a244b6b049abb07727c117"

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
