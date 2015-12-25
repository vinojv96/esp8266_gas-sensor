#include <ESP8266WiFi.h>
//#include <DNSServer.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <Wire.h>
#include "mq135-data.h"
#include "wifi-utils.h"
#include "wifi-creds.h"


char macStr[20];

void setup() {
  delay(1000);
  Serial.begin(115200);
  
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true)
    delay(10000);
  }

  
 // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:    
    status = WiFi.begin(ssid, pass);
    delay(10000);
  }
   
  // you're connected now, so print out the data:
  Serial.print("You're connected to the network");
  printCurrentNet();
  printWifiData();

  
    byte mac[6];  
    WiFi.macAddress(mac);
    sprintf(macStr,"%02x:%02x:%02x:%02x:%02x:%02x",mac[5],mac[4],mac[3],mac[2],mac[1],mac[0]);
}



void loop() {
  //dns.processNextRequest();  
  //server.handleClient();
  delay(5000);
  
  long valr = analogRead(A0);
  if(valr==0)
  {
    Serial.println("Sensor returned 0, smth is not right. Skipping loop.");
    return;
  }
  
  long val =  ((float)22000*(1023-valr)/valr); 
  long mq135_ro = mq135_getro(94010, 635);//8000;//mq135_getro(val, 500);
  //convert to ppm (using default ro)
  float valAIQ = mq135_getppm(val, mq135_ro);
  
  Serial.println("val raw = "+String(valr)+",val = "+String(val)+",ro = "+String(mq135_ro)+" ppm = "+String(valAIQ));
  if(valAIQ<=0)
    return;
      
  Serial.println("\nChecking connection...");
  wifiCheckReconnect(ssid,pass);
  printCurrentNet();
  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  if (client.connect("co2.jehy.ru", 80)) 
  {
    Serial.println("connected to server");  
    // Make a HTTP request:
    client.println("GET /send.php?data={\"id\":1,\"val\":"+String(valr)+",\"ppm\":"+String((int)valAIQ)+
    ",\"mac\":\""+String(macStr)+"\",\"SSID\":\""+WiFi.SSID()+"\"} HTTP/1.1");
    client.println("Host: co2.jehy.ru");
    client.println("Connection: close");
    client.println();
    client.stop();
    Serial.println("Request sent");
  }
}


