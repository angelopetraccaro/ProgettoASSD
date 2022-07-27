#include <LiquidCrystal.h>
#include <Servo.h>
#include "SR04.h"

#define TRIG_PIN 4 //pin per sensore ultrasuono
#define ECHO_PIN 5 //pin per sensore ultrasuono
#define PIN_SERVO 6
#define PIN_ir_enter 13 //sbarra ingresso
#define PIN_ir_back 2   //sbarra uscita
#define PIN_ir_carOvest1 16
#define PIN_ir_carEst1 15
#define PIN_ir_carEst2 14
#define PIN_buzzer 3


SR04 sr04 = SR04(ECHO_PIN,TRIG_PIN);
long a;       //valore letto dal sensore ad ultrasuoni
int flag1=0,flag2=0;
int slot=3;
int S1=0,S2=0,S3=0;  //zero se posti liberi
int stato=0,i;

Servo servo;
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

void setup() {
  Serial.begin(9600);
  pinMode(PIN_buzzer,OUTPUT);
  pinMode(PIN_SERVO,OUTPUT);
  pinMode(PIN_ir_enter,INPUT);
  pinMode(PIN_ir_back,INPUT);
  pinMode(PIN_ir_carOvest1,INPUT);
  pinMode(PIN_ir_carEst1,INPUT);
  pinMode(PIN_ir_carEst1,INPUT);

  readSensor();
  stato = S1+S2+S3;
  slot = slot-stato;
  
  lcd.begin(16, 2);
  lcd.print("Car Parking!");
  lcd.setCursor(0, 1);
  lcd.print("Slot left: ");
  lcd.print(slot);
  
  servo.attach(PIN_SERVO);
}

void loop() {
 //lettura e stampa su monitor la posizione dell'auto da Proximity sensor (ultrasuono)
 a=sr04.Distance();
 //Serial.print(a);
 //Serial.println("cm");
 
 // Descrive il passaggio dell'auto sotto il 1 sensore in ingresso sbarra
 // ir sensor sono sempre 1, 0 quando trovano l'ostacolo
 // il flag ricorda se è passata una macchina sul sensore sbarra 1 e 2.
 if(digitalRead (PIN_ir_enter) == LOW && flag1==0){
  if(slot>0){
    flag1=1; // attraversato il 1 sensore sbarra o in ingresso o uscita
    if(flag2==0){
      controlloZone();  
      servoMotoreOpen(); 
      slot = slot-1;
      lcd.setCursor(0, 1);
      lcd.print("Slot left: ");
      lcd.print(slot);
      lcd.print("    ");
    }
  }else{
    lcd.setCursor (0,0);
    lcd.write("Sorry :(     ");
    lcd.setCursor(0, 1);
    lcd.print("Parking full");
    tone(PIN_buzzer,988,100);
    delay(100);
    tone(PIN_buzzer,1319,850);
    delay(2000);
    display();
  }
 }

 // Descrive il passaggio dell'auto sotto il 2 sensore sbarra
 if(digitalRead (PIN_ir_back) == LOW && flag2==0 && slot<=2){
  flag2=1;  // attraversato il 2 sensore sbarra o in ingresso o uscita
  
  // Descrive il passaggio dell'auto sotto il 1 sensore di uscita dal parcheggio
  if(flag1==0){
    servoMotoreOpen();
    slot = slot+1;
    display();
  }
 }

 //chiusura della sbarra dopo passaggio sui 2 sensori
 if(flag1==1 && digitalRead(PIN_ir_back)==LOW){
   //da notare che se il sensore legge/non legge si chiude la sbarra
   while(digitalRead(PIN_ir_back)==LOW){ Serial.println("aspetto che oltrepassi il sensore"); }
   flag1=0, flag2=0;
   servoMotoreClose();
 }
 //chiusura della sbarra dopo passaggio sui 2 sensori
 if(flag2==1 && digitalRead(PIN_ir_enter)==LOW){
   while(digitalRead(PIN_ir_enter)==LOW){ Serial.println("aspetto che oltrepassi il sensore"); }
   flag1=0, flag2=0;
   servoMotoreClose();
 }
 
 
parcheggio_assistito();
}// chiusura loop





void display(){
  lcd.setCursor(0, 0);
  lcd.print("Car Parking!  ");
  lcd.setCursor(0, 1);
  lcd.print("Slot left: ");
  lcd.print(slot);
  lcd.print("    ");
}

// funzione di lettura valori dei sensori di ogni parcheggio
void readSensor(){
  if(digitalRead(PIN_ir_carEst1)==0){S1=1;}
  if(digitalRead(PIN_ir_carEst2)==0){S2=1;}
  if(digitalRead(PIN_ir_carOvest1)==0){S3=1;}
}

void parcheggio_assistito(){
  a=sr04.Distance();
  if (0<=a && a<=2){
    tone(PIN_buzzer,988,100);
    Serial.println("vicino");
    }
  else if (2<a && a<=5){
    tone(PIN_buzzer,988,100);
    delay(500);
    tone(PIN_buzzer,988,100);
    Serial.println("intermedio");
    }
  else if (5<a && a<=10){
    tone(PIN_buzzer,988,100);
    delay(1000);
    tone(PIN_buzzer,988,100);
    Serial.println("lontano");
    } 
  else noTone;
}

void servoMotoreClose(){
  for (i=1; i<=100; i++){
      servo.write(i); 
      delay(15);
  }
}

void servoMotoreOpen(){
  for (i=100; i>=0; i--){
      servo.write(i); 
      delay(15);
  }
}

void controlloZone(){
  //controllo occupazione posti zona est parcheggio
 if(digitalRead(PIN_ir_carEst1)==0 && digitalRead(PIN_ir_carEst2)==0){
   lcd.setCursor(0, 0);
   lcd.print("Full East Zone");
   lcd.setCursor(0, 1);
   lcd.print("<- Turn left");
   delay (2000);
   lcd.clear();
   display();
 }
 
 //controllo occupazione posti zona ovest parcheggio
 if(digitalRead(PIN_ir_carOvest1)==0){
   lcd.setCursor(0, 0);
   lcd.print("Full West Zone");
   lcd.setCursor(0, 1);
   lcd.print("Turn right ->");
   delay (2000);
   lcd.clear();
   display();
 }
} 
