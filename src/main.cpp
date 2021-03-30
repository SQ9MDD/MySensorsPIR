#include <Arduino.h>

#define CHILD_ID_AI1        1             // mysensors first analog input child ID
#define CHILD_ID_BI1        2             // mysensors binary input child ID

#define BATTERY_SENSE_PIN   A0            // voltage mesure pin
#define BI1_INPUT            3             // contactron input

// radio 
#define MY_RADIO_NRF24
#define MY_DEBUG                          // Enable debug prints
#define MY_NODE_ID      43                 // <--- !!! SET NODE ADDRESS HERE !!!
#define MY_RF24_CHANNEL 80                // channel from 0 to 125. 76 is default

#include <MySensors.h>
             
unsigned long SLEEP_TIME = 300000;        // default 5m sleep time between reads and data send (seconds * 1000 milliseconds)
unsigned int weight = 5;                  // Max 64 min 1, unsigned int = 0-65535 max from adc 1023 *64 = 65472
unsigned int avg_mesure = 0;

MyMessage msgAI1(CHILD_ID_AI1, V_VOLTAGE);
MyMessage msgBI1(CHILD_ID_BI1, V_STATUS);

void presentation(){
    char etykieta[] = "       ";
    int addr = MY_NODE_ID;  
    sendSketchInfo("AS-200R-DG", "1.1");
    sprintf(etykieta,"R%02u.AI1",addr);  present(CHILD_ID_AI1, S_MULTIMETER, etykieta);  
    sprintf(etykieta,"R%02u.BI1",addr);  present(CHILD_ID_BI1, S_BINARY, etykieta);  
}

void setup(){
    analogReference(INTERNAL);
    pinMode(BATTERY_SENSE_PIN,INPUT);
    pinMode(BI1_INPUT,INPUT_PULLUP);   
    //sleep(60000);
    avg_mesure = analogRead(BATTERY_SENSE_PIN);
}

void loop(){
    int sensorValue = analogRead(BATTERY_SENSE_PIN);
    bool bi1_state = digitalRead(BI1_INPUT) == HIGH;
    
    // 1M8, 470K divider across battery and using internal ADC ref of 1.1V
    // ((1e6+470e3)/470e3)*1.1 = Vmax
    // Vmax/1023 = Volts per bit = 0.00502926

    // smoothing readings 
    avg_mesure = (avg_mesure*(weight-1) + sensorValue) / weight;
    
    // calculate data
    float batteryPcnt = 100.0/835.0 * float(avg_mesure);
    batteryPcnt = constrain(batteryPcnt,0,100);
    float batteryV  = avg_mesure * 0.00502926;
    sendBatteryLevel(batteryPcnt);
    send(msgAI1.set(batteryV, 2)); 
    send(msgBI1.set(bi1_state,1));
    sleep(digitalPinToInterrupt(BI1_INPUT), CHANGE, SLEEP_TIME);                                                      // go to sleep
}