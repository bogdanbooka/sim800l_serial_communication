#include <SoftwareSerial.h>

#define SIM800_RX_PIN 7
#define SIM800_TX_PIN 8
#define SIM800_RESET_PIN 13

#define RELAY_1_PIN 6
#define RELAY_2_PIN 9

//because here is used a negative relays
#define RELAY_OFF HIGH
#define RELAY_ON LOW

SoftwareSerial serialSIM800(SIM800_RX_PIN, SIM800_TX_PIN);//Rx Tx

void resetSIM800(){
  Serial.println("Reset SIM800");
  pinMode(SIM800_RESET_PIN, OUTPUT);
  digitalWrite(SIM800_RESET_PIN, HIGH);
  delay(500);
  digitalWrite(SIM800_RESET_PIN, LOW);
  delay(500);
  pinMode(SIM800_RESET_PIN, INPUT);
  delay(1000);
  Serial.println("Reset SIM800 DONE");
}

void initRelays(){
  Serial.println("Init Relays");

  digitalWrite(RELAY_1_PIN, RELAY_OFF);
  pinMode(RELAY_1_PIN, OUTPUT);
 
  digitalWrite(RELAY_2_PIN, RELAY_OFF);
  pinMode(RELAY_2_PIN, OUTPUT);
  Serial.println("Init Relays DONE");
}

void setup() {
  Serial.begin(19200);
  while(!Serial){}
  
  Serial.println("Starting SIM800 SMS Command Processor");

  serialSIM800.begin(19200);
  while(!serialSIM800){}

  resetSIM800();

  initRelays();

  Serial.println("Setup Complete!");
}

void loop() {
 
    while (serialSIM800.available()){
      Serial.write(serialSIM800.read());
    }

    while (Serial.available()){
      serialSIM800.write(Serial.read());
    }
}
