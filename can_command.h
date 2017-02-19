#include "configuration.h"

#ifndef __CAN_COMMAND__H__
  #define __CAN_COMMAND__H__

  typedef struct
  {
    unsigned short int id;
    unsigned char buf[8];  
    unsigned char len;
    
  } CAN_COMMAND;

  bool compareCanCmds(const CAN_COMMAND *cmd1, const CAN_COMMAND *cmd2)
  {
      return (cmd1->id == cmd2->id && memcmp(cmd1->buf, cmd2->buf, sizeof(unsigned char) * cmd1->len/*buf*/) == 0);
  }
#endif


