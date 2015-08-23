#ifndef  TIMER_INT_H
#define  TIMER_INT_H
//================================================================================================
// timerInt.h
//------------------------------------------------------------------------------------------------
// Copyright 2010 Silicon Laboratories, Inc.
// http://www.silabs.com
//
// Header File Description:
//
// Target:
//    Any Silicon Labs C8051 MCU.
//
// IDE:
//    Silicon Laboratories IDE   version 3.3
//
// Tool Chains:
//    Keil
//       c51.exe     version 8.0.8
//       bl51.exe    version 6.0.5
//    SDCC
//       sdcc.exe    version 2.8.0
//       aslink.exe  version 1.75
//
// Project Name:
//    EZMacPro stack
//
// Release 2.0
//    - Official release the new EZMacPro stack with Frequency Hopper extension
//
// This software must be used in accordance with the End User License Agreement.
//
//================================================================================================
// compiler_defs.h must be included before this header file
//------------------------------------------------------------------------------------------------
// header check typedef
//------------------------------------------------------------------------------------------------
typedef struct PacketFiltersBytes
{
   U8 rcid;
   U8 rsid;
   U8 rdid;
   U8 packetlength;
} PacketFiltersBytes;

//------------------------------------------------------------------------------------------------
// Public Variables (API)
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
// Public function prototypes (API)
//------------------------------------------------------------------------------------------------
//=================================================================================================
//=================================================================================================
#endif //TIMER_INT_H

