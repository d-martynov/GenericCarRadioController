#include "configuration.h"
#include "potentiometer.h"
#include "can_command.h"

#ifndef __RADIO_COMMAND__H__
  #define __RADIO_COMMAND__H__

  typedef struct
  {
    const float kOhms;
    const bool ringEnabled;
    
  } RADIO_COMMAND;

  typedef struct
  {
    const CAN_COMMAND *canCmd;
    const RADIO_COMMAND *radioCmd;
    unsigned short int radioCmdInterval;

  } RADIO_CAN_COMMAND;

  const RADIO_COMMAND RADIO_CMD_SOURCE          {1.2f, false}; //Source/Off
  const RADIO_COMMAND RADIO_CMD_MUTE            {3.5f, false}; //Mute/ATT
  const RADIO_COMMAND RADIO_CMD_DISPLAY         {5.75f, false}; //Display
  const RADIO_COMMAND RADIO_CMD_NEXT_TRACK      {8.0f, false}; //Next Track
  const RADIO_COMMAND RADIO_CMD_PREV_TRACK      {11.25f, false}; //Prev Track
  const RADIO_COMMAND RADIO_CMD_VOLUME_UP       {16.0f, false}; //Volume Up
  const RADIO_COMMAND RADIO_CMD_VOLUME_DOWN     {25.0f, false}; //Volume Down
  const RADIO_COMMAND RADIO_CMD_BAND_ESCAPE     {62.75f, false}; //Band/Escape
    
  const RADIO_COMMAND RADIO_CMD_FOLDER_UP       {8.0f, true}; //Folder Up
  const RADIO_COMMAND RADIO_CMD_FOLDER_DOWN     {11.25f, true}; //Folder Down 
  const RADIO_COMMAND RADIO_CMD_VOICE_CONTROL   {62.75f, true}; //Voice control

  void enableRadioCmd(const RADIO_COMMAND *radioCmd, const unsigned short int interval)
  {
      setResistor(radioCmd->kOhms, interval, radioCmd->ringEnabled);           
  }

  void disableRadioCmd() { disableResistor(); }

#endif


