#include "configuration.h"
#include "potentiometer.h"
#include "radio_command.h"
#include "can_command.h"
#include "can_controller.h"

#include "astra_h.h"
#include "default_car_configuration.h"

void setup()
{
    #if (PRINT_CAN_DATA | PRINT_POTENTIOMETER_INFO | PRINT_STATUS_INFO | ENABLE_SERIAL_CONTROL)
      Serial.begin(115200);
    #endif
    
    #if ENABLE_POTENTIOMETER
      setupPotentiometer(); 
    #endif  

    #if ENABLE_CAN          
      for(unsigned char i = 0; i < sizeof(CAN_CONTROLLERS) / sizeof(CAN_CONTROLLER); i++)
      {
        CAN_CONTROLLER *c = &CAN_CONTROLLERS[i];
        c->setupPins();
  
        if(!c->begin())
        {
          #if PRINT_STATUS_INFO 
            Serial.print("Controller "); Serial.print(c->name); Serial.println(" --> Init ERROR");
          #endif
          return;
        }
        else
        {
          #if PRINT_STATUS_INFO 
            Serial.print("Controller "); Serial.print(c->name); Serial.println(" --> Init OK");
          #endif      
        }
      }
    #endif  

    #if PRINT_STATUS_INFO
      Serial.println("Steering wheel controller --> Init OK");
    #endif
    
    afterSetup();
}

void resetArduino()
{  
  pinMode(3, OUTPUT);
  digitalWrite(3, LOW);
}

void onCmdReceived(CAN_CONTROLLER *c, CAN_COMMAND *receivedCmd)
{
  #if (ENABLE_RADIO_CONTROL && ENABLE_POTENTIOMETER)          
    for(unsigned char i = 0; i < sizeof(RADIO_CAN_COMMANDS) / sizeof(RADIO_CAN_COMMAND); i++)
    {
      if(compareCanCmds(RADIO_CAN_COMMANDS[i].canCmd, receivedCmd))
      {
        const RADIO_CAN_COMMAND *foundCmd = &RADIO_CAN_COMMANDS[i];
        if(foundCmd->radioCmd != NULL)
          enableRadioCmd(foundCmd->radioCmd, foundCmd->radioCmdInterval);
        else 
          disableRadioCmd();
            
        break;
      }          
    }           
  #endif  
}

void loop()
{
  #if ENABLE_CAN      
    for(unsigned char cIndex = 0; cIndex < sizeof(CAN_CONTROLLERS) / sizeof(CAN_CONTROLLER); cIndex++)
    {
      CAN_CONTROLLER *c = &CAN_CONTROLLERS[cIndex];
      CAN_COMMAND *receivedCmd;
      
      c->fillCmdQueueFromCANController();
       
      while((receivedCmd = c->pollNextCmd()) != NULL)        
      {
        onCmdReceived(c, receivedCmd);  
        afterCmdReceived(c, receivedCmd);  
        free(receivedCmd);
      }
            
      #if ENABLE_CAN_PERIODICAL
        c->checkPeriodicalCmds();
      #endif
    }
  #endif
  
  #if ENABLE_SERIAL_CONTROL   
    if(Serial.available() > 0)
    {
      String readed = Serial.readStringUntil('\n');
      handleSerialInput(&readed);               
    }      
  #endif

  afterLoop();
}


