#ifndef __ASTRA_H_TEST_MODE__H__
  #define __ASTRA_H_TEST_MODE__H__
  
  #if ENABLE_TEST_MODE  
    #define TEST_MODE_INITIAL_DELAY 5000
    
    bool astraHAutoEnabledTestMode = false;
    bool astraHTestModeStatus = false;    //false = Normal | true = Test
    bool astraHEnablingTestMode = false;
    unsigned char astraHTestModeCount = 0;
    
    void astraHTryEnableTestMode()
    {
      if(astraHTestModeStatus)
        return;
        
      astraHEnablingTestMode = true;
    }
    
    void astraHEnableTestModeStep()
    {          
      if(!astraHTestModeStatus)
      {         
        if(astraHTestModeCount < 0xFF) //255 cmds        
          CAN_CONTROLLERS[CAN_A].sendCmd(&CMD_EHU_SETTINGS_PRESS, astraHTestModeCount++);                
        else //ERROR
        {
          astraHTestModeCount = 0;
          astraHEnablingTestMode = false;      
        }
      }
      else //OK!
      {            
        CAN_CONTROLLERS[CAN_A].sendCmdSequence(&CMD_EHU_BC_PRESS, &CMD_EHU_BC_RELEASE, 1, true, CAN_COMMAND_INTERVAL);
        
        astraHTestModeCount = 0;
        astraHEnablingTestMode = false;    
      }
    } 
  #endif
#endif
