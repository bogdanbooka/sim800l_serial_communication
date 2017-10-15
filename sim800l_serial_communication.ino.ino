#include <SoftwareSerial.h>
#include <MillisTimer.h>
#include <assert.h>

#define SIM800_RX_PIN 7
#define SIM800_TX_PIN 8
#define SIM800_RESET_PIN 13

#define RELAY_1_PIN 6
#define RELAY_2_PIN 9
#define WRONG_PIN -1

//because here is used a negative relays
#define RELAY_OFF HIGH
#define RELAY_ON LOW

#define RELAY_COUNT 2

class RelayController;

RelayController* relays[RELAY_COUNT] = {0,0};

class RelayController {
  public:
    RelayController(int pin, int timeout)
      : m_pin(pin)
      , m_timer(MillisTimer(timeout)){
      assert(m_pin != WRONG_PIN);
      assert(timeout > 10);
      
      digitalWrite(m_pin, RELAY_OFF);
      pinMode(m_pin, OUTPUT);
      Serial.print("Relay ");Serial.print(m_pin);Serial.println(" is ok");
    }

    void turnOnRelay(){
        if (m_timer.isRunning())
          return; 

        Serial.print("Relay ");Serial.print(m_pin);Serial.println(" is ON");
        digitalWrite(m_pin, RELAY_ON);

        m_timer.expiredHandler(RelayController::turnOffRelay);
        m_timer.start();
    }

    void update(){
      if (m_timer.isRunning())
        m_timer.run();
    }

  private:
    static void turnOffRelay(MillisTimer& timer) {
      timer.stop();

      for (int i = 0; i < RELAY_COUNT; ++i) {
        RelayController* relay = relays[i];
        if (relay->m_timer.ID == timer.ID){
          digitalWrite(relay->m_pin, RELAY_OFF);
          Serial.print("Relay ");Serial.print(relay->m_pin);Serial.println(" is OFF");
          return;
        }
      }
    }

    int m_pin = WRONG_PIN;
    MillisTimer m_timer;
};

void initRelays() {
  relays[0] = new RelayController(RELAY_1_PIN, 3000);
  relays[1] = new RelayController(RELAY_2_PIN, 2000);
}

void updateRelays(){
  for (int i = 0; i < RELAY_COUNT; ++i) {
    RelayController* relay = relays[i];
    if (relay != 0)
      relay->update();
  }
}

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

void __assert(const char *__func, const char *__file, int __lineno, const char *__sexp) {
  // transmit diagnostic informations through serial link.
  Serial.println(__func);
  Serial.println(__file);
  Serial.println(__lineno, DEC);
  Serial.println(__sexp);
  Serial.flush();
  // abort program execution.
  abort();
}
