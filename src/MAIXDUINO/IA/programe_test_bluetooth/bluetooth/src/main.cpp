#include <Arduino.h>
#include "BluetoothSerial.h"


BluetoothSerial SerialBT;

// put function declarations here:

int resultat;

void setup() {
  SerialBT.begin("ESP32");
  Serial.begin(115200);
  delay(3000);
  SerialBT.println("OK");
}

void loop() {
  if(Serial.available())SerialBT.write(Serial.read());
  if(SerialBT.available())Serial.write(SerialBT.read());
}
