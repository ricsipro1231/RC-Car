#include "SerialUART.h"
#include "Bluetooth.h"
#include <string.h>
#include <Arduino.h>

Bluetooth::Bluetooth() {

}

bool Bluetooth::Init() {
  Serial2.begin(9600);

  while(!Serial2);

  return true;
}

char Bluetooth::ReadMessage() {
  char Data;

  if(Serial2.available()) {
    Data = Serial2.read();
  }

  return Data;
}

String Bluetooth::ReadString() {
  String message = "";

  if(Serial2.available()) {
    message = Serial2.readString();
  }

  return message;
}
