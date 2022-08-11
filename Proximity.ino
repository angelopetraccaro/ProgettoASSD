#include <WiFi101.h>   //per arduino mkr1000
#include <PubSubClient.h>
#include "SR04.h"

const char WIFI_SSID[] = "EOLO_079848";  //nota: la conness deve essere 2.4GHz
const char WIFI_PASS[] = "b6u4fTLAq";
WiFiClient wifi;
int status = WL_IDLE_STATUS;
IPAddress server(192,168,1,108);  //192,168,43,137
PubSubClient client(wifi);

#define TRIG_PIN 4 //pin per sensore ultrasuono
#define ECHO_PIN 5 //pin per sensore ultrasuono

SR04 sr04 = SR04(ECHO_PIN,TRIG_PIN);
long a;       //valore letto dal sensore ad ultrasuoni
int id=0;

void setup() {
  Serial.begin(9600);
  
  while(!Serial){;}
  Serial.println("ok...connessione...");
  while(WiFi.status() != WL_CONNECTED){
    status=WiFi.begin(WIFI_SSID, WIFI_PASS);
    delay(500);
  }
  Serial.println("Connected to WiFi\n");
 
  client.setServer(server,1883);

  if(client.connect("arduino1")){
    Serial.println("MQTT connected");  
  }else{
    Serial.println("MQTT not connected");
    Serial.println(client.state());
  } 
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
 //lettura e stampa su monitor la posizione dell'auto da Proximity sensor (ultrasuono)
 a=sr04.Distance();
 Serial.print(a);
 Serial.println("cm");
 if(a<=5)
  passaggio_auto();
}


void passaggio_auto(){
  id++;
  char cstr[16];
  itoa(id, cstr, 10);
  Serial.println("Auto all'ingresso.");
  client.publish("/topic/car",cstr);
  delay(5000);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduino1")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
