
#include "DRA818.h"


HardwareSerial Serial1(1);
HardwareSerial Serial2(2);

#define TX1_pin  27
#define RX1_pin  14

#define TX2_pin  4
#define RX2_pin  13

#define RxPTT 2
#define RxPD 5
#define RxAudioOn 39
#define RxAudioOut 26
#define RxAudioIn 33


#define TxPTT 15
#define TxPD 23
#define TxAudioOn 34
#define TxAudioOut 25
#define TxAudioIn 32

#define GPIO3 21
#define GPIO4 19
#define GPIO5 18
#define GPIO6 22
#define GPIO7 36
#define GPIO8 35
#define GPIO9 0

#define RXFreq 446.5000 
#define TXFreq 441.5000

const String morseArray[63] = {"-.-.--", "", "", "...-..-", "", ".-...", ".----.", "-.--.", "-.--.-", "", ".-.-.", "--..--", "-....-", ".-.-.-", "-..-.", "-----", ".----", "..---", "...--",
"....-", ".....", "-....", "--...", "---..", "----.", "---...", "-.-.-.", "", "-...-", "", "..--..", ".--.-.", ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", 
"-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..", "", "", "", "", "..--.-"}; // letters, numbers, symbols

const String Ident = "KD9KCK REPEATER";

const int MorseCodeDelay = 80;

DRA818 *SA818In;   
DRA818 *SA818Out;   



unsigned long TxStart = 0;
bool TXOn = false;
unsigned long LastIdent = 0;

void initPins(){
  pinMode(RxPTT,OUTPUT);
  digitalWrite(RxPTT,HIGH);
  pinMode(RxPD,OUTPUT);
  digitalWrite(RxPD,LOW);
  pinMode(RxAudioOn,INPUT);
  pinMode(RxAudioOut,INPUT);
  pinMode(RxAudioIn,INPUT);

  pinMode(TxPTT,OUTPUT);
  digitalWrite(TxPTT,HIGH);
  pinMode(TxPD,OUTPUT);
  digitalWrite(TxPD,LOW);
  pinMode(TxAudioOn,INPUT);
  pinMode(TxAudioOut,INPUT);
  pinMode(TxAudioIn,INPUT);
}

void initModules(){
  digitalWrite(RxPD,HIGH);
  digitalWrite(TxPD,HIGH);
  SA818In = DRA818::configure(Serial1, DRA818_VHF, RXFreq, RXFreq, 1, 8, 12, 12, DRA818_12K5, false, false, false, &Serial);
  SA818Out = DRA818::configure(Serial2, DRA818_VHF,TXFreq, TXFreq, 1, 8, 12, 12, DRA818_12K5, false, false, false, &Serial);
  
}

void registerISR(){
  attachInterrupt(RxAudioOn, RxAudioOnFALLING, FALLING);
  attachInterrupt(RxAudioOn, RxAudioOnRISING, RISING);
  
}

void playIdent(){

  for(int i = 0; i <= Ident.length(); i++)
  {
  
    if(Ident[i]>=33 && Ident[i]<=95) // check for targeted ASCII range
    {
        String morseToSend = morseArray[Ident[i]-33]; // use morse array to covert
        for(int x = 0; x <= morseToSend.length(); x++)
        {
          if(TXOn):
            return
          sendDashOrDot(morseToSend[x]);
        }
          delay(MorseCodeDelay*3); //between letters delay.
    }
    if (Ident[i]==32) // if space 
    {
      delay(MorseCodeDelay*4); //  delay for space (adding oneUnitDelay*4 delay to get oneUnitDelay*7 in total)
    }
  }


}

void sendDashOrDot(byte sendChar){  
  if (sendChar==0x2E) // if dot
  {
      ledcWriteTone(1,800);
      delay(MorseCodeDelay);
      ledcWrite(1,0);
      delay(MorseCodeDelay);
  }
  if (sendChar==0x2D) // if dash
  {
      ledcWriteTone(1,550);
      delay(MorseCodeDelay);
      ledcWrite(1,0);
      delay(MorseCodeDelay);
  }
}


void setup() {
 initPins();
 initModules();
 LastIdent = millis();

 ledcSetup(1,800,1); 
 ledcAttachPin(TxAudioOut,1);

 
 digitalWrite(TxPTT,LOW);
 playIdent();
 delay(10);
 digitalWrite(TxPTT,HIGH);
}

void loop() {
 noInterrupts();
 if(TxStart - millis() > 180000){
  TxOn = false;
  digitalWrite(TxPTT,HIGH);
 }
 interrupts();

if(LastIdent-millis() > 1800000){
 digitalWrite(TxPTT,LOW);
 playIdent();
 delay(10);
 if(!TxOn){
  digitalWrite(TxPTT,HIGH);
 }
 LastIdent = millis();
}
 
}

void IRAM_ATTR RxAudioOnRISING() {
    TXOn = false;
    digitalWrite(TxPTT,HIGH);
}

void IRAM_ATTR RxAudioOnFALLING() {
    TXOn = true;;
    TxStart = millis();
    digitalWrite(TxPTT,LOW);
}
