#include "configuration.h"

#include "lib/mcp_can.cpp"
#include "lib/mcp_can.h"
#include "lib/mcp_can_dfs.h"
#include "lib/QList.h"
#include "lib/QList.cpp"

#ifndef __CAN_CONTROLLER__H__
  #define __CAN_CONTROLLER__H__

  typedef struct
  {    
    const CAN_COMMAND  *cmd;       
    unsigned short int interval;    
    unsigned long lastMillis;
   
  } CAN_COMMAND_PERIODICAL;

  class CAN_CONTROLLER
  {
    private:
      #if ENABLE_CAN_WHITELIST
        unsigned short int *whitelistIDs;
        unsigned char whitelistIDsLength;
      #endif
      #if ENABLE_CAN_BLACKLIST
        unsigned short int *blacklistIDs;
        unsigned char blacklistIDsLength;
      #endif
      #if ENABLE_CAN_PERIODICAL
        CAN_COMMAND_PERIODICAL **periodicalCmds;
        unsigned char periodicalCmdsLength; 
      #endif
        
      QList<CAN_COMMAND*> rxQueue;  

      CAN_COMMAND *readCmdFromCANController()
      {
          if(this->CAN->checkReceive() != CAN_MSGAVAIL)
            return NULL;
      
          CAN_COMMAND *cmd = (CAN_COMMAND*)malloc(sizeof(CAN_COMMAND));
          memset(cmd, 0, sizeof(CAN_COMMAND));
              
          CAN->readMsgBuf(&(cmd->len), cmd->buf);
          cmd->id = CAN->getCanId();
      
          #if ENABLE_CAN_BLACKLIST
            for(unsigned char i = 0; i < this->blacklistIDsLength; i++)
              if(this->blacklistIDs[i] == cmd->id)
              {
                  free(cmd);
                  return NULL;
              }
          #endif
          
          if(PRINT_CAN_DATA)
            printCanCmd("Received", this->name, cmd);          
          
          return cmd;
      }
      
      void printCanCmd(const char *prefix, const char *controllerName, CAN_COMMAND *cmd)
      {
        if(prefix != NULL)
          Serial.print(prefix);

        if(controllerName != NULL)
        {
          Serial.print(" (");
          Serial.print(controllerName);
          Serial.print(")");
        }

        Serial.print(" : ");
        Serial.print(millis());
        Serial.print(" - 0x");
        Serial.print(cmd->id, HEX);
        Serial.print(" -> ");

        for(int i = 0; i < cmd->len; i++)
        {
          Serial.print(cmd->buf[i], HEX);
          Serial.print(',');  
        }

        Serial.println();
        Serial.flush();
      }
    
    public:
      const char* name;
      
      unsigned char bitrate;
      unsigned char clockSet;
      unsigned char mode;      
  
      unsigned char csPin;
  
      MCP_CAN *CAN;

      CAN_CONTROLLER(const char *n, unsigned char bitR, unsigned char clk, unsigned char mod, unsigned char cs)
      {
        this->name = n;
        this->bitrate = bitR;
        this->clockSet = clk;
        this->mode = mod; 
        this->csPin = cs;
        this->CAN = new MCP_CAN(this->csPin);

        #if ENABLE_CAN_WHITELIST
          this->whitelistIDsLength = 0;
          this->whitelistIDs = NULL;
        #endif
        #if ENABLE_CAN_BLACKLIST
          this->blacklistIDsLength = 0;                  
          this->blacklistIDs = NULL;
        #endif        
        #if ENABLE_CAN_PERIODICAL
          this->periodicalCmds = NULL;  
          this->periodicalCmdsLength = 0;     
        #endif
      }

      void setupPins()
      {
        digitalWrite(this->csPin, SWITCH_DISABLED_LOGICAL_LEVEL); 
        pinMode(this->csPin, OUTPUT);      
      }

      bool begin()
      {
          unsigned char i = 1;
          for(; this->CAN->begin(this->bitrate, this->clockSet, this->mode) != CAN_OK; i++)
          {
              if(PRINT_STATUS_INFO){ Serial.print(i); Serial.println(" - CAN BUS init fail."); }
              
              delay(1000);
    
              if(i >= CAN_INIT_MAX_RETRIES)
              {
                  if(PRINT_STATUS_INFO) 
                  {
                    Serial.println("Can init limit reached. Performing an Arduino reset."); 
                    Serial.flush();                                 
                  }
                  
                  resetArduino();
              }
          }   

          return true;
      }

      /** SEND **/      
      bool sendCmd(const CAN_COMMAND *cmd, byte extraByte)
      {    
          if(cmd->len >= 8)
              return false;
              
          CAN_COMMAND *newCmd = (CAN_COMMAND*)malloc(sizeof(CAN_COMMAND));
          memcpy(newCmd, cmd, sizeof(CAN_COMMAND));
          newCmd->buf[newCmd->len] = extraByte;
          newCmd->len++;
          
          int stat = this->sendCmd(newCmd);
          free(newCmd);
                
          return stat;
      }

      bool sendCmd(const CAN_COMMAND *cmd)
      {   
          if(PRINT_CAN_DATA)
            printCanCmd("Sending", this->name, (CAN_COMMAND*)cmd);              
          else
              delay(1);
          
          return (CAN->sendMsgBuf(cmd->id, 0, cmd->len, (byte*)cmd->buf) == CAN_OK);
      }      
            
      void sendCmdSequence(const CAN_COMMAND *cmd1, const CAN_COMMAND *cmd2, unsigned short int sequenceCount, boolean addSequenceToCommand, unsigned short int delayBetweenSequences)
      {
          unsigned short int i = 0;
          if(cmd1 != NULL)
          {
            for(;i < sequenceCount; i++)
            {
              if(addSequenceToCommand)
                this->sendCmd(cmd1, (byte)i);
              else
                this->sendCmd(cmd1);
                
              delay(delayBetweenSequences);
            }
          }
      
          if(cmd2 != NULL)
          {
            if(addSequenceToCommand)
              this->sendCmd(cmd2, (byte)i-1);
            else
              this->sendCmd(cmd2);
          }
      }
      
      /** RECEIVE **/
      bool msgAvailableInCANController(){ return (this->CAN->checkReceive() == CAN_MSGAVAIL); };

      bool msgAvailableInQueue(){ return (rxQueue.length() > 0); }

      unsigned char fillCmdQueueFromCANController()
      {
        unsigned char i = 0;
        for(; msgAvailableInCANController() && rxQueue.length() < CAN_QUEUE_MAX_SIZE; i++)
        {
          CAN_COMMAND *cmd = readCmdFromCANController();
          if(cmd != NULL)
            rxQueue.push_back(cmd);
        }

        return i;
      }     
    
      CAN_COMMAND *pollNextCmd()
      {
          if(!msgAvailableInQueue())
            return NULL;

          CAN_COMMAND *cmd = rxQueue.front();
          rxQueue.pop_front();

          return cmd;
      }
      
      /** PERIODICAL CMDS **/
      #if ENABLE_CAN_PERIODICAL
        void addPeriodicalCmd(const CAN_COMMAND_PERIODICAL *per) { addPeriodicalCmd(per, false); }
        
        void addPeriodicalCmd(const CAN_COMMAND_PERIODICAL *per, bool sendNow)
        {
          if(this->periodicalCmds == NULL)        
            this->periodicalCmds = (CAN_COMMAND_PERIODICAL**)malloc(sizeof(CAN_COMMAND_PERIODICAL*));                 
          else        
            this->periodicalCmds = (CAN_COMMAND_PERIODICAL**)realloc(this->periodicalCmds, (this->periodicalCmdsLength + 1) * sizeof(CAN_COMMAND_PERIODICAL*));        
                 
          this->periodicalCmds[this->periodicalCmdsLength++] = (CAN_COMMAND_PERIODICAL*)per;
  
          if(sendNow)
            this->sendCmd(per->cmd);
        }
  
        void clearPeriodicalCmds()
        {                
          free(periodicalCmds);
          periodicalCmds = NULL;
          periodicalCmdsLength = 0;  
        }
  
        void checkPeriodicalCmds()
        {
          for(unsigned char i = 0; i < this->periodicalCmdsLength; i++)
          {
            unsigned long m = millis();
            if(m -  this->periodicalCmds[i]->lastMillis >  this->periodicalCmds[i]->interval)
            {
                sendCmd( this->periodicalCmds[i]->cmd);              
                this->periodicalCmds[i]->lastMillis = m;
            }
          }
        }
      #endif
      
      /** WHITE LIST **/ 
      #if ENABLE_CAN_WHITELIST
        void setupWhitelist()
        {           
            if(this->whitelistIDsLength <= 0)
              return;       
          
            this->CAN->init_Mask(0, 0, 0x7FF);
            this->CAN->init_Mask(1, 0, 0x7FF); 
        
            for(unsigned char i = 0; i < this->whitelistIDsLength && i < 6; i++) // there are 6 filter in mcp2515
              this->CAN->init_Filt(i, 0, this->whitelistIDs[i]);  
        }     
        
        void addWhitelistID(unsigned short int Id)
        {
          if(this->whitelistIDs == NULL)        
            this->whitelistIDs = (unsigned short int*)malloc(sizeof(unsigned short int));                 
          else        
            this->whitelistIDs = (unsigned short int*)realloc(this->whitelistIDs, (this->whitelistIDsLength + 1) * sizeof(unsigned short int));
                 
          this->whitelistIDs[this->whitelistIDsLength++] = Id;
        }
      #endif

      /** BLACK LIST **/
      #if ENABLE_CAN_BLACKLIST
        void addBlacklistID(unsigned short int Id)
        {
          if(this->blacklistIDs == NULL)        
            this->blacklistIDs = (unsigned short int*)malloc(sizeof(unsigned short int));                 
          else        
            this->blacklistIDs = (unsigned short int*)realloc(this->blacklistIDs, (this->blacklistIDsLength + 1) * sizeof(unsigned short int));
                 
          this->blacklistIDs[this->blacklistIDsLength++] = Id;
        }
  
        void clearBlacklistIDs()
        {                
          free(blacklistIDs);
          blacklistIDs = NULL;
          blacklistIDsLength = 0;  
        }
  
        unsigned short int getBlacklistIDsLength(){ return this->blacklistIDsLength; }
        unsigned short int getBlacklistID(unsigned short int index){ return this->blacklistIDs[index]; }
      #endif
  };

  #define CAN_A 0
  #define CAN_B 1
  CAN_CONTROLLER CAN_CONTROLLERS[]
  {
    CAN_CONTROLLER("A", CAN_95KBPS, MCP_CLOCKSET_8MHZ, MODE_NORMAL, 10),  //D10 ->  CS
    //CAN_CONTROLLER("B", CAN_95KBPS, MCP_CLOCKSET_8MHZ, MODE_NORMAL, 9),   //D9  ->  CS 
  };  
#endif

