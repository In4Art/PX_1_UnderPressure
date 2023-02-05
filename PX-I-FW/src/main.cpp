#include <Arduino.h>

#ifdef ESP8266
 #include <ESP8266WiFi.h>
#else //ESP32
 #include <WiFi.h>
#endif

#include <ModbusIP_ESP8266.h>

#include "creds.h"
#include <ModeControl.h>
#include <WifiControl.h>

#include "Drv8833.h"

#define PX_NUM 1
#define PX_REG 100 + (PX_NUM * 10)
#define PX_STATE_REG 110 + PX_NUM //lowest PX_NUM = 1 !!!!

enum {
  PX_OK = 0
};


#define FULL_OPEN_SW D1
#define MIDDLE_SW D2
#define FULL_SQ_SW D0

#define IN1_PIN D5
#define IN2_PIN D6
#define ENA_PIN D7
#define FLT_PIN 10 //this pin isn't actually used, but the Drv8833 lib initializes it to INPUT_PULLUP. -> it is dangerously double used by modecontrol here!!!!

#define SQ_SPEED 1023

Drv8833 pxiMtr(IN1_PIN, IN2_PIN, ENA_PIN, FLT_PIN);

char ssid[] = SSID ;        // your network SSID (name)
char pass[] = PW;                    // your network password
WifiControl pxWifi(ssid, pass, PX_NUM);

ModbusIP pxModbus;

int8_t pxState = 0;
void setState(int8_t state);

int8_t demoState = 0;
void demoCallback(uint32_t dTime, px_mode_t mode);
ModeControl pxMC(3, &demoCallback, 30000, &pxWifi);

void setup() 
{

  delay(2000);
  Serial.begin(57600, SERIAL_8N1, SERIAL_TX_ONLY);
 
  pinMode(FULL_OPEN_SW, INPUT_PULLUP);
  pinMode(MIDDLE_SW, INPUT_PULLUP);
  pinMode(FULL_SQ_SW, INPUT_PULLDOWN_16);

  //time out used for waiting for a connection to occur
  //useful to change during testing to speed things up when dev-ing with no C&C
  pxWifi.setTimeOut(30000);

  //try to connect to C&C when demo switch is not switched on
  if(digitalRead(3) == HIGH){
    Serial.println("Connecting to C&C...");
    int8_t res = pxWifi.init();
    if(res == -1){
      Serial.println("No C&C found, starting up in demo mode!");
    }
  }

   //create the modbus server, and add a holding register (addHreg) and Ireg
  pxModbus.server(502);
  pxModbus.addHreg(PX_REG, 0);
  pxModbus.addIreg(PX_REG, 0);
  pxModbus.addHreg(PX_STATE_REG, PX_OK);

  pxMC.init();//initialize modeControl


  //seek start pos -> full open
  Serial.println("Seeking start post");
  if(digitalRead(FULL_OPEN_SW) == HIGH){
    pxiMtr.speed(1000, BWD);
    pxiMtr.start();
  }

  while(digitalRead(FULL_OPEN_SW) == HIGH){
    delay(1);
  }
  pxiMtr.stop();
  Serial.println("Start pos reached");

}


uint32_t sw_t = 0;
void loop() {
  

  if(millis() - sw_t > 1000){
        sw_t = millis();
    Serial.println("Switch states");
    Serial.print("FULL_OPEN_SW: ");
    Serial.println(digitalRead(FULL_OPEN_SW));
    Serial.print("MIDDLE_SW: ");
    Serial.println(digitalRead(MIDDLE_SW));
    Serial.print("FULL_SQ_SW: ");
    Serial.println(digitalRead(FULL_SQ_SW));
  }

  pxModbus.task();

  pxMC.run();

   //this should  copy the holding reg value to ireg
  if(pxModbus.Hreg(PX_REG) != pxModbus.Ireg(PX_REG)){
    pxModbus.Ireg(PX_REG, pxModbus.Hreg(PX_REG));
  }

  if(pxState != pxModbus.Ireg(PX_REG)){
    setState((int8_t)pxModbus.Ireg(PX_REG));
  }

  if(pxWifi.getStatus() != WL_CONNECTED && pxState == 0 && !pxiMtr.isRunning() ){
    pxWifi.reConn();
  }

  if(pxiMtr.isRunning()){
    switch(pxState){
      case 0:
        if(digitalRead(FULL_OPEN_SW) == LOW){
            pxiMtr.stop();
          }
          break;
      case 1:
        if(pxiMtr.getDirection() == BWD){
          if(digitalRead(MIDDLE_SW) == HIGH && digitalRead(FULL_SQ_SW) == LOW){
          pxiMtr.stop();
          }
        }else if(pxiMtr.getDirection() == FWD){
          if(digitalRead(MIDDLE_SW) == LOW && digitalRead(FULL_SQ_SW) == LOW){
            pxiMtr.stop();
          }
        }
        break;
      case 2:
        if(digitalRead(MIDDLE_SW) == LOW && digitalRead(FULL_SQ_SW) == HIGH){
          pxiMtr.stop();
        }
        break;
      default:
        break;

    }
    
  }
}


void setState(int8_t state)
{
  pxState = state;

  Serial.print("pxState : ");
  Serial.println(pxState);

  switch(pxState){
    case 0:
      Serial.println("setstate case 0");
      if(digitalRead(FULL_OPEN_SW) == HIGH){
        pxiMtr.speed(SQ_SPEED, BWD);
        pxiMtr.start();
        }
        break;
    case 1:
      Serial.println("setstate case 1");
      if(digitalRead(MIDDLE_SW) == HIGH && digitalRead(FULL_SQ_SW) == LOW){
        Serial.println("Middle pos, starting FWD");
        pxiMtr.speed(SQ_SPEED, FWD);
        pxiMtr.start();
      }else if( digitalRead(MIDDLE_SW) == LOW && digitalRead(FULL_SQ_SW) == HIGH){
        Serial.println("Middle pos, starting BWD");
        pxiMtr.speed(SQ_SPEED, BWD);
        pxiMtr.start();
        }
        break;
    case 2:
      Serial.println("setstate case 2");
      if(digitalRead(FULL_SQ_SW) == LOW){
        pxiMtr.speed(SQ_SPEED, FWD);
        pxiMtr.start();
      }
      break;
    default:
      break;

  }
  
}


void demoCallback(uint32_t dTime, px_mode_t mode)
{
  if(mode == PX_DEMO_MODE){
  if(demoState > 3){
        demoState = 0;
        
      }
      if(demoState > 2){
        pxModbus.Hreg(PX_REG, 4 - demoState);
        Serial.print("Checking PX status reg in demo mode: ");
        Serial.println(pxModbus.Hreg(PX_REG));
       
      }else{
        pxModbus.Hreg(PX_REG, demoState);
        Serial.print("Checking PX status reg in demo mode: ");
        Serial.println(pxModbus.Hreg(PX_REG));
        
      }
      demoState++;
  }else if(mode == PX_CC_MODE){
    demoState = 0;
    pxModbus.Hreg(PX_REG, demoState);
  }
}