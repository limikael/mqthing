#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>

typedef void MQTHING_MESSAGE_FUNC(String topic, String message);
typedef void MQTHING_CONNECT_FUNC();

class MqThing {
private:
	int _buttonPin;
	int _ledPin;
	int _prevButtonState;
  bool _initialized;
  DNSServer _dnsServer;
  ESP8266WebServer _webServer;
  String _name;
  bool _apEnabled;
  String _ssid;
  String _pass;
  int _lastStatus;
  WiFiClient _wifiClient;
  PubSubClient _pubSub;
  String _pubSubUser;
  String _pubSubPass;
  String _pubSubServer;

	void _handleButtonPress();
  void _handleNotFound();
  void _init();
  bool _checkRedirect();
  void _handlePage();
  void _checkButton();
  void _updateLed();
  void _configNetwork();
  String _htmlEntities(String s);
  String _readEepromString(int addr, int maxlen=32);
  void _writeEepromString(int addr, String s, int maxlen=32);
  int _state;
  void _checkStatus();

  MQTHING_MESSAGE_FUNC *_messageFunc;
  MQTHING_CONNECT_FUNC *_connectFunc;

public:
	MqThing();

  void setButtonPin(int buttonPin);
  void setLedPin(int ledPin);
  void setName(String name);

  void setApEnabled(bool enabled);
  bool getApEnabled();

  boolean isConnected();
  void setServer(String server, int port);
  void setServerAuth(String user, String pass);

  bool publish(String topic, String payload);
  bool subscribe(String topic);
  bool unsubscribe(String topic);

  void onMessage(MQTHING_MESSAGE_FUNC *func);
  void onConnect(MQTHING_CONNECT_FUNC *func);

	void loop();
};
