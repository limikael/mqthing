#include "MqThing.h"
#include "MQTHING_TEMPLATES.h"
#include <EEPROM.h>

MqThing::MqThing() {
  _initialized=false;
	_buttonPin=-1;
	_ledPin=-1;
	_prevButtonState=HIGH;
  _name="My Thing";
  _apEnabled=false;
  _ssid="";
  _pass="";
  _lastStatus=-1;

  _pubSub.setClient(_wifiClient);
  _pubSub.setCallback([this](const char* topic, byte* payload, unsigned int length) {
    if (this->_messageFunc) {
      char pl[length+1];
      memcpy(pl,payload,length);
      pl[length]='\0';
      this->_messageFunc(String(topic),String(pl));
    }
  });
  _pubSubUser="";
  _pubSubPass="";
  _pubSubServer="";

  _connectFunc=NULL;
  _messageFunc=NULL;
}

bool MqThing::subscribe(String topic) {
  return _pubSub.subscribe(topic.c_str());
}

void MqThing::onConnect(MQTHING_CONNECT_FUNC *func) {
  _connectFunc=func;
}

void MqThing::onMessage(MQTHING_MESSAGE_FUNC *func) {
  _messageFunc=func;
}

void MqThing::setServer(String server, int port) {
  _pubSubServer=server;
  _pubSub.setServer(_pubSubServer.c_str(),port);
}

void MqThing::setServerAuth(String user, String pass) {
  _pubSubUser=user;
  _pubSubPass=pass;
}

void MqThing::setName(String name) {
  _name=name;
}

void MqThing::setButtonPin(int buttonPin) {
  _buttonPin=buttonPin;
  if (_buttonPin>=0) {
    pinMode(_buttonPin,INPUT_PULLUP);
  }
}

void MqThing::setLedPin(int ledPin) {
  _ledPin=ledPin;
  if (_ledPin>=0) {
    pinMode(_ledPin,OUTPUT);
  }
}

bool MqThing::publish(String topic, String message) {
  if (!_pubSub.connected())
    return false;

  return _pubSub.publish(topic.c_str(),message.c_str());
}

void MqThing::_configNetwork() {
  Serial.print("Configuring network, access point: ");
  Serial.println(_apEnabled);

  if (_apEnabled) {
    _pubSub.disconnect();

    WiFi.mode(WIFI_AP);
    while (!(WiFi.getMode()&WIFI_AP))
      delay(10);
    
    Serial.print(String("Starting AP: ")+_name+String(": "));
    Serial.println(WiFi.softAP(_name));

    Serial.print("Access point ip:");
    Serial.println(WiFi.softAPIP());

    _dnsServer.setErrorReplyCode(DNSReplyCode::NoError);

    Serial.print("Starting DNS Server:");
    Serial.println(_dnsServer.start(53, "*", WiFi.softAPIP()));

    Serial.println("Starting web server...");
    _webServer.begin();
  }

  else {
    _webServer.stop();
    _dnsServer.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);

    WiFi.begin(_ssid,_pass);
  }
}

void MqThing::setApEnabled(bool enabled) {
  if (_apEnabled==enabled)
    return;

  _apEnabled=enabled;

  if (!_initialized)
    return;

  _configNetwork();
}

bool MqThing::getApEnabled() {
  return _apEnabled;
}

// Check the current web server request. If it is not a request for the host
// where the web server is running, redirect. Return true if we did a redirect.
bool MqThing::_checkRedirect() {
  String host=_webServer.hostHeader();
  String serverIp=_webServer.client().localIP().toString();

  if (!host.equals(serverIp)) {
    Serial.println("Redirecting...");
    _webServer.sendHeader("Location",String("http://")+serverIp,true);
    _webServer.send(302, "text/plain", "");
    _webServer.client().stop();
    return true;
  }

  return false;
}

String MqThing::_htmlEntities(String s) {
  s.replace("\"","&quot;");
  s.replace("<","&lt;");
  s.replace(">","&gt;");
  s.replace("&","&amp;");

  return s;
}

