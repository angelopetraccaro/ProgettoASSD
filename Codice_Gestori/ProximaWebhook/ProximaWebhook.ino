#include <SPI.h>
#include <WiFi101.h>
#include <PubSubClient.h>
#include <LiquidCrystal.h>
#include <ArduinoJson.h>
#include <Servo.h>
#include "SR04.h"

#define PIN_SERVO 3
#define TRIG_PIN 4 //pin per sensore ultrasuono
#define ECHO_PIN 5 //pin per sensore ultrasuono
#define PIN_ir_enter 13 //sbarra ingresso
#define PIN_ir_back 14 //sbarra uscita

char ssid[] = "AngeloP";            
char pass[] = "angelo123";      
int keyIndex = 0;   // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;      //connection status
WiFiServer server(80);            //server socket
WiFiClient wifi;
PubSubClient clientMQTT(wifi);
IPAddress serverMQTT(192,168,43,137);  //192,168,43,137

SR04 sr04 = SR04(ECHO_PIN,TRIG_PIN);
long a;       //valore letto dal sensore ad ultrasuoni
int id=0,i;
int flag1=0,flag2=0;
int slot=-1;
boolean enableGate=false, flag3=false, mod=false;
unsigned long StartTime,CurrentTime,ElapsedTime;
Servo servo;
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

/* il client si connette alla Wi-Fi
 * client si connette al broker mqtt sulla porta 1883. 
 * viene attivato un web_server su un determinato IP, sulla porta 80. */
void setup() {
  Serial.begin(9600);
  connect_WiFi();
  server.begin();
  printWifiStatus();

  pinMode(PIN_SERVO,OUTPUT);
  servo.attach(PIN_SERVO);
  clientMQTT.setServer(serverMQTT,1883);
  if(clientMQTT.connect("arduino1")){
    Serial.println("MQTT connected"); 
  }else{
    Serial.println("MQTT not connected");
    Serial.println(clientMQTT.state());
  } 

  lcd.begin(16, 2);
  lcd.setCursor(2, 0);
  lcd.print("Car Parking!");
  lcd.setCursor(0, 1);
  lcd.print("Inizializzazione");
}

void loop() {
  WiFiClient client = server.available();
  delay(150);  // delay tra: connessione del client al server arduino e disponibilità del messaggio
  if (client) {
    gestioneRichiesta(client);
  }

  clientMQTT.loop();
  if (!clientMQTT.connected()) {
    reconnect();
  }
  
 a=sr04.Distance();
 if(a<=5 && !flag3)
  passaggio_auto();

 /* Apre la sbarra a seguito del passaggio sotto il 1° sensore del gate (in entrata).
  * flag1 viene settato a 1 per ricordare che un auto sta entrando nel parcheggio.
  * ir sensor sono sempre 1, 0 quando trovano l'ostacolo.  */
 if(digitalRead (PIN_ir_enter) == LOW && flag1==0 && enableGate== true){
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
 if(digitalRead (PIN_ir_back) == LOW && flag2==0 && slot<=2 && slot>=0){
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
   enableGate=false;
   flag3=false;
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
   slot = slot+1;
   display();
   flag1=0, flag2=0;
   servoMotoreClose();
   id++;
   char out[128];
   
   StaticJsonDocument<256> message;
   message["id"] = id;
   message["mod"] = "esce";

   serializeJson(message, out);
   StartTime = millis();
   clientMQTT.publish("/topic/car",out);
   delay(4000); //per attendere elaborazione dell'AI
 }
}
//------------------------------------------------------------//---------------------------------------------------------//

/* Viene chiamata a seguito dell'evento di trigger. 
 * client effettua il publish per segnalare all'AI_Emulator che un'auto vorrebbe entrare nel parcheggio.
 * flag3=true blocca il lancio di più eventi di trigger quando l'auto rimane davanti al sensore ultrasuoni.
 * mod=false rappresenta la modalità 'ingresso di un'auto', mod=true rappresenta 'uscita di un'auto'*/
void passaggio_auto(){
  Serial.println("Auto all'ingresso.");
  id++;
  char out[128];

  StaticJsonDocument<256> message;
  message["id"] = id;
  message["mod"] = "entra";

  serializeJson(message, out);
  StartTime = millis();
  clientMQTT.publish("/topic/car",out);
  
  flag3=true;
  mod=false;
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

void connect_WiFi() {
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 5 seconds for connection:
    delay(5000);
    Serial.println("Connesso alla WiFi"); 
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

/* Legge il msg di richiesta, estrae i posti_disp ed effettua il controllo di stato.
 * se la richiesta è andata a buon fine invia lo status 200 OK, altrimenti segnala l'errore.
 * mod abilita/disabilita il controllo di stato (se in uscita non serve).*/
// arduino diventa il server che legge i messaggi inviati dal subscriber
void gestioneRichiesta(WiFiClient client) {
  char c;
  Serial.println("new client");  
  char res [10];
  int i=0;
  String response= String("");

  while (client.connected()) {  
    if(client.available()){
      c = client.read();        
      response +=c;
      //Serial.print(c);
      if(c == ']'){
       client.println("HTTP/1.1 201 Created");
       client.println();
       break;
      }
    }else{
      lcd.setCursor(0, 0);
      lcd.print("500 Internal ");
      lcd.setCursor(0, 1);
      lcd.print("Server Error!");
      client.println("HTTP/1.1 500 Internal Server Error");
      client.stop();
      flag3=false;
      delay(2000);
      display();
    }
  }
  client.stop();
  //Serial.print(response);
  String state= String("");
  for( i = 0; i< response.length();i++){
     if(response[i] == '[')
       for( int j = i ; response[j] != ']'; j++)
          state += response[j];
  }
  int posti_disponibili;
  if(id>9)
    posti_disponibili = atoi(&state[17]);
  else
    posti_disponibili = atoi(&state[16]); 

  // Alla prima interazione gli slot vengono settati da AI_Emulator
  if(id==1) slot=posti_disponibili; 
  display();
  
  CurrentTime = millis(); 
  ElapsedTime = CurrentTime - StartTime;  Serial.println(ElapsedTime);
  Serial.print("Posti disponibili=");   
  Serial.println(posti_disponibili);
  Serial.println("client disconnected");
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
      delay(3000); //per non far inviare tanti eventi quando il park è full
    }
  }else{
    Serial.print("ERRORE nel controllo di stato. I posti disponibili non corrispondono.");
    // invio del messaggio alla raspberry collegata al monitor della smart_city
  }
 }
}
