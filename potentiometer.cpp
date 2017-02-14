#include "potentiometer.h"

void setupPotentiometer()
{  
  digitalWrite(TIP_CONNECTION_PIN, SWITCH_DISABLED_LOGICAL_LEVEL); //DISABLED BY DEFAULT   
  digitalWrite(RING_CONNECTION_PIN, SWITCH_DISABLED_LOGICAL_LEVEL);
  digitalWrite(POT_CS_PIN, SWITCH_DISABLED_LOGICAL_LEVEL);
  
  pinMode(TIP_CONNECTION_PIN, OUTPUT);  
  pinMode(RING_CONNECTION_PIN, OUTPUT);
  pinMode(POT_CS_PIN, OUTPUT);

  #if !ENABLE_CAN
    SPI.begin();
  #endif

  delay(1000);
}

void setResistor(float kOhms, unsigned short int interval, bool ring)
{
    int s = getStepForPot(kOhms);
      
    #if PRINT_POTENTIOMETER_INFO
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
    #endif
	
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

void disableResistor()
{    
    digitalWrite(TIP_CONNECTION_PIN, SWITCH_DISABLED_LOGICAL_LEVEL);   
    digitalWrite(RING_CONNECTION_PIN, SWITCH_DISABLED_LOGICAL_LEVEL);
    digitalPotWrite(0); //Min resistor
    
    #if PRINT_POTENTIOMETER_INFO
      Serial.println("Resistor disabled");
    #endif
}

unsigned short int getStepForPot(float kOhms)
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

void digitalPotWrite(unsigned int value)
{
  digitalWrite(POT_CS_PIN, SWITCH_ENABLED_LOGICAL_LEVEL);
  SPI.transfer(POT_ADDRESS);
  SPI.transfer(value);
  digitalWrite(POT_CS_PIN, SWITCH_DISABLED_LOGICAL_LEVEL);
}