void MqThing::_handlePage() {
  Serial.println("Serving config page...");

  String body=String(MQTHING_PAGE);
  bool doConnect=false;

  if (_webServer.hasArg("ssid")) {
    Serial.print("Updating ssid config: ");
    Serial.print(_webServer.arg("ssid"));
    Serial.print(":");
    Serial.println(_webServer.arg("pass"));

    _ssid=_webServer.arg("ssid");
    _pass=_webServer.arg("pass");

    _writeEepromString(0,_name,32);
    _writeEepromString(32,_ssid,32);
    _writeEepromString(64,_pass,32);
    EEPROM.commit();

    body.replace("{formdisplay}","none");
    body.replace("{messagedisplay}","block");

    doConnect=true;
  }

  else {
    body.replace("{formdisplay}","block");
    body.replace("{messagedisplay}","none");
  }

  body.replace("{title}",_htmlEntities(_name));
  body.replace("{ssid}",_htmlEntities(_ssid));
  body.replace("{pass}",_htmlEntities(_pass));

  _webServer.send(200,"text/html",body);

  if (doConnect) {
    delay(1000);
    _apEnabled=false;
    _configNetwork();
  }
}

String MqThing::_readEepromString(int addr, int maxlen) {
  char buf[maxlen];

  for (int i=0; i<maxlen; i++)
    buf[i]=EEPROM.read(addr+i);

  buf[maxlen-1]='\0';

  return String(buf);
}

void MqThing::_writeEepromString(int addr, String s, int maxlen) {
  char buf[maxlen];

  for (int i=0; i<maxlen; i++)
    buf[i]=s.charAt(i);

  buf[maxlen-1]='\0';
  for (int i=0; i<maxlen; i++)
    EEPROM.write(addr+i,buf[i]);
}

void MqThing::_init() {
  if (_initialized)
    return;

  _initialized=true;
  EEPROM.begin(1024);

  Serial.println("Initializing thing...");

  if (_readEepromString(0,32)==_name) {
    Serial.println("EEPROM head valid, loading config");
    _ssid=_readEepromString(32,32);
    _pass=_readEepromString(64,32);
  }

  else
    Serial.println("EEPROM head not valid, using default");

  _webServer.on("/",[this](){
    if (this->_checkRedirect())
      return;

    this->_handlePage();
  });

  _webServer.onNotFound([this](){
    if (this->_checkRedirect())
      return;

    this->_webServer.send(404, "text/plain", "Resource not found.");
  });

  _configNetwork();
}

void MqThing::_checkButton() {
  if (_buttonPin<0)
    return;

  int buttonState=digitalRead(_buttonPin);
  if (!buttonState && _prevButtonState) {
    setApEnabled(!getApEnabled());
    delay(50);
  }

  _prevButtonState=buttonState;
}

bool MqThing::isConnected() {
  if (getApEnabled())
    return false;

  if (WiFi.status()!=WL_CONNECTED)
    return false;

  if (!_pubSub.connected())
    return false;

  return true;
}

void MqThing::_updateLed() {
  static String blinkPatterns[5]={
    "xxxxxxxxxx..........",
    "xxxxxxxxxx",
    "x..........",
    "xxxxxxxxxx.",
    "x.x.........."
  };

  if (_ledPin<0)
    return;

  int ledMode=1;

  if (getApEnabled())
    ledMode=0;

  else {
    switch (WiFi.status()) {
      case WL_DISCONNECTED:
        ledMode=3;
        break;

      case WL_CONNECTED:
        if (_pubSub.connected())
          ledMode=1;

        else
          ledMode=4;
        break;

      default:
        ledMode=2;
        break;
    }
  }

  int l=blinkPatterns[ledMode].length();
  int index=(millis()/100)%l;

  if (blinkPatterns[ledMode][index]=='x')
    digitalWrite(_ledPin,HIGH);

  else
    digitalWrite(_ledPin,LOW);
}

void MqThing::_checkStatus() {
  int status=WiFi.status();

  if (status!=_lastStatus) {
    Serial.print("Status changed to: ");
    Serial.println(status);
    _lastStatus=status;

    if (status==WL_CONNECTED) {
      if (_ledPin>=0)
        digitalWrite(_ledPin,LOW);

      Serial.println("Connecting to MQTT...");
      bool connectStatus=_pubSub.connect(_name.c_str(),_pubSubUser.c_str(),_pubSubPass.c_str());
      Serial.print("MQTT connection: ");
      Serial.println(connectStatus);

      if (connectStatus && _connectFunc)
        _connectFunc();
    }

    else {
      if (_pubSub.connected())
        _pubSub.disconnect();
    }
  }
}

void MqThing::loop() {
  if (!_initialized)
    _init();

  _checkButton();
  _updateLed();

  if (_apEnabled) {
    _dnsServer.processNextRequest();
    _webServer.handleClient();
  }

  _checkStatus();
  _pubSub.loop();
}
