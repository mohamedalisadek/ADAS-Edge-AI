#ifndef REGISTERS_COMMON_H
#define REGISTERS_COMMON_H

#include "../SERVICES/STD_TYPES.h"

// Common Interrupt Control Register
#define INTCON      (*((volatile u8*)0x0B))
#define PIR1        (*((volatile u8*)0x0C))
#define OPTION_REG  (*((volatile u8*)0x81))

// Common bit definitions
#define GIE_BIT     7
#define PEIE_BIT    6
#define PEIE        6  // Alternative name

#endif
