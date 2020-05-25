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


void setup() {
  // put your setup code here, to run once:
  pinMode(tempPin, INPUT);
  pinMode(fanPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  analogWrite(fanPin, currentSpeed);
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


  
