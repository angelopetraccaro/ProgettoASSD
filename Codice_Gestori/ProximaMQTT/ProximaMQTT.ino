#include <WiFi101.h>   //per arduino mkr1000
#include <PubSubClient.h>
#include "SR04.h"
#include <ArduinoJson.h>
#include <Servo.h>
#include <LiquidCrystal.h>

#define PIN_SERVO 3
#define TRIG_PIN 4 //pin per sensore ultrasuono
#define ECHO_PIN 5 //pin per sensore ultrasuono
#define PIN_ir_enter 13 //sbarra ingresso
#define PIN_ir_back 14 //sbarra uscita

const char WIFI_SSID[] = "TP-Link_0DBE";  //nota: la conness deve essere 2.4GHz
const char WIFI_PASS[] = "PROXIMACity";
WiFiClient wifi;
int status = WL_IDLE_STATUS;
IPAddress server(192,168,0,50);  //192,168,43,137
PubSubClient client(wifi);

SR04 sr04 = SR04(ECHO_PIN,TRIG_PIN);
long a;       //valore letto dal sensore ad ultrasuoni
int id=0,gateid=1,i;
int flag1=0,flag2=0;
int slot=-1;
boolean enableGate=false,flag3=false, mod=false,flag5 = false,flag6=false;
unsigned long StartTime,CurrentTime,ElapsedTime;
Servo servo;
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

/* il client si connette alla Wi-Fi
 * client si connette al broker mqtt sulla porta 1883.
 * client si sottoscrive al topic "topic/status" e setta la callback.*/
void setup() {
  pinMode(PIN_SERVO,OUTPUT);
  servo.attach(PIN_SERVO);
  
  while(WiFi.status() != WL_CONNECTED){
    status=WiFi.begin(WIFI_SSID, WIFI_PASS);
    delay(500);
  }
 
  client.setServer(server,1883);

  if(client.connect("arduino1")){
    //Serial.println("MQTT connected"); 
    //sottoscrizione topic "status" per lettura stato ProximaCity 
    client.subscribe("parking/status/free",1);  
  }//else{
    //Serial.println("MQTT not connected");
    //Serial.println(client.state());
  //} 
  client.setCallback(callback);

  lcd.begin(16, 2);
  lcd.setCursor(2, 0);
  lcd.print("Car Parking!");
  lcd.setCursor(0, 1);
  lcd.print("Inizializzazione");
}

void loop() {
  client.loop();
  if (!client.connected()) {
    reconnect();
  }
  
 a=sr04.Distance();
 //Serial.print(a);
 //Serial.println("cm");
 if(a<=5 && !flag3)
  passaggio_auto();

 /* Apre la sbarra a seguito del passaggio sotto il 1° sensore del gate (in entrata).
  * flag1 viene settato a 1 per ricordare che un auto sta entrando nel parcheggio.
  * ir sensor sono sempre 1, 0 quando trovano l'ostacolo.  */
 if(digitalRead (PIN_ir_enter) == LOW && flag1==0 && enableGate == true){
  if(slot>0){
    flag1=1; 
    if(flag2==0){  
      servoMotoreOpen(); 
    }
  }//else{
   // Serial.println("Parking full");
  //}
 }

 /* Apre la sbarra a seguito del passaggio sotto il 1° sensore del gate (in uscita).
  * flag2=1 per ricordare che un auto sta uscendo dal parcheggio.
  * flag1=0 per assicurarsi dell'assenza di un'auto in ingresso mentre si sta uscendo.
  * slot<=2 blocca il n° di auto in uscita dal parcheggio, rispettando la capienza
  * flag5 serve per abilitare l'uscita dal gate quando l'id è zero. 
  * flag6 blocca l'uscita quando un'auto sta entrando ed ha lanciato l'evento di trigger.*/
 if((digitalRead (PIN_ir_back) == LOW && flag2==0 && !flag6 && slot<=5 && slot>=0 && flag5) || (digitalRead (PIN_ir_back) == LOW && !flag5 && !flag6)){
  flag2=1; 
  flag5=true;
  if(flag1==0){
    servoMotoreOpen();
  }
 }

 /* Chiude la sbarra a seguito del passaggio sotto il 1° e 2° sensore del gate (in ingresso).
  * flag1=1 ricorda il passaggio dell'auto sotto il 1° sensore. 
  * flag1 e flag2 sono posti a zero per permettere il successivo ingresso/uscita di un'auto.
  * slot viene decrementato a seguito dell'ingresso di un'auto nel parcheggio.
  * enableGate=false per evitare l'ingresso delle auto in assenza dell'evento di trigger. 
  * flag3=false abilita l'evento di trigger. */
 if(flag1==1 && digitalRead(PIN_ir_back)==LOW){
   while(digitalRead(PIN_ir_back)==LOW){ //Serial.println("aspetto che oltrepassi il sensore"); 
   }
   servoMotoreClose();
   slot = slot-1;
   display();
   enableGate=false;
   flag3=false;
   flag6 = false;
 }
 
 /* Chiude la sbarra a seguito del passaggio sotto il 1° e 2° sensore del gate (in uscita).
  * flag2=1 ricorda il passaggio dell'auto sotto il 2° sensore. 
  * flag1 e flag2 sono posti a zero per permettere il successivo ingresso/uscita di un'auto.
  * slot viene incrementato a seguito dell'uscita di un'auto dal parcheggio. 
  * client effettua il publish per segnalare all'AI_Emulator l'uscita di un'auto dal parcheggio.
  * mod=true rappresenta la modalità:'uscita di un'auto' e non serve il controllo di stato.*/
 if(flag2==1 && digitalRead(PIN_ir_enter)==LOW){
   while(digitalRead(PIN_ir_enter)==LOW){ //Serial.println("aspetto che oltrepassi il sensore"); 
    }
   mod=true;
   servoMotoreClose();
   slot = slot+1;
   display();
   
   id++;
   char out[128];
   StaticJsonDocument<256> message;
   message["id"] = id;
   message["gateid"] = gateid;
   message["mod"] = "esce";
   serializeJson(message, out);
   client.publish("parking/car",out,1);
   delay(6000);  //per attendere elaborazione dell'AI
 }
}
//---------------------------------------------------//----------------------------------------------------//

