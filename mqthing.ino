#include "MqThing.h"

MqThing thing;

void on_connect() {
  Serial.println("Yey... Connect...");

  thing.subscribe("green");
  thing.publish("hello","connected...");
}

void on_message(String topic, String payload) {
  Serial.println(String("In: ")+topic+String(": ")+payload);

  if (topic=="green") {
    if (payload=="on") {
      digitalWrite(D6,HIGH);
    }

    else {
      digitalWrite(D6,LOW);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");

  thing.setButtonPin(D1);
  thing.setLedPin(D7);
  thing.setName("Mavis");
  thing.setServer("postman.cloudmqtt.com",13342);
  thing.setServerAuth("hbpiywwf","q6YEkRNQwT4x");
  thing.onConnect(on_connect);
  thing.onMessage(on_message);

  pinMode(D6,OUTPUT);
}

void loop() {
  thing.loop();
}
