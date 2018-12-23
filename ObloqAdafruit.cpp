#include "ObloqAdafruit.h"

ObloqAdafruit::ObloqAdafruit(Stream *serial, const String& ssid, const String& pwd, const String& iotId, const String& iotPwd)
{
    this->_serial = serial;
    this->_ssid   = ssid;
    this->_pwd    = pwd;
    this->_iotId  = iotId;
    this->_iotPwd = iotPwd;
}

ObloqAdafruit::~ObloqAdafruit()
{}

void ObloqAdafruit::setMsgHandle(MsgHandle handle)
{
    this->_msgHandle = handle;
}


void ObloqAdafruit::update()
{
    // update state machine
    switch(this->_currentState)
    {
        case State::ping : ping(); break;
        case State::wifiConnecting: connectWifi(); break;
        default:break;
    }

    // listen to serial data
    if (this->_serial && this->_serial->available() > 0)
	{
		String data = this->_serial->readStringUntil('\r');
		while (true)
		{
			int length = data.length();
			int index = data.indexOf('\r');
			String subData = data.substring(0, index);
			this->receiveData(subData);

			if (index == -1 || length - index <= 1)
			{
				break;
			}
			else
			{
				subData = data.substring(index + 1, length);
				data = String(subData);
			}
		}
	}

    // poll subscribed topics
    this->pollTopicArray();
    this->flushPublishQueue();
}

void ObloqAdafruit::receiveData(const String& data)
{
	splitString(this->_receiveStringIndex,data,"|");

    if(this->_receiveStringIndex[systemProtocol::systemType] == SYSTEMTYPE)
    {
        if(this->_receiveStringIndex[systemProtocol::systemCode] == SYSTEMPING)
        {
            this->_currentState = State::wifiConnecting;
            this->_time = millis() - this->_wifiConnectInterval;
            this->_publishQueue.pending = false;
            this->_fetching = false;
        }
    }
	else if(this->_receiveStringIndex[wifiProtocol::wifiType] == WIFITYPE)
	{
        if(this->_receiveStringIndex[wifiProtocol::wifiCode] == WIFIDISCONNECT && this->_wifiState == WIFICONNECTED)
        {
            this->_wifiState = WIFIDISCONNECT;
            this->_currentState = State::wifiConnecting;
            this->_publishQueue.pending = false;
            this->_fetching = false;
        }
		else if(this->_receiveStringIndex[wifiProtocol::wifiCode] == WIFICONNECTED)
        {
			this->_ip = this->_receiveStringIndex[wifiProtocol::wifiMessage];
            this->_wifiState = WIFICONNECTED;
            this->_currentState = State::none;
            this->_publishQueue.pending = false;
            this->_fetching = false;
        }
		return;
	}
    else if(this->_receiveStringIndex[httpProtocol::httpType] == HTTPTYPE)
    {
        this->httpHandle(this->_receiveStringIndex[httpProtocol::httpCode],this->_receiveStringIndex[httpProtocol::httpMessage]);
    }


}

void ObloqAdafruit::subscribe(String topic)
{
    this->subscribe(topic, this->_pollInterval);
}

void ObloqAdafruit::subscribe(String topic, long updateInterval)
{
    // abort if too many topics are already subscribed
    if(this->_topicCount >= MAXTOPICNUMBER)
        return;

    // ignore if topic already subscribed
    for(int i = 0; i < this->_topicCount; i++)
    {
        if(this->_topicArray[i].name == topic)
        {
            return;
        }
    }

    // add topic to the topics array
    this->_topicArray[this->_topicCount++] = {topic, "", 0, updateInterval};

}


