#include <Arduino.h>

#include "configuration.h"
          
#ifndef __ASTRA_H_DIS__H__
  #define __ASTRA_H_DIS__H__
  
  #if ENABLE_DISPLAY
    #define DIS_MESSAGE "Opel Astra H"
    
    #define DIS_REFRESH_INTERVAL      5000  //ms
    #define DIS_MAX_CHARACTERS        14  
    #define DIS_RANDOM_TEXT_INTERVAL  10000 //ms
    #define DIS_TEXT_START_END_DELAY  1750  //ms
    #define DIS_TEXT_STEP_DELAY       300  //ms

    #if ENABLE_RANDOM_TEXTS
      const char *DIS_MESSAGES[]
      {    
        "Opel Astra H"
      };  
  
      unsigned long astraHLastRandomDISMillis = 0;
      unsigned long astraHLastRandomSeed = 0;
      char astraHLastRandomNumber = -1;

      int astraHCustomRandom(int min, int max, boolean notRepeated) //[min,max) || [min,max-1]
      { 
        unsigned char r;
        do
        {       
          astraHLastRandomSeed += analogRead(A2) + micros();       
          r = (astraHLastRandomSeed % (max-min)) + min;
        }
        while(notRepeated && r == astraHLastRandomNumber);
       
        astraHLastRandomNumber = r;        
        return r;        
      }
  
      const char *astraHGetRandomMessage()
      {
        return DIS_MESSAGES[astraHCustomRandom(0, sizeof(DIS_MESSAGES) / sizeof(const char*), true)];            
      }
    #endif
        
    const CAN_COMMAND CMD_DIS_REQUEST_SET_TEXT          {0x6C1, {0x10, 0x28, 0x40, 0x00, 0x4F, 0x03, 0x10, 0x11}, 8}; 
    const CAN_COMMAND CMD_DIS_RESPONSE_SET_TEXT         {0x2C1, {0x30, 0x00, 0x02}, 3};
        
    char *astraHDISText = NULL; //FULL TEXT
    bool astraHFreeDISText = false;
    unsigned char astraHDISTextPos = 0;
    unsigned char astraHDISTextLength = 0;    

    unsigned long astraHLastDISMarqueeMillis = 0;
    unsigned long astraHLastDISMillis = 0;

    void astraHSetDISText(CAN_CONTROLLER *can, const char *msg);
    void astraHSendToDIS(CAN_CONTROLLER *can, const char *inMsg);
    void astraHSendToDIS(CAN_CONTROLLER *can);
    void astraHRefreshDISText(CAN_CONTROLLER *can);
    bool astraHIsFirstChar();
    bool astraHIsLastMarqueeChar();
    bool astraHHasRemainingMarqueeText();
    bool astraHCheckMarquee();
    
    /*** DIS ***/
    void astraHSetDISText(CAN_CONTROLLER *can, String *msg)
    {      
      unsigned char len = msg->length();
      char *m = (char*)malloc(len+1);
      msg->toCharArray(m, len+1);
      
      astraHSetDISText(can, m);
    }

    void astraHSetDISText(CAN_CONTROLLER *can, char *msg)
    {      
      if(astraHFreeDISText)
        free(astraHDISText);
      
      astraHDISTextPos = 0;
      astraHDISText = msg;         
      astraHFreeDISText = true;
      astraHDISTextLength = strlen(msg);
    }     
    
    void astraHSetDISText(CAN_CONTROLLER *can, const char *msg)
    {  
      if(astraHFreeDISText)
        free(astraHDISText);
            
      astraHDISTextPos = 0;
      astraHDISText = (char*)msg;         
      astraHFreeDISText = false;
      astraHDISTextLength = strlen(msg);
    }     
    
    void astraHSendToDIS(CAN_CONTROLLER *can, const char *inMsg)
    { 
      unsigned char remainingLen = (astraHDISTextLength - astraHDISTextPos);
      unsigned char partLen = (remainingLen > DIS_MAX_CHARACTERS ? DIS_MAX_CHARACTERS : remainingLen);
      
      char *msg = (char*)malloc(DIS_MAX_CHARACTERS);
      memset(msg, 0x00, DIS_MAX_CHARACTERS);
      memcpy(msg, (inMsg + astraHDISTextPos), partLen);

      if(PRINT_DISPLAY_INFO)
      {
        Serial.print("Sending (DIS) : ");
        Serial.print(millis());
        Serial.print("ms - ");
        Serial.write(msg, partLen);
        Serial.println();
      }
      
      char buf[] = 
      {
        0x21, 0x00, msg[0], 0x00, msg[1], 0x00, msg[2], 0x00, 
        0x22, msg[3], 0x00, msg[4], 0x00, msg[5], 0x00, msg[6],
        0x23, 0x00, msg[7], 0x00, msg[8], 0x00, msg[9], 0x00,
        0x24, msg[10], 0x00, msg[11], 0x00, msg[12], 0x00, msg[13],
        0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
      };
                          
      CAN_COMMAND *cmd = (CAN_COMMAND*)malloc(sizeof(CAN_COMMAND));
      cmd->id = 0x6C1;
      cmd->len = 8;
    
      for(unsigned char i = 0; i < sizeof(buf) / 8; i++)
      {
          memcpy(&cmd->buf, &buf[i*8], 8);
          can->sendCmd(cmd);
      }
    
      free(cmd);
      free(msg);

      astraHLastDISMillis = astraHLastDISMarqueeMillis = millis();      
    }  
    
    void astraHSendToDIS(CAN_CONTROLLER *can)
    { 
      astraHSendToDIS(can, astraHDISText);
    } 
       
    void astraHRefreshDISText(CAN_CONTROLLER *can)
    {
      can->sendCmd(&CMD_DIS_REQUEST_SET_TEXT); //Request with max length (14 char)  
    }

    /*** MARQUEE ***/
    bool astraHIsFirstChar(){ return (astraHDISTextPos == 0); }
    bool astraHIsLastMarqueeChar(){ return (astraHDISTextPos == (astraHDISTextLength - DIS_MAX_CHARACTERS)); }
    bool astraHHasRemainingMarqueeText(){ return (astraHDISTextPos < (astraHDISTextLength - DIS_MAX_CHARACTERS)); }

    bool astraHCheckMarquee()
    {
      long current = millis();            
      unsigned int del = (astraHIsFirstChar() || astraHIsLastMarqueeChar() ? DIS_TEXT_START_END_DELAY : DIS_TEXT_STEP_DELAY);
      
      if((current - astraHLastDISMarqueeMillis) > del)
      {       
        astraHLastDISMarqueeMillis = current;
        
        if(astraHHasRemainingMarqueeText())
        {
          astraHDISTextPos++;      
          return true;
        }
        else
        {
          #if ENABLE_RANDOM_TEXTS
            if((current - astraHLastRandomDISMillis) >= DIS_RANDOM_TEXT_INTERVAL)
            {
              astraHLastRandomDISMillis = current;
              astraHSetDISText(&CAN_CONTROLLERS[CAN_A], astraHGetRandomMessage());    
              return true;        
            }      
            else 
            {
              astraHDISTextPos = 0;  
              return false;
            }
          #else      
            astraHDISTextPos = 0;
            return false;
          #endif
        }
      }     
      else return false;
    }   
  #endif
#endif 
