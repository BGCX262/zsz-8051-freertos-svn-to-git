#ifndef EZMACPRO_CONST_H
#define EZMACPRO_CONST_H
//================================================================================================
// EZMacPro_const.h
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

//definition for the parameters table
#define START_FREQUENCY_1					0
#define START_FREQUENCY_2					1
#define START_FREQUENCY_3					2
#define STEP_FREQUENCY						3
#define MAX_CHANNEL_NUMBER					4
#define PREAMBLE_IF_ONE_CHANNEL			5
#define PREAMBLE_IF_TWO_CHANNEL			6
#define PREAMBLE_IF_THREE_CHANNEL		7
#define PREAMBLE_IF_FOUR_CHANNEL			8

#ifdef FOUR_CHANNEL_IS_USED
	#define SEARCH_TIME						9
#endif
#ifdef MORE_CHANNEL_IS_USED
	#define SEARCH_TIME						4
#endif

#define CHANNEL_NUMBERS					5
#define PREAMBLE_LENGTH					6
#define PREAMBLE_LENGTH_REG_VALUE	7


#define PREAMBLE_DETECTION_THRESHOLD	5 //in nibble

//Channel parameter table
#ifdef FOUR_CHANNEL_IS_USED
	extern SEGMENT_VARIABLE (Parameters[4][10], U8, code);
#endif
#ifdef MORE_CHANNEL_IS_USED
	extern SEGMENT_VARIABLE (Parameters[4][8], U8, code);	
#endif


extern SEGMENT_VARIABLE (EZMacProByteTime[4], U16, code);
extern SEGMENT_VARIABLE (FrequencyTable[50],U8, code);


#endif //EZMACPRO_CONST_H
