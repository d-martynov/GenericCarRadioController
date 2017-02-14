#include <Arduino.h>
#include <SPI.h>
#include "configuration.h"

#ifndef __POTENTIOMETER__H__
  #define __POTENTIOMETER__H__

  #define SWITCH_ENABLED_LOGICAL_LEVEL LOW
  #define SWITCH_DISABLED_LOGICAL_LEVEL HIGH    

  #define TIP_CONNECTION_PIN 4
  #define RING_CONNECTION_PIN 5 
  
  //MCP4131  
  #define POT_CS_PIN      8       
  #define POT_ADDRESS     0x00  
  #define POT_STEPS       129
  #define POT_BASE_OHM    280 
  #define POT_STEP_OHM    760

  void setupPotentiometer();
  void disableResistor();
  void setResistor(float kOhms, unsigned short int interval, bool ring);
  unsigned short int getStepForPot(float kOhms);
  void digitalPotWrite(unsigned int value);
#endif


