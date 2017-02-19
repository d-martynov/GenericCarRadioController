/********* CARS **********/
#define NONE_CAR 0
#define OPEL_ASTRA_H_CAR 1

/********* CONFIG **********/
#define CAR_MODEL               OPEL_ASTRA_H_CAR

#define ENABLE_CAN              1
#define ENABLE_CAN_WHITELIST    1
#define ENABLE_CAN_BLACKLIST    0
#define ENABLE_POTENTIOMETER    1
#define ENABLE_RADIO_CONTROL    1
#define ENABLE_SERIAL_CONTROL   1
#define ENABLE_CAN_PERIODICAL   1

//SPECIFIC CAR CONFIGURATION -> Opel Astra H
#define ENABLE_RADIO_SIMULATION 1
#define ENABLE_TEST_MODE        1
#define ENABLE_DISPLAY          1
#define ENABLE_RANDOM_TEXTS     1

//TIMING CONFIGURATION
#define RADIO_COMMAND_INTERVAL  50 //ms
#define CAN_COMMAND_INTERVAL    100 //ms
#define CAN_INIT_MAX_RETRIES    5
#define CAN_RESET_TIMEOUT       3000 //ms
#define CAN_QUEUE_MAX_SIZE      8 //items

//DEBUG CONFIGURATION
#ifndef __CONFIGURATION_H__
  #define __CONFIGURATION_H__
  
  bool PRINT_CAN_DATA           = false;
  bool PRINT_DISPLAY_INFO       = false;
  bool PRINT_POTENTIOMETER_INFO = false;  
  bool PRINT_STATUS_INFO        = true;
#endif
/********* END CONFIG **********/

void resetArduino();


