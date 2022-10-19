#include <WiFi101.h>   //per arduino mkr1000
#include <PubSubClient.h>
#include "SR04.h"
#include <SPI.h>
#include <ArduinoJson.h>
#include <Servo.h>
#include <LiquidCrystal.h>

#define PIN_SERVO 3
#define TRIG_PIN 4 //pin per sensore ultrasuono
#define ECHO_PIN 5 //pin per sensore ultrasuono
#define PIN_ir_enter 13 //sbarra ingresso
#define PIN_ir_back 14 //sbarra uscita

const char WIFI_SSID[] = "AngeloP";  //nota: la conness deve essere 2.4GHz
const char WIFI_PASS[] = "angelo123";
int port = 8081;
WiFiClient wifi;
WiFiClient clientREST;

int status = WL_IDLE_STATUS;
IPAddress server(192,168,43,137);  //192,168,43,137
PubSubClient clientMQTT(wifi);

SR04 sr04 = SR04(ECHO_PIN,TRIG_PIN);
long a;       //valore letto dal sensore ad ultrasuoni
int id=0,i;
Servo servo;
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
int flag1=0,flag2=0;
int slot=3;
boolean enableGate=false, flag3= false;
String response;
unsigned long StartTime,CurrentTime,ElapsedTime;

/* il client si connette alla Wi-Fi
 * clientMQTT si connette al broker mqtt sulla porta 1883.
 * clientREST si connette al server tomcat attivo su porta 8081*/
void setup() {
  Serial.begin(9600);
  pinMode(PIN_SERVO,OUTPUT);
  servo.attach(PIN_SERVO);
  lcd.begin(16, 2);
  lcd.print("Car Parking!");
  lcd.setCursor(0, 1);
  lcd.print("Slot left: ");
  lcd.print(slot);
  
  while(!Serial){;}
  Serial.println("ok...connessione...");
  while(WiFi.status() != WL_CONNECTED){
    status=WiFi.begin(WIFI_SSID, WIFI_PASS);
    delay(500);
  }
  Serial.println("Connected to WiFi\n");
  clientMQTT.setServer(server,1883);
  
  if(clientMQTT.connect("arduino1")){
    Serial.println("MQTT connected"); 
  }else{
    Serial.println("MQTT not connected");
    Serial.println(clientMQTT.state());
  } 

  if (clientREST.connect(server,8081)) {
    Serial.println("connected to server");
  } else {
    Serial.println(" not connected to server");
  }
}

void loop() {
  clientMQTT.loop();
  if (!clientMQTT.connected()) {
    reconnect();
  }
  if (!clientREST.connected()) {
    reconnectREST();
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
  }else{
    Serial.println("Parking full");
  }
 }

 /* Apre la sbarra a seguito del passaggio sotto il 1° sensore del gate (in uscita).
  * flag2=1 per ricordare che un auto sta uscendo dal parcheggio.
  * flag1=0 per assicurarsi dell'assenza di un'auto in ingresso mentre si sta uscendo.
  * slot<=2 blocca il n° di auto in uscita dal parcheggio, rispettando la capienza. */
 if(digitalRead (PIN_ir_back) == LOW && flag2==0 && slot<=2){
  flag2=1;
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
   flag1=0, flag2=0;
   servoMotoreClose();
   slot = slot-1;
   display();
   flag3= false;
   enableGate=false;
 }
 
 /* Chiude la sbarra a seguito del passaggio sotto il 1° e 2° sensore del gate (in uscita).
  * flag2=1 ricorda il passaggio dell'auto sotto il 2° sensore. 
  * flag1 e flag2 sono posti a zero per permettere il successivo ingresso/uscita di un'auto.
  * slot viene incrementato a seguito dell'uscita di un'auto dal parcheggio. 
  * clientMQTT effettua il publish per segnalare all'AI_Emulator l'uscita di un'auto dal parcheggio. */
 if(flag2==1 && digitalRead(PIN_ir_enter)==LOW){
   while(digitalRead(PIN_ir_enter)==LOW){ //Serial.println("aspetto che oltrepassi il sensore");
   }
   flag1=0, flag2=0;
   servoMotoreClose();
   slot = slot+1;
   display(); 
   id++;
   char out[128];

   StaticJsonDocument<256> message;
   message["id"] = id;
   message["mod"] = "esce";
   serializeJson(message, out);

   clientMQTT.publish("/topic/car",out);
 }
}
//---------------------------------------------------//----------------------------------------------------//

