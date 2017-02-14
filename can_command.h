#include "configuration.h"

#ifndef __CAN_COMMAND__H__
  #define __CAN_COMMAND__H__

  typedef struct
  {
    unsigned short int id;
    unsigned char buf[8];  
    unsigned char len;
    
  } CAN_COMMAND;

  char *canCmdToString(const char *prefix, const char *controllerName, const CAN_COMMAND *cmd)
  {
      unsigned short int len = 90 + (prefix != NULL ? strlen(prefix) : 0);
      char *serialBuf = (char*)malloc(len);
      memset(serialBuf, 0, len);
  
      if(prefix != NULL)
        sprintf(serialBuf, "%s", prefix);

      if(controllerName != NULL)
        sprintf(serialBuf, "%s (%s):", serialBuf, controllerName);
  
      sprintf(serialBuf, "%s %ldms",serialBuf,  millis());
      sprintf(serialBuf, "%s - 0x%03X -> ", serialBuf, cmd->id);
  
      for(int i = 0; i < cmd->len; i++)
        sprintf(serialBuf, "%s%02X,", serialBuf, cmd->buf[i]);
        
      return serialBuf;
  }

  bool compareCanCmds(const CAN_COMMAND *cmd1, const CAN_COMMAND *cmd2)
  {
      return (cmd1->id == cmd2->id && memcmp(cmd1->buf, cmd2->buf, sizeof(unsigned char) * cmd1->len/*buf*/) == 0);
  }
#endif


