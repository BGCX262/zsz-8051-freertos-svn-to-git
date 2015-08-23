#ifndef SI443X_CONST_B1_H
#define SI443X_CONST_B1_H
//================================================================================================
// si443x_const_b1.h
//================================================================================================
// Copyright 2010 Silicon Laboratories, Inc.
// http://www.silabs.com
//
// C File Description:
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
//#include "compiler_defs.h"
//---------------------------------------------------------------------------------------------------
// Definitions for RF setting
//---------------------------------------------------------------------------------------------------
#define NUMBER_OF_SAMPLE_SETTING		4
#define NUMBER_OF_PARAMETER			14



extern SEGMENT_VARIABLE (RfSettingsB1[NUMBER_OF_SAMPLE_SETTING][NUMBER_OF_PARAMETER], U8, code);


#endif //SI443X_CONST_B1_H
