
/* DroneBot */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "HTTPSRedirect.h"

//Ultrasonic sensor
const int Trig_pin = D2;
const int Echo_pin = D3;
const int MAX_DISTANCE = 300;

long duration, distance; //กำหรดตัวแปรเก็บค่าระยะ
int percentage;

// Enter network credentials:
const char* ssid     = "TENTENHOME_2.4G TOT";
const char* password = "timmy9649";

// Enter Google Script Deployment ID: CHANGE IF YOU CREATED NEW SPREADSHEET AND NEW DEPLOYMENT
const char *GScriptId = "AKfycbwhKHPdY3uxKjdvneRt2aey6H4uwLfTSrINZU1nyCGCakjtOOUF5yoH87MdK-SHNnuCGw";

// Enter command (insert_row or append_row) and your Google Sheets sheet name (default is Sheet1):
String payload_base =  "{\"command\": \"insert_row\", \"sheet_name\": \"Sheet1\", \"values\": ";
String payload = "";

// Google Sheets setup (do not edit)
const char* host = "script.google.com";
const int httpsPort = 443;
const char* fingerprint = "";
String url = String("/macros/s/") + GScriptId + "/exec";
HTTPSRedirect* client = nullptr;

void setup() {

  Serial.begin(9600);
  pinMode(Trig_pin, OUTPUT);
  pinMode(Echo_pin, INPUT);

  // Connect to WiFi
  WiFi.begin(ssid, password);             
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());

  // Use HTTPSRedirect class to create a new TLS connection
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");
  
  Serial.print("Connecting to ");
  Serial.println(host);

  // Try to connect for a maximum of 5 times
  bool flag = false;
  for (int i=0; i<5; i++){ 
    int retval = client->connect(host, httpsPort);
    if (retval == 1){
       flag = true;
       Serial.println("Connected");
       break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }
  if (!flag){
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    return;
  }
  delete client;    // delete HTTPSRedirect object
  client = nullptr; // delete HTTPSRedirect object
}


void loop() {
long duration;
  digitalWrite(Trig_pin, LOW);
  delayMicroseconds(5);
  digitalWrite(Trig_pin, HIGH);
  delayMicroseconds(5);
  digitalWrite(Trig_pin, LOW);
  duration = pulseIn(Echo_pin, HIGH);
  //distance = duration * 0.034/2;
  distance = (duration/2) / 29.1; //คำนวณเป็น centimeters  
  percentage = map(distance, 0, 25, 0, 100);

  if(percentage < 0) {
    percentage = 0;
  }
  else if(percentage > 100) {
    percentage = 100;
  }
  Serial.print("Percentage: ");
  Serial.print(percentage);
  Serial.print("%   Distance: ");
  Serial.print(distance);
  Serial.print(" cm");

  static bool flag = false;
  if (!flag){
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    flag = true;
    client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
  }
  if (client != nullptr){
    if (!client->connected()){
      client->connect(host, httpsPort);
    }
  }
  else{
    Serial.println("Error creating client object!");
  }
  
  // Create json object string to send to Google Sheets
  payload = payload_base + "\"" + distance + "," + percentage + "\"}";
  
  // Publish data to Google Sheets
  Serial.println(" Publishing data...");
  Serial.println(payload);
  if(client->POST(url, host, payload)){ 
    // do stuff here if publish was successful
  }
  else{
    // do stuff here if publish was not successful
    Serial.println("Error while connecting");
  }

  // a delay of several seconds is required before publishing again    
  delay(5000);
}
