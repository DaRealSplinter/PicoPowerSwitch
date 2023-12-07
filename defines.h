/****************************************************************************************************************************
  defines.h

  Ethernet_Generic is a library for the W5x00 Ethernet shields trying to merge the good features of
  previous Ethernet libraries

  Built by Khoi Hoang https://github.com/khoih-prog/Ethernet_Generic
 ***************************************************************************************************************************************/

#ifndef defines_h
#define defines_h

#define SerialDebug Serial1

#define DEBUG_ETHERNET_GENERIC_PORT SerialDebug

// Debug Level from 0 to 4
#define _ETG_LOGLEVEL_ 0

#undef ETHERNET_USE_SAMD
#undef ETHERNET_USE_NRF528XX
#undef ETHERNET_USE_SAM_DUE

#if defined(ETHERNET_USE_RPIPICO)
#undef ETHERNET_USE_RPIPICO
#endif
#define ETHERNET_USE_RPIPICO true


#if ETHERNET_USE_RPIPICO
// For RPI Pico using E. Philhower RP2040 core
#define USE_THIS_SS_PIN PIN_SPI0_SS  //17
#endif

#define SS_PIN_DEFAULT USE_THIS_SS_PIN

// For RPI Pico
#if (_ETG_LOGLEVEL_ > 3)
#warning Use RPI-Pico RP2040 architecture
#endif

#ifndef BOARD_NAME
#define BOARD_NAME BOARD_TYPE
#endif

#include <SPI.h>

///////////////////////////////////////////////////////////

// W5100 chips can have up to 4 sockets.  W5200 & W5500 can have up to 8 sockets.
// Use EthernetLarge feature, Larger buffers, but reduced number of simultaneous connections/sockets (MAX_SOCK_NUM == 2)
#define ETHERNET_LARGE_BUFFERS

#include "Ethernet_Generic.h"

#if defined(ETHERNET_LARGE_BUFFERS)
#define SHIELD_TYPE "W5x00 using Ethernet_Generic Library with Large Buffer"
#else
#define SHIELD_TYPE "W5x00 using Ethernet_Generic Library"
#endif

#endif  //defines_h
