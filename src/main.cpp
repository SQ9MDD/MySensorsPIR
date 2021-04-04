#include <Arduino.h>

#define CHILD_ID_AI1        1               // mysensors first analog input child ID
#define CHILD_ID_BI1        2               // mysensors binary input child ID
#define BATTERY_SENSE_PIN   A0              // voltage mesure pin
#define BI1_INPUT           3               // contactron input

// setup radio
#define MY_RADIO_NRF24                      // type of transceiver
#include <MySensors.h>
             
unsigned long SLEEP_TIME = 300000;          // default 5m sleep time between reads and data send (seconds * 1000 milliseconds)
float weight = 5.0;                         // scale, filter for smoothing measurements
float avg_mesure = 0.0;                     // variable for average value of the last measurements
float sensorValue = 0.0;                    //
MyMessage msgAI1(CHILD_ID_AI1, V_VOLTAGE);  //
MyMessage msgBI1(CHILD_ID_BI1, V_STATUS);   //

void presentation(){
    char etykieta[] = "        ";
    int addr = MY_NODE_ID;  
    sendSketchInfo("MySensorsPIR", "1.3");
    sprintf(etykieta,"R%02u.AI1",addr);  present(CHILD_ID_AI1, S_MULTIMETER, etykieta);  
    sprintf(etykieta,"R%02u.BI1",addr);  present(CHILD_ID_BI1, S_BINARY, etykieta);  
}

void setup(){
    analogReference(INTERNAL);
    pinMode(BATTERY_SENSE_PIN,INPUT);

    #ifdef PIR_SENSOR
        pinMode(BI1_INPUT,INPUT); 
    #elif CONTACTRON_SENSOR
        pinMode(BI1_INPUT,INPUT_PULLUP); 
    #endif  

    #ifdef INTERNAL_VOLT_MESURE
        avg_mesure = hwCPUVoltage() / 1000.0;
    #else
        avg_mesure = analogRead(BATTERY_SENSE_PIN) * 0.00502926;
    #endif
}

void loop(){
    bool bi1_state = digitalRead(BI1_INPUT);

    #ifdef INTERNAL_VOLT_MESURE
        sensorValue = hwCPUVoltage() / 1000.0;
    #else
        sensorValue = analogRead(BATTERY_SENSE_PIN) * 0.00502926;
    #endif

    // smoothing readings 
    avg_mesure = (avg_mesure*(weight-1) + sensorValue) / weight;
    
    // calculate battery percent
    // 3,4V - 0%
    // 4,2V - 100%
    float bat_low = 3.4;
    float batteryPcnt = 100.0/(4.2-bat_low) * float(avg_mesure-bat_low);
    batteryPcnt = constrain(batteryPcnt,0,100);

    // send data
    sendBatteryLevel(batteryPcnt);
    send(msgAI1.set(avg_mesure, 2)); 
    send(msgBI1.set(bi1_state,1));

    // go to sleep
    sleep(digitalPinToInterrupt(BI1_INPUT), CHANGE, SLEEP_TIME);                                                      
}