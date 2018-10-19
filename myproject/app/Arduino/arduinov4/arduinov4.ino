#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
char* ssid = "TP-LINK_B1027A";
char* password = "64959900";
const int yesbuttonPin = 5;    // the number of the pushbutton pin
const int nobuttonPin = 4;
const int ledPin = 13;      // the number of the LED pin
const int yesled = 2;
const int noled = 0;

// Variables will change:
int ledState = HIGH;         // the current state of the output pin
int yesbuttonState;             // the current reading from the input pin
int nobuttonState;
int lastyesButtonState = LOW;   // the previous reading from the input pin
int lastnoButtonState = LOW;

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastyesDebounceTime = 0;  // the last time the output pin was toggled
unsigned long lastnoDebounceTime = 0;
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

void setup() {
  pinMode(yesbuttonPin, INPUT);
  pinMode(nobuttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(yesled, OUTPUT);
  pinMode(noled,OUTPUT);
  // set initial LED state
  digitalWrite(ledPin, ledState);
  digitalWrite(yesled, LOW);
  digitalWrite(noled,LOW);
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.println("Waiting for connections...");
  }
  Serial.println("Wifi Connected!");
}

void loop() {
  HTTPClient http;    //Declare object of class HTTPClient
  http.begin("http://192.168.1.106:5000/ArduinoInput");      //Specify request destination
  http.addHeader("Content-Type", "application/json");  //Specify content-type header
  StaticJsonBuffer<300> JSONbuffer;
  JsonObject& JSONencoder = JSONbuffer.createObject();
  // read the state of the switch into a local variable:
  int yesreading = digitalRead(yesbuttonPin);
  int noreading = digitalRead(nobuttonPin);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (yesreading != lastyesButtonState) {
    // reset the debouncing timer
    lastyesDebounceTime = millis();
  }
  else if(noreading != lastnoButtonState){
    lastnoDebounceTime= millis();
  }

  if ((millis() - lastyesDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (yesreading != yesbuttonState) {
      yesbuttonState = yesreading;
      

      // only toggle the LED if the new button state is HIGH
      if (yesbuttonState == HIGH) {
        ledState = !ledState;
        digitalWrite(yesled,HIGH);
        JSONencoder["input"] = "YES";
        char JSONmessageBuffer[300];
        JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
        Serial.println(JSONmessageBuffer);
        int httpCode = http.POST(JSONmessageBuffer);
        Serial.println(httpCode);
        http.end();
        delay(1000);
        digitalWrite(yesled,LOW);
      

      }
    }
  }
  if ((millis() - lastnoDebounceTime) > debounceDelay){
    if(noreading != nobuttonState){
      nobuttonState = noreading;
      if(nobuttonState == HIGH){
        unsigned long LastMillis = millis();
        ledState = !ledState;
        digitalWrite(noled,HIGH);
        unsigned long CurrentMillis = millis();
        JSONencoder["input"] = "NO";
        char JSONmessageBuffer[300];
        JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
        Serial.println(JSONmessageBuffer);
        int httpCode = http.POST(JSONmessageBuffer);
        Serial.println(httpCode);
        http.end();
        delay(1000);
        digitalWrite(noled,LOW);
        
      }
    }  
  }

  // set the LED:
  digitalWrite(ledPin, ledState);

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastyesButtonState = yesreading;
  lastnoButtonState = noreading;
}
