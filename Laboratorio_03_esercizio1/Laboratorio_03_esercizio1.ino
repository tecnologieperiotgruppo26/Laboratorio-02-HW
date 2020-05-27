/**
 *    IOT laboratorio HW-2
 *    esercizio 1
 *    todo: SMART Home Controller
 *          parte 1, ventola che si accende proporzionalmente
 *          con temperatura 25-30
 */

#include <math.h>
#include <LiquidCrystal_PCF8574.h>
#include <string.h>

/**
 * set di temperature, 4 valori per presenza di persone 
 * e 4 per l'assenza
 */
float tempFanMinNoPeople = 25;
float tempFanMaxNoPeople = 30;
float tempLedMinNoPeople = 20;
float tempLedMaxNoPeople = 25;
float tempFanMinWithPeople = 25;
float tempFanMaxWithPeople = 35;
float tempLedMinWithPeople = 15;
float tempLedMaxWithPeople = 25;

int tempFanMin = 0;
int tempFanMax = 0;
int tempLedMin = 0;
int tempLedMax = 0;

float temp;

const int ledPin = 11;
const int tempPin = A1;
const long int R0 = 100000;
const int beta = 4275;
const int sleepTime = 10000;

const int fanPin = 10;
/* Valore corrente speed da 0 a 255 */
float currentSpeed = 0;
float ledPower = 0;

const int pirPin = 7;
const unsigned long timeoutPir = 1800000;         /* timeout pir, circa 30 minuto 1800 secondi */
volatile unsigned long checkTimePir = 0;
unsigned long currentMillis;

/**
 * QUESTO FLAG INDICA LA PRESENZA DI PERSONE, 0 ASSENZA E 1 PRESENZA
 */
int flag = 0;

const int soundPin = 3;
const unsigned long soundInterval = 600000;      /* 10 minuti in millis */
const unsigned long timeoutSound = 2400000;      /* 40 minuti timeout */
volatile unsigned long checkTimeSound = 0;
int countSoundEvent = 0;

int setupLCD = 0;
LiquidCrystal_PCF8574 lcd(0x27);



void setup() {
  /* Primo setup dei componenti */
  pinMode(tempPin, INPUT);
  pinMode(fanPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  analogWrite(fanPin, currentSpeed);

  pinMode(pirPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(pirPin), checkPresence, CHANGE);

  pinMode(soundPin, OUTPUT);

  lcd.begin(16, 2);
  lcd.setBacklight(255);
  lcd.home();
  lcd.clear();
  
  while (!Serial);
  /* E' necessario aprire il monitor altrimenti il programma non parte */
  Serial.begin(9600);
  Serial.println("Lab 2 Starting");
}

void loop() {
  if (flag==0){   
    tempFanMin = tempFanMinNoPeople;
    tempFanMax = tempFanMaxNoPeople;
    tempLedMin = tempLedMinNoPeople;
    tempLedMax = tempLedMaxNoPeople;
  }
  else if (flag==1){    
    tempFanMin = tempFanMinWithPeople;
    tempFanMax = tempFanMaxWithPeople;
    tempLedMin = tempLedMinWithPeople;
    tempLedMax = tempLedMaxWithPeople;
  }

  temp = checkTemp();
  /**
   * Controllo ventilatore
   */
  if (temp < tempFanMax && temp > tempFanMin){
    currentSpeed = (temp-tempFanMin)*20.0/100*255.0;
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
  /**
   * controllo led
   */
  if (temp < tempLedMax && temp > tempLedMin){
    ledPower = abs(temp-tempLedMin)*20.0/100*255.0;
    analogWrite(ledPin, ledPower);
  }
  else {
    ledPower = 0;
    analogWrite(ledPin, ledPower);
  }

  currentMillis = millis();
  
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


  /**
   * al posto della delay uitilizzo un ciclo while 
   * in cui attento il passare del tempo e checko la presenza di suoni
   * inoltre suddivido il tempo in multipli di 5 secondi per 
   * il cambio schermata sull'LCD
   */
  unsigned long delayMillis = millis();
  while (millis() - delayMillis <= sleepTime){
    checkSound();
    /**
     * magari il valore della funzione modulo è da rivedere
     * non so quanto vada veloce arduino, in caso è da trovare il giusto timing,
     * magari il risultato minore di 10 o roba del genere
     */
    if ((millis() - delayMillis)%5000 == 0){
      lookLCD();
    }
    if (Serial.available()){
      listenSerial();
    }
  //delay(sleepTime);
  }
}

float checkTemp(){
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


/**
 * Forse sarebbe meglio mettere come interrupt il sensore di
 * rumore, avendo a disposizione questi sensori si portrebbe 
 * diminuire la sensibilità del sensore di prossimità così che
 * ogni ciclo del loop principale possa catturare comunque le 
 * variazioni del sensore, mentre quello di rumore (più semplice
 * e basilare, e soprattutto solo digitale) si potrebbe impostare
 * sull'interrupt
 */
 
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
  if (digitalRead(soundPin) == LOW){
      if (countSoundEvent == 0){
        checkTimeSound = millis();
      }
      countSoundEvent++;
    }
}

void lookLCD(){
  /**
   * In base al setup corrente faccio la print della schermata 
   */
  if (setupLCD == 0){
    lcd.setCursor(0, 0);
    lcd.print("T=");
    lcd.print(String(temp));
    lcd.print(" Pres:");
    lcd.print(String(flag));
    lcd.setCursor(0, 1);
    lcd.print("AC:");
    lcd.print(String(currentSpeed/2.55));
    lcd.print(" HT:");
    lcd.print(String(ledPower/2.55));
    setupLCD = 1;
  }
  else if(setupLCD == 1){
    lcd.setCursor(0, 0);
    lcd.print("AC m:");
    lcd.print(String(tempFanMin));
    lcd.print(" M:");
    lcd.print(String(tempFanMax));
    lcd.setCursor(0, 1);
    lcd.print("HT m:");
    lcd.print(String(tempLedMin));
    lcd.print(" M:");
    lcd.print(String(tempLedMax));
    setupLCD = 0;
  }
}

/**
 * Dalla read devo leggere 8 valori, in sequenza saranno
  int tempFanMinNoPeople = 25;
  int tempFanMaxNoPeople = 30;
  int tempLedMinNoPeople = 20;
  int tempLedMaxNoPeople = 25;
  int tempFanMinWithPeople = 25;
  int tempFanMaxWithPeople = 35;
  int tempLedMinWithPeople = 15;
  int tempLedMaxWithPeople = 25;

  I VALORI IN INPUT DEVONO ESSERE DIVISI DA /
 */
void listenSerial(){
  char data[46];

  if (Serial.available() > 0) {
    int i, timeout = 60000; 
    char* data;
    Serial.setTimeout(timeout); /* Timeout riferito alla lettura dell'input settato a 1 minuto per permettere all'utente di scrivere */
    data = Serial.readString();
    sscanf(data, "%f/%f/%f/%f/%f/%f/%f/%f", &tempFanMinNoPeople, &tempFanMaxNoPeople, &tempLedMinNoPeople, &tempLedMaxNoPeople, &tempFanMinWithPeople, &tempFanMaxWithPeople, &tempLedMinWithPeople, &tempLedMaxWithPeople);
  }
} 

  
