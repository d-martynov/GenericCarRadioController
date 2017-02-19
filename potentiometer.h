#include <SPI.h>

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
  #define POT_BASE_OHM    620 //825
  #define POT_STEP_OHM    750 
  

  void digitalPotWrite(unsigned int value)
  {
    digitalWrite(POT_CS_PIN, SWITCH_ENABLED_LOGICAL_LEVEL);
    SPI.transfer(POT_ADDRESS);
    SPI.transfer(value);
    digitalWrite(POT_CS_PIN, SWITCH_DISABLED_LOGICAL_LEVEL);
  }

  unsigned char getStepForPot(float kOhms)
  {
      unsigned int ohms = (kOhms * 1000);
      unsigned short int s = ((float)(ohms - POT_BASE_OHM) / POT_STEP_OHM) + 0.5f;
    
      if(s < 0)
        return 0;
      else if(s > POT_STEPS-1)
        return POT_STEPS-1;
      else
        return s;
  }

  void disableResistor()
  {    
      digitalWrite(TIP_CONNECTION_PIN, SWITCH_DISABLED_LOGICAL_LEVEL);   
      digitalWrite(RING_CONNECTION_PIN, SWITCH_DISABLED_LOGICAL_LEVEL);
      digitalPotWrite(0); //Min resistor
      
      if(PRINT_POTENTIOMETER_INFO)
        Serial.println("Resistor disabled");    
  }
  
  void setupPotentiometer()
  {  
    digitalWrite(TIP_CONNECTION_PIN, SWITCH_DISABLED_LOGICAL_LEVEL); //DISABLED BY DEFAULT   
    digitalWrite(RING_CONNECTION_PIN, SWITCH_DISABLED_LOGICAL_LEVEL);
    
    pinMode(TIP_CONNECTION_PIN, OUTPUT);  
    pinMode(RING_CONNECTION_PIN, OUTPUT);
    pinMode(POT_CS_PIN, OUTPUT);
  
    SPI.begin();
    digitalPotWrite(0); //Min resistor
    SPI.end();
  }

  void setResistor(float kOhms, unsigned short int interval, bool ring)
  {
      int s = getStepForPot(kOhms);
        
      if(PRINT_POTENTIOMETER_INFO)
      {
        float realPotResistor = ((float)s * POT_STEP_OHM + POT_BASE_OHM) / 1000;
      
        Serial.print("Resistor ");
        Serial.print(kOhms);
        Serial.print(" with pot step: ");
        Serial.print(s);
        Serial.print(" (real should be around ");
        Serial.print(realPotResistor);
        Serial.print(" kOhm -> ");
        Serial.print(realPotResistor - kOhms);
        Serial.print(" deviation)");
        
        if(interval > 0)
        {
          Serial.print(" for ");
          Serial.print(interval);
          Serial.print(" ms.");       
        }
  
        if(ring)Serial.println(" (with RING)");
        else Serial.println();
      }
    
      digitalPotWrite(s);
      
      if(ring)
        digitalWrite(RING_CONNECTION_PIN, SWITCH_ENABLED_LOGICAL_LEVEL);
        
      digitalWrite(TIP_CONNECTION_PIN, SWITCH_ENABLED_LOGICAL_LEVEL);      
      
      if(interval > 0)
      {
          delay(interval);
          disableResistor();
      }
  }  
  
  
  
  
  
 

#endif