void ObloqAdafruit::pollTopicArray()
{
    // do not poll if a publish is pending, or if fetching already... or no topic subscribed
    if(this->_publishQueue.pending || this->_fetching || this->_topicCount == 0)
        return;

    this->_time = millis();

    if((this->_time - this->_topicArray[this->_currentTopic].lastMillis) > this->_topicArray[this->_currentTopic].updateInterval)
    {
        String url = "http://io.adafruit.com/api/v2/"+this->_iotId+"/feeds/"+this->_topicArray[this->_currentTopic].name+"/data/retain?X-AIO-Key="+this->_iotPwd;
        String getMsg = "|3|1|" + url + _separator;
        this->sendMsg(getMsg);
        this->_fetching = true;
        this->_topicArray[this->_currentTopic].lastMillis = this->_time;
    }

}




void ObloqAdafruit::flushPublishQueue()
{
    // nothing to do if nothing in the queue
    if(!this->_publishQueue.pending)
        return;

    String url = "http://io.adafruit.com/api/v2/"+this->_iotId+"/feeds/"+this->_publishQueue.topic+"/data?X-AIO-Key="+this->_iotPwd;
    //String url = "http://192.168.0.14:3000/posts";
    String content = "{\"value\":\""+this->_publishQueue.value+"\"}";
    String postMsg = "|3|2|" + url + "," + content + _separator;
    this->sendMsg(postMsg);
    this->_publishQueue.pending = false;

}


// convenience signatures
void ObloqAdafruit::publish(const String& topic, int value)    { this->publish(topic, String(value));}
void ObloqAdafruit::publish(const String& topic, long value)   { this->publish(topic, String(value));}
void ObloqAdafruit::publish(const String& topic, double value) { this->publish(topic, String(value));}
void ObloqAdafruit::publish(const String& topic, float value)  { this->publish(topic, String(value));}


void ObloqAdafruit::publish(const String& topic, const String& value)
{
    //add the message to the "one message queue"
    this->_publishQueue.topic = topic;
    this->_publishQueue.value = value;
    //this flag forces to send this before polling subscribed topics
    this->_publishQueue.pending = true;
}

void ObloqAdafruit::ping()
{
    if ((millis() - this->_time) >= this->_pingInterval)
    {
        this->_time = millis();
        this->_serial->print(F("|1|1|\r"));
    }
}

void ObloqAdafruit::connectWifi()
{
    if ((millis() - this->_time) >= this->_wifiConnectInterval)
    {
        this->_time = millis();
        String connectWifiMsg = "|2|1|" + _ssid + "," + _pwd + _separator;
        this->sendMsg(connectWifiMsg);
    }
}

bool ObloqAdafruit::isWifiConnected()
{
    if(this->_wifiState == WIFICONNECTED)
        return true;
    else
        return false;
}

void ObloqAdafruit::sendMsg(const String & msg)
{
	if (this->_serial)
	{
        this->_serial->print(msg + "\r");
	}
}

int ObloqAdafruit::splitString(String data[],String str,const char* delimiters)
{
    char *s = (char *)(str.c_str());
    int count = 0;
    data[count] = strtok(s, delimiters);
    while(data[count])
    {
        data[++count] = strtok(NULL, delimiters);
    }
    return count;
}


void ObloqAdafruit::httpHandle(const String& code, const String& message){

    // a succesful post message
    // anyway many post messages dont show up
    // or some appear as an empty message with code=200 ...bizarre...
    // here because they are too long....=(
    if(!message || code=="201"){
        // do nothing
    }
    else{
        if(this->_msgHandle)
        {
            //a succesful get message
            if(code=="200"){
                //discard bullshit
                message.replace(",,,", "");
                message.replace("\n", "");

                //trigger if message changed
                if(this->_topicArray[this->_currentTopic].lastMsg != message){
                    this->_msgHandle(this->_topicArray[this->_currentTopic].name,message);
                    this->_topicArray[this->_currentTopic].lastMsg = message;
                }
            }
            else{
                //todo: catch errors =)
            }

            // move to next topic
            // handle one topic at a time to know which topic is responding
            this->_currentTopic++;
            this->_currentTopic %= this->_topicCount;
        }

    }

    // enable fetch again
    this->_fetching = false;

}
