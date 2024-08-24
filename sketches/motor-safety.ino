#include <SoftwareSerial.h>  //Create software serial object to communicate with SIM800L
SoftwareSerial GSM(8, 9);    //SIM800L Tx & Rx is connected to Arduino #8 & #9

char phone_no1[] = "+923217432026";
char phone_no2[] = "+923074257049";

#define bt_M A0  // Button1 Pin A0 use for Send Message
#define bt_C A1  // Button2 Pin A1 use for Call

#define LED 2  // Led Pin D2 use for LED

char inchar;  // Will hold the incoming character from the GSM shield

void setup() {  // put your setup code here, to run once

  Serial.begin(9600);  //Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
  GSM.begin(9600);     //Begin serial communication with Arduino and SIM800L

  pinMode(bt_M, INPUT_PULLUP);  // declare bt_M as input
  pinMode(bt_C, INPUT_PULLUP);  // declare bt_C as input

  pinMode(LED, OUTPUT);  // declare LED as output

  Serial.println("Initializing....");
  initModule("AT", "OK", 1000);                 //Once the handshake test is successful, it will back to OK
  initModule("ATE1", "OK", 1000);               //this command is used for enabling echo
  initModule("AT+CPIN?", "READY", 1000);        //this command is used to check whether SIM card is inserted in GSM Module or not
  initModule("AT+CMGF=1", "OK", 1000);          //Configuring TEXT mode
  initModule("AT+CNMI=2,2,0,0,0", "OK", 1000);  //Decides how newly arrived SMS messages should be handled
  Serial.println("Initialized Successfully");
}

void loop() {

  if (GSM.available() > 0) {
    inchar = GSM.read();
    Serial.print(inchar);
    if (inchar == 'R') {
      inchar = GSM.read();
      if (inchar == 'I') {
        inchar = GSM.read();
        if (inchar == 'N') {
          inchar = GSM.read();
          if (inchar == 'G') {
            initModule("ATH", "OK", 1000);  // this command is used for Call busy
          }
        }
      }
    }

    else if (inchar == 'L') {
      delay(10);
      inchar = GSM.read();
      if (inchar == 'E') {
        delay(10);
        inchar = GSM.read();
        if (inchar == 'D') {
          delay(10);
          inchar = GSM.read();

          if (inchar == '1') {
            digitalWrite(LED, 1);
            sendSMS(phone_no1, "LED is On");
            sendSMS(phone_no2, "LED is On");
          } else if (inchar == '0') {
            digitalWrite(LED, 0);
            sendSMS(phone_no1, "LED is Off");
            sendSMS(phone_no2, "LED is Off");
          }
        }
      }
    }
  }


  if (digitalRead(bt_M) == 0) {
    sendSMS(phone_no1, "Hello Muhammad Ansar from GSM SMS");
  }
  if (digitalRead(bt_C) == 0) {
    callUp(phone_no1, phone_no2);
  }

  delay(5);
}



void sendSMS(char *number, char *msg) {
  GSM.print("AT+CMGS=\"");
  GSM.print(number);
  GSM.println("\"\r\n");  //AT+CMGS=”Mobile Number” <ENTER> - Assigning recipient’s mobile number
  delay(500);
  GSM.println(msg);  // Message contents
  delay(500);
  GSM.write(byte(26));  //Ctrl+Z  send message command (26 in decimal).
  delay(3000);
}

void callUp(char *number1, char *number2) {
  int callPicked = 0;
  int numberIteration = 1;
  while (callPicked == 1) {
    if (numberIteration == 1) {
      Serial.println("Dialing number1");
      GSM.print("ATD + ");GSM.print(number1);GSM.println(";");
      numberIteration = 2;
    } else {
      Serial.println("Dialing number2");
      GSM.print("ATD + ");GSM.print(number2);GSM.println(";");
      numberIteration = 1;
    }

    // Wait for response with timeout
    unsigned long startTime = millis();
    String response = "";
    while (millis() - startTime < 30000) {  // 30 second timeout
      if (GSM.available()) {
        char c = GSM.read();
        response += c;
        if (response.indexOf("NO CARRIER") != -1) {
          break;
        }
        if (response.indexOf("BUSY") != -1) {
          break;
        }
        if (response.indexOf("NO DIALTONE") != -1) {
          break;
        }
        if (response.indexOf("OK") != -1 && response.indexOf("CONNECT") != -1) {
          callPicked = 1;
          break;
        }
      }
      delay(10);
    }

    Serial.println("Response of call: " + response);
    GSM.println("ATH");  // Hang up
    delay(5000);
  }
}


void initModule(String cmd, char *res, int t) {
  while (1) {
    Serial.println(cmd);
    GSM.println(cmd);
    delay(100);
    while (GSM.available() > 0) {
      if (GSM.find(res)) {
        Serial.println(res);
        delay(t);
        return;
      } else {
        Serial.println("Error");
      }
    }
    delay(t);
  }
}