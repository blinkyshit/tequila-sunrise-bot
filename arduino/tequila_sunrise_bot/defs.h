#ifndef defs_h
#define defs_h

#include <stdint.h>
//#include "WProgram.h"

struct motor_command
{
     int8_t   motor;
     uint16_t duration;
};

#endif