/* Viene chiamata a seguito dell'evento di trigger. 
 * client effettua il publish per segnalare all'AI_Emulator che un'auto vorrebbe entrare nel parcheggio.
 * flag3=true blocca il lancio di più eventi di trigger quando l'auto rimane davanti al sensore ultrasuoni.
 * mod=false rappresenta la modalità 'ingresso di un'auto', mod=true rappresenta 'uscita di un'auto' 
 * flag6 blocca l'uscita quando un'auto sta entrando ed ha lanciato l'evento di trigger.*/
void passaggio_auto(){
  id++;
  char out[128];
  Serial.println("Auto all'ingresso.");

  StaticJsonDocument<256> message;
  message["id"] = id;
  message["gateid"] = gateid;
  message["mod"] = "entra";
  flag6 = true;

  lcd.setCursor(0, 0);
  lcd.print("Ho lanciato      ");
  lcd.setCursor(0, 1);
  lcd.print("l'evento        ");

  serializeJson(message, out);
  StartTime = millis();
  client.publish("parking/car",out,1);

  lcd.setCursor(0, 0);
  lcd.print("Evento lanciato!   ");
  lcd.setCursor(0, 1);
  lcd.print("Prosegui...      ");
  
  flag3=true; 
  mod=false;
  delay(6000);
  flag3=false;
}

/* Chiude la sbarra del gate.
 * Ad ogni iterazione la sbarra viene abbassata di un grado.*/
void servoMotoreClose(){
  for (i=1; i<=100; i++){
      servo.write(i); 
      delay(15);
  }
  flag1=0, flag2=0;
}

/* Apre la sbarra del gate.
 * Ad ogni iterazione la sbarra viene alzata di un grado.*/
void servoMotoreOpen(){
  for (i=100; i>=0; i--){
      servo.write(i); 
      delay(15);
  }
}

/* client si riconnette al broker mqtt quando viene persa la connessione.*/
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    //Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduino1")) {
      //Serial.println("connected");
    } else {
      //Serial.print("failed, rc=");
      //Serial.print(client.state());
      //Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/* mostra la stampa delle informazioni sul display di arduino.*/
void display(){
  lcd.setCursor(0, 0);
  lcd.print("Car Parking!    ");
  lcd.setCursor(0, 1);
  lcd.print("Slot left: ");
  lcd.print(slot);
  lcd.print("    ");
}

/* viene chiamata a seguito della presenza di un msg sul topic "topic/status".
 * legge il msg, lo converte in Json ed estrae i posti disponibili.
 * Si effettua il controllo di stato 'posti_disp==slot' 
 * enableGate=true abilita l'ingresso dell'auto nel parcheggio.
 * flag3=false abilita l'evento di trigger anche con parking_full.*/
 
void callback(char* topic, byte* payload, unsigned int length){
  String msg; //per trasformare il payload in string, byte per byte
  //Serial.println("Message payload:");
  for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
    msg += (char)payload[i];  
  }
  
  StaticJsonDocument<200> doc; //per trasformare il msg in json
  deserializeJson(doc, msg);
  int posti_disponibili  = doc["postiliberi"];

  // Alla prima interazione gli slot vengono settati da AI_Emulator
  if(id==1) slot=posti_disponibili; 
  display();
   
  CurrentTime = millis(); 
  ElapsedTime = CurrentTime - StartTime;  Serial.println(ElapsedTime);
  //Serial.print("Posti disponibili: ");
  //Serial.println(posti_disponibili);

  if(!mod){
    if(posti_disponibili==slot){
      if(slot >0)
        enableGate=true;     
      else {
        Serial.println("Parking full");
        lcd.setCursor(0, 0);
        lcd.print("Parking full!  ");
        lcd.setCursor(0, 1);
        lcd.print("            ");
        flag3=false;
        flag6 = false;
        delay(3000); //per non far inviare tanti eventi quando il park è full
      }
    }else{
      //Serial.print("ERRORE nel controllo di stato. I posti disponibili non corrispondono.");
      // invio del messaggio alla raspberry collegata al monitor della smart_city
      lcd.setCursor(0, 0);
      lcd.print("Errore nel contr");
      lcd.setCursor(0, 1);
      lcd.print("ollo di stato(");
      lcd.print(slot);
      lcd.print(")");
    }
  }
}
