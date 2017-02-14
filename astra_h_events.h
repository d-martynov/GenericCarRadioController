#include "configuration.h"

#ifndef __ASTRA_H_EVENTS__H__
  #define __ASTRA_H_EVENTS__H__
  
  void afterCmdReceived(CAN_CONTROLLER *can, CAN_COMMAND *receivedCmd)
  {
    #if ENABLE_RADIO_SIMULATION        
      if(receivedCmd->id == 0x506)
        can->sendCmd(&CMD_EHU_REQ_BC_STATUS);
      else if(compareCanCmds(&CMD_STEERING_WHEEL_BC_WHEEL_RELEASE, receivedCmd))
        can->sendCmdSequence(&CMD_EHU_BC_PRESS, &CMD_EHU_BC_RELEASE, 1, true, CAN_COMMAND_INTERVAL);
      
      #if ENABLE_DISPLAY
        else if(compareCanCmds(&CMD_DIS_RESPONSE_SET_TEXT, receivedCmd))
          astraHSendToDIS(can);
      #endif     
  
      #if ENABLE_TEST_MODE 
        else if(receivedCmd->id == 0x454) //TEST MODE STATUS
          astraHTestModeStatus = (receivedCmd->buf[4] == 0x03);
      #endif
    #endif
  }
    
  void afterSetup()
  {
    #if ENABLE_CAN
      #if ENABLE_CAN_WHITELIST
        CAN_CONTROLLERS[CAN_A].addWhitelistID(0x206); //CIM - Steering Wheel
      #endif
  
      #if ENABLE_RADIO_SIMULATION
         #if ENABLE_CAN_FILTER
            CAN_CONTROLLERS[CAN_A].addWhitelistID(0x506); //BC
         #endif
         
         CAN_CONTROLLERS[CAN_A].sendCmd(&CMD_EHU_ON);

         #if ENABLE_CAN_PERIODICAL
            CAN_CONTROLLERS[CAN_A].addPeriodicalCmd(&PER_EHU_UNK1);   
            CAN_CONTROLLERS[CAN_A].addPeriodicalCmd(&PER_EHU_UNK2);                             
         #endif

         #if ENABLE_TEST_MODE && ENABLE_CAN_WHITELIST
            CAN_CONTROLLERS[CAN_A].addWhitelistID(0x454); //Test mode
         #endif
  
         #if ENABLE_DISPLAY
            #if ENABLE_CAN_WHITELIST              
              CAN_CONTROLLERS[CAN_A].addWhitelistID(0x2C1); //Display
            #endif
            
            #if ENABLE_RANDOM_TEXTS
              astraHSetDISText(&CAN_CONTROLLERS[CAN_A], astraHGetRandomMessage());
            #else
              astraHSetDISText(&CAN_CONTROLLERS[CAN_A], DIS_MESSAGE);
            #endif
         #endif 
      #endif

      #if ENABLE_CAN_WHITELIST
        CAN_CONTROLLERS[CAN_A].setupWhitelist();   
      #endif
    #endif
  }
  
  void afterLoop()
  {
    #if ENABLE_TEST_MODE   
      if(!astraHAutoEnabledTestMode && millis() > TEST_MODE_INITIAL_DELAY)
      {
        astraHAutoEnabledTestMode = true;
        astraHTryEnableTestMode();  
      }
      
      if(astraHEnablingTestMode)   
        astraHEnableTestModeStep();
    #endif
  
    #if ENABLE_DISPLAY            
      if(astraHCheckMarquee())//STEP
        astraHRefreshDISText(&CAN_CONTROLLERS[CAN_A]);        
      else
      {      
        long current = millis();        
        if((current - astraHLastDISMillis) >= DIS_REFRESH_INTERVAL)
        {
          astraHLastDISMillis = current;
          astraHRefreshDISText(&CAN_CONTROLLERS[CAN_A]);                  
        }
      }
    #endif
  }

  void handleSerialInput(String *in)
  {    
    #if ENABLE_SERIAL_CONTROL
        if(in->startsWith("settings"))                CAN_CONTROLLERS[CAN_A].sendCmdSequence(&CMD_EHU_SETTINGS_PRESS, &CMD_EHU_SETTINGS_RELEASE, 1, true, CAN_COMMAND_INTERVAL);
        else if(in->startsWith("bc"))                 CAN_CONTROLLERS[CAN_A].sendCmdSequence(&CMD_EHU_BC_PRESS, &CMD_EHU_BC_RELEASE, 1, true, CAN_COMMAND_INTERVAL);
        else if(in->startsWith("up"))                 CAN_CONTROLLERS[CAN_A].sendCmdSequence(&CMD_EHU_ARROW_UP_PRESS, &CMD_EHU_ARROW_UP_RELEASE, 1, true, CAN_COMMAND_INTERVAL);
        else if(in->startsWith("down"))               CAN_CONTROLLERS[CAN_A].sendCmdSequence(&CMD_EHU_ARROW_DOWN_PRESS, &CMD_EHU_ARROW_DOWN_RELEASE, 1, true, CAN_COMMAND_INTERVAL);
        else if(in->startsWith("left"))               CAN_CONTROLLERS[CAN_A].sendCmdSequence(&CMD_EHU_ARROW_LEFT_PRESS, &CMD_EHU_ARROW_LEFT_RELEASE, 1, true, CAN_COMMAND_INTERVAL);
        else if(in->startsWith("right"))              CAN_CONTROLLERS[CAN_A].sendCmdSequence(&CMD_EHU_ARROW_RIGHT_PRESS, &CMD_EHU_ARROW_RIGHT_RELEASE, 1, true, CAN_COMMAND_INTERVAL);

        #if ENABLE_RADIO_CONTROL
          else if(in->startsWith("RAD:"))
          {
              String cmd = in->substring(4);
              if(cmd.startsWith("vol_up"))              enableRadioCmd(&RADIO_CMD_VOLUME_UP, RADIO_COMMAND_INTERVAL);        
              else if(cmd.startsWith("vol_down"))       enableRadioCmd(&RADIO_CMD_VOLUME_DOWN, RADIO_COMMAND_INTERVAL);
              else if(cmd.startsWith("next_track"))     enableRadioCmd(&RADIO_CMD_NEXT_TRACK, RADIO_COMMAND_INTERVAL);
              else if(cmd.startsWith("prev_track"))     enableRadioCmd(&RADIO_CMD_PREV_TRACK, RADIO_COMMAND_INTERVAL);
              else if(cmd.startsWith("display"))        enableRadioCmd(&RADIO_CMD_DISPLAY, RADIO_COMMAND_INTERVAL);
              else if(cmd.startsWith("mute"))           enableRadioCmd(&RADIO_CMD_MUTE, RADIO_COMMAND_INTERVAL);
              else if(cmd.startsWith("source"))         enableRadioCmd(&RADIO_CMD_SOURCE, RADIO_COMMAND_INTERVAL);
              else if(cmd.startsWith("band_escape"))    enableRadioCmd(&RADIO_CMD_BAND_ESCAPE, RADIO_COMMAND_INTERVAL);
              else if(cmd.startsWith("folder_up"))      enableRadioCmd(&RADIO_CMD_FOLDER_UP, RADIO_COMMAND_INTERVAL);
              else if(cmd.startsWith("folder_down"))    enableRadioCmd(&RADIO_CMD_FOLDER_DOWN, RADIO_COMMAND_INTERVAL);
              else if(cmd.startsWith("voice_control"))  enableRadioCmd(&RADIO_CMD_VOICE_CONTROL, RADIO_COMMAND_INTERVAL);
          }    
        #endif
        
        #if ENABLE_DISPLAY
          else if(in->startsWith("DIS:"))
          {
              String msg = in->substring(4);
              long current = millis();
              
              #if ENABLE_RANDOM_TEXTS
                astraHLastRandomDISMillis = current;                
              #endif
              astraHLastDISMarqueeMillis = current;
              astraHSetDISText(&CAN_CONTROLLERS[CAN_A], &msg);
          }
        #endif
      #endif 
  }
#endif
