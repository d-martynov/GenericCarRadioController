#if(CAR_MODEL==NONE_CAR)
  #ifndef __DEFAULT_CAR_CONFIGURATION__ 
    #define __DEFAULT_CAR_CONFIGURATION__
    #define CAR_CONFIGURATION_SET 1    
      const RADIO_CAN_COMMAND RADIO_CAN_COMMANDS[]{};

      void handleSerialInput(String *in){}
      void afterSetup(){}
      void afterCmdReceived(CAN_CONTROLLER *can, CAN_COMMAND *receivedCmd){}
      void afterLoop(){}
  #endif
#endif


