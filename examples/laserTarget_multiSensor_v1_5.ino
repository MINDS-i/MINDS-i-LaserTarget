#include "laserTarget.h"

const int HYPER_LEDPIN = 5;
const int FIRE_LEDPIN = 4;
const int SIG_LEDPIN = 3;
const int potPin = A5;
const int resetPin = 0;
const int hyperPin = 1;
const int m_photoSensorPin[] = {A0, A1, A2, A3, A4};

uint32_t m_resetTime=2000;
uint32_t m_reset_hit_time;
bool     m_resetDown;
uint32_t m_hyperTime = 1000;
uint32_t m_hyper_hit_time;
bool     m_hyperDown;

uint32_t m_timer;
int      m_sensorValues[5];
bool     m_targetHit;

static laserTarget* m_plaserTarget=NULL;

void PrintDumpValues2(int darkValues[], int signalSample[], int deltaRises[], bool pulsesHigh[], bool pulseHigh, bool tval1/*signal*/, bool tval/*calibrate*/, bool hyper)
{
  Serial.print(m_timer);
  Serial.print(",");
  for(int i=0; i<NUMSENSORS; i++){
    Serial.print(darkValues[i]);
    Serial.print(",");
  }
  for(int i=0; i<NUMSENSORS; i++){
    Serial.print(signalSample[i]);
    Serial.print(",");
  }
  for(int i=0; i<NUMSENSORS; i++){
    Serial.print(deltaRises[i]);
    Serial.print(",");
  }
  for(int i=0; i<NUMSENSORS; i++){
    Serial.print(pulsesHigh[i]);
    Serial.print(",");
  }
  Serial.print(pulseHigh);
  Serial.print(",");
  Serial.print(tval1);
  Serial.print(",");
  Serial.print(tval);
  Serial.print(",");
  Serial.print(hyper);
  Serial.print("\n");
}

void readSensorValues()
{
  for(int i=0; i<5; i++){
    m_sensorValues[i]=analogRead(m_photoSensorPin[i]);
  }
}
void reset(){
  m_resetDown=false;
  m_targetHit=false;
  readSensorValues();
  digitalWrite(FIRE_LEDPIN, LOW);  
  digitalWrite(HYPER_LEDPIN, LOW);
}
bool checkReset(){
  int reset_val = digitalRead(resetPin);
  bool doReset=false;
  if(reset_val==LOW){
    if(m_resetDown){
      uint32_t time_down = m_timer - m_reset_hit_time;
      if(time_down>=m_resetTime){
        doReset=true;
      }
    }else{
      m_reset_hit_time=m_timer;
      m_resetDown=true;
    }
  }else{
    m_resetDown=false;
  }
  return doReset;
}
bool checkHyper(){
  bool goHyper=false;
  int hyper_val = digitalRead(hyperPin);
  if(hyper_val==HIGH){
    m_hyperDown=false;
    return false;
  }
  if(!m_hyperDown){
    m_hyperDown=true;
    m_hyper_hit_time = m_timer;
  }else{
    uint32_t time_down = m_timer - m_hyper_hit_time;
    if(time_down>=m_hyperTime){
      goHyper=true;
    }
  }
  return goHyper;
}
void setup() {
  m_plaserTarget = new laserTarget;
  m_timer=millis();
  Serial.begin(9600);
  pinMode(SIG_LEDPIN, OUTPUT);
  pinMode(FIRE_LEDPIN, OUTPUT);
  pinMode(HYPER_LEDPIN, OUTPUT);
  pinMode(resetPin, INPUT);
  pinMode(hyperPin, INPUT);
  reset();
  int potValue = analogRead(potPin);
  m_plaserTarget->Init(m_timer, m_sensorValues, potValue);
}

void loop() {
  m_timer = millis();
  readSensorValues();
  if(!m_targetHit){
    if(m_plaserTarget->Exec(m_timer, m_sensorValues)){
      if(m_plaserTarget->GetPulseHigh()){
        digitalWrite(SIG_LEDPIN, HIGH);
      }else{
        digitalWrite(SIG_LEDPIN, LOW);
      }
      if(checkHyper()){
        digitalWrite(HYPER_LEDPIN, HIGH);
        m_plaserTarget->ForceHyper();
      }
      /***********debug******************/
      if(m_plaserTarget->doPrint()){
        int darkValues[NUMSENSORS];
        int signalSample[NUMSENSORS];
        int deltaRises[NUMSENSORS];
        bool pulsesHigh[NUMSENSORS];
        bool pulseHigh;
        bool _signal; 
        bool calibrate;
        bool hyper;

        m_plaserTarget->GetDarkValues(darkValues);
        m_plaserTarget->GetSignalDragValues(signalSample);
        m_plaserTarget->GetDeltaRiseValues(deltaRises);
        m_plaserTarget->GetSignalHighs(pulsesHigh);
        pulseHigh = m_plaserTarget->GetPulseHigh();
        _signal = m_plaserTarget->GetSignalPulse();
        calibrate=m_plaserTarget->GetDoCalibrate();
        hyper = m_plaserTarget->GetHyper();
        PrintDumpValues2(darkValues, signalSample, deltaRises, pulsesHigh, pulseHigh, _signal, calibrate, hyper);
      }
      /**********************************/
    }else{
      /*laser target run has ended this should mean that target has been hit*/
      m_targetHit = m_plaserTarget->GetTargetHit();
      if(m_targetHit){
         digitalWrite(FIRE_LEDPIN, HIGH);  
      }
    }
  }else{
    if(checkReset()){
      /*do reset*/
      m_targetHit=false;
      reset();
      int potValue = analogRead(potPin);
      m_plaserTarget->reset(m_timer, m_sensorValues, potValue);
      /*debug*/
      Serial.print(m_plaserTarget->GetTargetHit_dt());
      /*******/
    }
  }
  delay(1);
}
