#include <WiFi101.h>   //per arduino mkr1000
#include <PubSubClient.h>

const char WIFI_SSID[] = "AngeloP";  //nota: la conness deve essere 2.4GHz
const char WIFI_PASS[] = "angelo123";
WiFiClient wifi;
int status = WL_IDLE_STATUS;
IPAddress server(192,168,43,137);
PubSubClient client(wifi);

void setup() {
  pinMode(6,OUTPUT);
  Serial.begin(9600);
  while(!Serial){
   ;  
  }
  Serial.println("ok...connessione...");
  while(WiFi.status() != WL_CONNECTED){
    status=WiFi.begin(WIFI_SSID, WIFI_PASS);
    delay(500);
  }
  Serial.println("Connected to WiFi\n");

  client.setServer(server,1883);

  if(client.connect("arduino1")){
    Serial.println("MQTT connected");
      
    client.subscribe("/nomeTopic");   
  
  }else{
    Serial.println("MQTT not connected");
    Serial.println(client.state());
  }

  client.setCallback(callback);  //qui il client è un subscriber e la funz viene chiamata a seguito di pub
}

void loop() {
  client.loop();  //recupera messaggi del client che sono arrivati dal server
  client.publish("temperatura","8");  
  client.publish("/nomeTopic","on",1);   
}

void callback(char* topic, byte* payload, unsigned int length){
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  
  String msg; //per trasformare il payload byte per byte
  Serial.println("Message payload:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    msg += (char)payload[i];  
  }
 
  Serial.println();
  Serial.println("-----------------------");
  
  if(strcmp(topic,"/nomeTopic")==0){
    if(msg=="on"){
      digitalWrite(6,HIGH);
      delay(1000);
      digitalWrite(6,LOW);
      delay(1000);
      Serial.println("on");
    } else if (msg=="off"){
      digitalWrite(6,LOW);
      Serial.println("off");
    }
  }
  
}