/* Viene chiamata a seguito dell'evento di trigger. 
 * clientMQTT effettua il publish per segnalare all'AI_Emulator che un'auto vorrebbe entrare nel parcheggio.
 * flag3=true blocca il lancio di più eventi di trigger quando l'auto rimane davanti al sensore ultrasuoni.
 * clientREST effettua il polling finchè non ottiene lo stato dall'AI_Emulator. Quindi effettua una GET HTTP,
 * legge la risposta, estrae i posti_disponibili ed effettua il controllo di stato. 
 * enableGate=true abilita l'ingresso dell'auto nel parcheggio.
 * flag3=false abilita l'evento di trigger anche con parking_full.*/
void passaggio_auto(){
  id++;
  char out[128];
  Serial.println("Auto all'ingresso.");

  StaticJsonDocument<256> message;
  message["id"] = id;
  message["mod"] = "entra";

  serializeJson(message, out);
  StartTime = millis();
  clientMQTT.publish("/topic/car",out);
  
  flag3=true;
  delay(1000);  //1 sec per attendere l'update stato

  // Make a HTTP request:
  int posti_disponibili=0;
  char status_code;
 do{
   if(clientREST.connected()){
      clientREST.println("GET /AIEmulator2/DriverRest/status/"+String(id)+" HTTP/1.1");
      clientREST.println("Host: 127.0.0.1");
      clientREST.println("Connection: close");
      clientREST.println();
    }else{clientREST.println("ClientREST non connesso");}
  
    while (!clientREST.available()){}
    String res="";
    if(clientREST.connected()){
      while(clientREST.available()){
        char c=clientREST.read();
        res+=c;
      }
      status_code=res[9];
      if(status_code=='2'){
        int posizione = res.indexOf('{');
        String stato=res.substring(posizione); 
        posti_disponibili=atoi(&stato[21]);
       }else {Serial.print("404 Not Found");
              lcd.setCursor(0, 0);
              lcd.print("Status code:  ");
              lcd.setCursor(0, 1);
              lcd.print("404 Not Found  ");
              clientREST.stop();
              reconnectREST();
       }
    }else{Serial.print("non connesso");}
  clientREST.flush();
 }while(status_code!='2');
  clientREST.stop();  //forzare lo stop del clientREST e rifare la connessione
  display();

  CurrentTime = millis(); 
  ElapsedTime = CurrentTime - StartTime;  Serial.println(ElapsedTime);
  Serial.print("Posti disponibili: ");
  Serial.println(posti_disponibili);
  
  if(posti_disponibili==slot){
    if(slot >0)
      enableGate=true;     
    else {
      Serial.println("Parking full");
      lcd.setCursor(0, 0);
      lcd.print("Parking full!  ");
      lcd.setCursor(0, 1);
      lcd.print("            ");
      delay(3000); //per non far inviare tanti eventi quando il park è full
      flag3=false;
    }
  }else{
    Serial.print("ERRORE nel controllo di stato. I posti disponibili non corrispondono.");
    // invio del messaggio alla raspberry collegata al monitor della smart_city
  }
 }

/* Chiude la sbarra del gate.
 * Ad ogni iterazione la sbarra viene abbassata di un grado.*/
void servoMotoreClose(){
  for (i=1; i<=100; i++){
      servo.write(i); 
      delay(15);
  }
}

/* Apre la sbarra del gate.
 * Ad ogni iterazione la sbarra viene alzata di un grado.*/
void servoMotoreOpen(){
  for (i=100; i>=0; i--){
      servo.write(i); 
      delay(15);
  }
}

/* mostra la stampa delle informazioni sul display di arduino.*/
void display(){
  lcd.setCursor(0, 0);
  lcd.print("Car Parking!  ");
  lcd.setCursor(0, 1);
  lcd.print("Slot left: ");
  lcd.print(slot);
  lcd.print("    ");
}

/* clientMQTT si riconnette al broker mqtt quando viene persa la connessione.*/
void reconnect() {
  // Loop until we're reconnected
  while (!clientMQTT.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (clientMQTT.connect("arduino1")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(clientMQTT.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/* clientREST si riconnette al server tomcat quando viene persa la connessione.*/
void reconnectREST() {
  // Loop until we're reconnected
  while (!clientREST.connected()) {
    Serial.print("Attempting REST connection...");
    // Attempt to connect
    if (clientREST.connect(server,8081)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(clientREST.status());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      //delay(5000);
    }
  }
}




 
