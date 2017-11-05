#include <SoftwareSerial.h>
#include <MillisTimer.h>
#include <assert.h>

#define SIM800_RX_PIN 7
#define SIM800_TX_PIN 8

#define RELAY_1_PIN 6
#define RELAY_2_PIN 9
#define WRONG_RELAY_PIN -1

#define RELAY_COUNT 2

//because here is used a negative relays
#define RELAY_OFF HIGH
#define RELAY_ON LOW

typedef enum{
  InvalidRelayIndex=-1,
  Relay1=0,
  Relay2,
  RelayCount
} RelayIndexes;

class RelayController;

RelayController* relays[(int)RelayCount] = {0,0};

class RelayController {
  public:
    RelayController(int pin, int timeout)
      : m_pin(pin)
      , m_timer(MillisTimer(timeout)){
      assert(m_pin != WRONG_RELAY_PIN);
      assert(timeout > 10);
      
      digitalWrite(m_pin, RELAY_OFF);
      pinMode(m_pin, OUTPUT);
      int i =0;i;
      Serial.print("Relay ");
      Serial.print("on ");
      Serial.print(m_pin);
      Serial.println(" is ok");
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

    int m_pin = WRONG_RELAY_PIN;
    MillisTimer m_timer;
};

void initRelays() {
  relays[0] = new RelayController(RELAY_1_PIN, 3000);
  relays[1] = new RelayController(RELAY_2_PIN, 2000);
}

void updateRelays(){
  for (int i = 0; i < (int)RelayCount; ++i) {
    RelayController* relay = relays[i];
    if (relay != 0)
      relay->update();
  }
}

SoftwareSerial serialSIM800(SIM800_RX_PIN, SIM800_TX_PIN);//Rx Tx

class Sim800Reader{
public:
  static String getResponse(int readTimeout){
    Sim800Reader reader(readTimeout);
    
    String result;
    reader.m_timer.start();
    while (reader.m_timer.isRunning()){
      if (serialSIM800.available()) result.concat((char)serialSIM800.read());
      reader.m_timer.run();
    }
    return result;
  }
private:
  Sim800Reader(int readTimeout)
  : m_timer(MillisTimer(readTimeout)){
    assert(readTimeout > 10);
    m_timer.expiredHandler(Sim800Reader::stopReading);
  }
  static void stopReading(MillisTimer& timer) {
    timer.stop();
  }
private:
  MillisTimer m_timer;
};

void waitForSim800Reply(){
  do{
    delay(30);
  }while(!serialSIM800.available());
}

void receiveSIM800Answer(){
  waitForSim800Reply();
  while (serialSIM800.available()){
    Serial.write(serialSIM800.read());
  }
  Serial.println();
}

#define SIM800_RESET_PIN 13

void resetSIM800(){
  Serial.println("Reset SIM800 --------");

  Serial.println("Set SIM800 OFF");//////////////////off
  pinMode(SIM800_RESET_PIN, OUTPUT);
  digitalWrite(SIM800_RESET_PIN, HIGH);
  delay(1000);
  digitalWrite(SIM800_RESET_PIN, LOW);
  delay(1000);

  Serial.println("Set SIM800 ON");///////////////////on
  pinMode(SIM800_RESET_PIN, INPUT);
  delay(3000);//wait for loading

//initial check
  serialSIM800.println("AT");
  Serial.println(Sim800Reader::getResponse(200));//OK

//set full functionality
  serialSIM800.println("AT+CFUN=1");
  Serial.println(Sim800Reader::getResponse(4000));//OK

//set SMS format to ASCII
  serialSIM800.println("AT+CMGF=1");
  Serial.println(Sim800Reader::getResponse(200));//OK
  
  Serial.println("Reset SIM800 DONE");
}

void setup() {
  Serial.begin(19200);
  while(!Serial){}
  
  Serial.println("Starting SIM800 SMS Command Processor");

  serialSIM800.begin(19200);
  while(!serialSIM800);

  resetSIM800();

  initRelays();

  Serial.println("Setup Complete!");

  delay(3000);
  tryHandleNewSms();
}


void turnOnRelay(RelayIndexes relayIndex){
  
  assert(relayIndex > InvalidRelayIndex);
  
  assert(relayIndex < RelayCount);

  relays[static_cast<const int>(relayIndex)]->turnOnRelay();
}

#define MAIN_SENDER_NUMBER "+71234567890"
#define END_OF_MESSAGE "^*^*OK^*"
#define NEW_NLRC "^*"

//AT+CMGL="ALL"
#define READ_SMS_LIST_COMMAND "AT+CMGR=1,0"
#define REMOVE_READ_SMS "AT+CMGD=1,0"

void tryHandleNewSms(){
  Serial.println("read List Of New Sms");

  serialSIM800.println(READ_SMS_LIST_COMMAND);
  String lst = Sim800Reader::getResponse(1000);

  lst.replace("\n","*");
  lst.replace("\r","^");
  
  Serial.println(lst);
  Serial.println("End read of new SMS");

  if (!lst.endsWith(END_OF_MESSAGE)){
    Serial.println("End is NOT ok");
    return;
  }
  Serial.println("End is OK");
  int pos = lst.indexOf(MAIN_SENDER_NUMBER);
  if (pos == -1){
    Serial.println("Unknown sender");
    return;
  }
  Serial.println("Sender is OK");
  //try separate message
  //cut of "OK"
  pos = lst.indexOf(END_OF_MESSAGE);
  lst.remove(pos);

  String message = lst.substring(lst.lastIndexOf(NEW_NLRC)+2);
  Serial.print("SMS message: ");Serial.println(message);

  if (message == "1")
      turnOnRelay(Relay1);
  else if (message == "2")
      turnOnRelay(Relay2);
  //remove SMS AT+CMGD=1,3
  Serial.println("Remove SMS");
  serialSIM800.println(REMOVE_READ_SMS);
  Sim800Reader::getResponse(200);
  Serial.println("Remove SMS DONE");
}

void onIncomingMessage(const String& atResponce){

//+CMTI: "SM",2
  if (atResponce.indexOf("+CMTI: \"SM\"") > -1){
    tryHandleNewSms();
  }
}

void loop() {
  updateRelays();

  if (serialSIM800.available()){
    Serial.print("SIM--");
    String lst = Sim800Reader::getResponse(500);

    lst.replace("\n","*");
    lst.replace("\r","^");

    Serial.print(lst);
    Serial.println("--SIM");

    onIncomingMessage(lst);
  }

  while (Serial.available()){
    char sr = Serial.read();
    if (sr == '~')
      tryHandleNewSms();
    else{
      serialSIM800.write(sr);
    }
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
