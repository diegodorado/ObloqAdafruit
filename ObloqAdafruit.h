#ifndef _ObloqAdafruit_H_
#define _ObloqAdafruit_H_
#include "Arduino.h"


#define MAXTOPICNUMBER 5

#define SUCCESSED "1"
#define FAILED    "2"

#define SYSTEMTYPE "1"
#define WIFITYPE   "2"
#define HTTPTYPE   "3"

#define SYSTEMPING      "1"
#define SYSTEMVERSION   "2"
#define SYSTEMHEARTBEAT "3"

#define WIFIDISCONNECT    "1"
#define WIFICONNECTING    "2"
#define WIFICONNECTED     "3"
#define WIFICONNECTFAILED "4"


typedef struct  {
    String name;
    String lastMsg;
    long lastMillis;
    long updateInterval;
} Topic;

typedef struct  {
    String topic;
    String value;
    bool pending;
} PublishQueue;


enum State
{
    none,
    ping,
    wifiConnecting
};

class systemProtocol
{
public:
    static const uint8_t systemType    = 0;
    static const uint8_t systemCode    = 1;
    static const uint8_t systemMessage = 2;
};

class wifiProtocol
{
public:
    static const uint8_t wifiType    = 0;
    static const uint8_t wifiCode    = 1;
    static const uint8_t wifiMessage = 2;
};

class httpProtocol
{
public:
    static const uint8_t httpType    = 0;
    static const uint8_t httpCode    = 1;
    static const uint8_t httpMessage = 2;
};

class ObloqAdafruit
{
public:
    typedef void (*MsgHandle)(const String& topic, const String& message);

public:
    ObloqAdafruit(Stream *serial, const String& ssid, const String& pwd, const String& iotId, const String& iotPwd);
    ~ObloqAdafruit();

    void setMsgHandle(MsgHandle handle);
    bool isWifiConnected();
    void update();
    void subscribe(String topic);
    void subscribe(String topic, long updateInterval);
    void publish(const String& topic, int value);
    void publish(const String& topic, long value);
    void publish(const String& topic, double value);
    void publish(const String& topic, float value);
    void publish(const String& topic, const String& value);

private:

    Stream *_serial = NULL;
    String _receiveStringIndex[10] = {};
    String _separator = "|";
	String _ssid = "";
	String _pwd = "";
    String _iotId = "";
    String _iotPwd = "";
    String _ip = "";
    PublishQueue _publishQueue = {"","",false};

    bool _enable = false;
    bool _isSerialReady = false;
    bool _fetching = false;
    String _wifiState = "";

    MsgHandle _msgHandle = NULL;

    enum State _currentState = State::ping;
    Topic _topicArray[MAXTOPICNUMBER];
    int _topicCount = 0;
    int _currentTopic = 0;

    unsigned long _time = 0;
    unsigned long _pingInterval = 2000;
    unsigned long _wifiConnectInterval = 60000;
    unsigned long _pollInterval = 1000;

private:
    void httpHandle(const String& code, const String& message);
    void sendMsg(const String & msg);
    void ping();
    void connectWifi();
    void receiveData(const String& data);
    int  splitString(String data[],String str,const char* delimiters);
    void pollTopicArray();
    void flushPublishQueue();

};

#endif
