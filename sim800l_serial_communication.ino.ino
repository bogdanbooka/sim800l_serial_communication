#include <SoftwareSerial.h>

#define SIM800_RX_PIN 7
#define SIM800_TX_PIN 8
#define SIM800_RESET_PIN 13

#define RELAY_1_PIN 6
#define RELAY_2_PIN 9

SoftwareSerial serialSIM800(SIM800_RX_PIN, SIM800_TX_PIN);//Rx Tx

void resetSIM800(){
  Serial.println("Reset SIM800");
  pinMode(SIM800_RESET_PIN, OUTPUT);
  delay(500);
  digitalWrite(SIM800_RESET_PIN, HIGH);
  delay(500);
  digitalWrite(SIM800_RESET_PIN, LOW);
  delay(500);
  pinMode(SIM800_RESET_PIN, INPUT);
  Serial.println("Reset SIM800 DONE");
}

void resetRelays(){
  Serial.println("Reset Relays");
  pinMode(RELAY_1_PIN, OUTPUT);
  delay(500);
  digitalWrite(RELAY_1_PIN, LOW);
  delay(500);
  digitalWrite(RELAY_1_PIN, HIGH);
  delay(500);
 
  digitalWrite(RELAY_2_PIN, HIGH);
  pinMode(RELAY_2_PIN, OUTPUT);
//  delay(500);
  delay(500);
  digitalWrite(RELAY_2_PIN, HIGH);
  delay(500);
  Serial.println("Reset Relays DONE");//*/
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(19200);
  while(!Serial){}
  
  Serial.println("Starting SIM800 SMS Command Processor");


  serialSIM800.begin(19200);
  while(!serialSIM800){}
  Serial.println("Setup Complete!");

  resetSIM800();

  resetRelays();
}

void loop() {
 
    while (serialSIM800.available()){
      Serial.write(serialSIM800.read());
    }

    while (Serial.available()){
      serialSIM800.write(Serial.read());
    }
}
