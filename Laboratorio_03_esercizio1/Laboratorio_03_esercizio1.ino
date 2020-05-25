/**
 *    IOT laboratorio HW-2
 *    esercizio 1
 *    todo: SMART Home Controller
 *          parte 1, ventola che si accende proporzionalmente
 *          con temperatura 25-30
 */

#include <math.h>


int tempMin = 25;
int tempMax = 30;
int tempSogliaLed = 20;

const int ledPin = 11;
const int tempPin = A1;
const long int R0 = 100000;
const int beta = 4275;
const int sleepTime = 5000;

const int fanPin = 10;
/*valore current speed da 0 a 255*/
float currentSpeed = 0;
float ledPower = 0;

const int pirPin = 7;
const unsigned long timeoutPir = 1800000;         /*timeout pir, circa 30 minuto 1800 secondi*/
volatile unsigned long checkTimePir = 0;
int flag = 0;

const int soundPin = 3;
const unsigned long soundInterval = 600000;      /*10 minuti in millis*/
const unsigned long timeoutSound = 2400000;      /*40 minuti timeout*/
volatile unsigned long checkTimeSound = 0;
int countSoundEvent = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(tempPin, INPUT);
  pinMode(fanPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  analogWrite(fanPin, currentSpeed);

  pinMode(pirPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(pirPin), checkPresence, CHANGE);

  pinMode(soundPin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(soundPin), checkSound, FALLING);
  
  while (!Serial);
  /* se non apri il monitor seriale, il programma non parte */
  Serial.println("Lab 2 Starting");
  
}

void loop() {
  // put your main code here, to run repeatedly:
  float  temp = checkTemp();
  if (temp < tempMax && temp > tempMin){
    currentSpeed = (temp-tempMin)*20/100*255;
    analogWrite(fanPin, currentSpeed);
    Serial.print("Temperatura :");
    Serial.print(temp);
    Serial.print(" velocita :");
    Serial.println(currentSpeed);
  }
  else{
    currentSpeed = 0;
    analogWrite(fanPin, currentSpeed);
  }
  
  if (temp < tempMin && temp > tempSogliaLed){
    ledPower = abs(temp-tempMin)*20/100*255;
    analogWrite(ledPin, ledPower);
  }
  else {
    ledPower = 0;
    analogWrite(ledPin, ledPower);
  }

  unsigned long currentMillis = millis();
  
  if (currentMillis - checkTimePir >= timeoutPir) {
    flag = 0;
  }
  
  if (currentMillis - checkTimeSound >= soundInterval ){
    if (countSoundEvent > 50){
      /*c'è una persona!*/
      flag = 1;
    }
    else if (currentMillis - checkTimeSound >= timeoutSound){
      flag = 0;
    }
    countSoundEvent = 0;
  }
  
  delay(sleepTime);
  
}

int checkTemp(){
  int vDigit = analogRead(tempPin);
  //calcolo il valore di R, successivamente
  //uso il datasheet per ricavare le formule di conversione e 
  //calcolo T, per poi convertire in Celsius
  float R = ((1023.0/vDigit)-1.0);
  R = R0*R;
  float loga = log(R/R0)/beta;
  float TK = 1.0/((loga)+(1.0/298.15));
  float TC = TK-273.15;
  return TC;
}

void checkPresence(){
  if (digitalRead(pirPin)==HIGH){
    flag = 1;
    checkTimePir = millis();
    Serial.print("movimento rilevato!");
  }
  else if(digitalRead(pirPin)==LOW){
  }
}

void checkSound(){
  if (countSoundEvent == 0){
    checkTimeSound = millis();
  }
  countSoundEvent++;
}


  
