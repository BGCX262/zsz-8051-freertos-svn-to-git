//================================================================================================
// EZMacPro_const.c
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
//---------------------------------------------------------------------------------------------------
// Includes
//
// These includes must be in a specific order. Dependencies listed in comments.
//---------------------------------------------------------------------------------------------------
#include "compiler_defs.h"
#include "hardware_defs.h"                   // requires compiler_defs.h
#include "EZMacPro_defs.h"
#include "EZMacPro.h"                        // requires EZMacPro_defs.h
#include "EZMacPro_const.h"						// requires EZMacPro.h	
//-----------------------------------------------------------------------------------------------
//
// EZMacPro Code constants
//
//-----------------------------------------------------------------------------------------------

#ifdef FOUR_CHANNEL_IS_USED

#ifdef ANTENNA_DIVERSITY_ENABLED
	#ifdef FREQUENCY_BAND_434
   const SEGMENT_VARIABLE (Parameters[4][10], U8, code) =
   {
		//FR_S1,   FR_S2,  FR_S3, 	FR_ST,   MAX_CH,  PR1,  PR2,  PR3,  PR4,	ST
		{0x53,    0x53,    0x40, 	33,      4,       8,    10,   13,   16,	3}, 		//DR: 2.4kbps, DEV: +-38.4kHz, BBBW: 85.1kHz
		{0x53,    0x53,    0x40, 	33,      4,       8,    10,   13,   16,	3},		//DR: 9.6kbps, DEV: +-38.4kHz, BBBW: 95.3kHz
		{0x53,    0x5A,    0x40, 	36,      2,       8,    14,   19,   24,	5},		//DR: 50kbps, DEV: +-25kHz, BBBW: 112.1kHz
		{0x53,    0x61,    0xC0, 	0,       1,       8,    18,   25,   32,	7}			//DR: 128kbps, DEV: +-64kHz, BBBW: 269.3kHz
	}; 
	#elif defined FREQUENCY_BAND_868
   const SEGMENT_VARIABLE (Parameters[4][10], U8, code) =
   {
   	//FR_S1	FR_S2,	FR_S3,	FR_ST,  	MAX_CH,  PR1,	PR2,  PR3,  PR4, 	ST
      {0x73, 	0x2C, 	0x60,  	45,     	14,      8,    10,   13,   16,	3},		//DR: 2.4kbps, DEV: +-38.4kHz, BBBW: 95.3kHz
      {0x73, 	0x2C, 	0x60,   	45,     	14,      8,    10,   13,   16,	3},		//DR: 9.6kbps, DEV: +-38.4kHz, BBBW: 102.2kHz
      {0x73, 	0x2E, 	0xE0,   	78,     	8,       8,    14,   19,   24,	5},		//DR: 50kbps, DEV: +-25kHz, BBBW: 112.8kHz
      {0x73, 	0x32, 	0x00,   	100,    	6,       8,    18,   25,   32,	7}			//DR: 128kbps, DEV: +-64kHz, BBBW: 269.3kHz
	};

	#elif defined FREQUENCY_BAND_915
   const SEGMENT_VARIABLE (Parameters[4][10], U8, code) =
   {
	   //FR_S1,  FR_S2,	FR_S3,	FR_ST,	MAX_CH,  PR1,  PR2,  PR3,	PR4,	ST	
      {0x75,    0x0A,   0x80,  	48,   	60,      8,    10,   13,  	16,	3},		//DR: 2.4kbps, DEV: +-38.4kHz, BBBW: 95.3kHz
      {0x75,    0x0A,   0x80,   	48,   	60,      8,    10,   13,  	16,	3},		//DR: 9.6kbps, DEV: +-38.4kHz, BBBW: 102.2kHz
      {0x75,    0x0A,   0x80,   	78,   	37,      8,    14,   19,  	24,	5},		//DR: 50kbps, DEV: +-25kHz, BBBW: 112.8kHz
      {0x75,    0x0A,   0x80,   	100,  	29,      8,    18,   25,  	32,	7}			//DR: 128kbps, DEV: +-64kHz, BBBW: 269.3kHz
   };
	#endif

#else
	#ifdef FREQUENCY_BAND_434
   const SEGMENT_VARIABLE (Parameters[4][10], U8, code) =
   {
		//FR_S1,   FR_S2,  FR_S3, 	FR_ST,   MAX_CH,  PR1,  PR2,  PR3,  PR4,	ST
		{0x53,    0x53,    0x40, 	33,      4,       4,    10,   13,   16,	3}, 		//DR: 2.4kbps, DEV: +-38.4kHz, BBBW: 85.1kHz
		{0x53,    0x53,    0x40, 	33,      4,       4,    10,   13,   16,	3},		//DR: 9.6kbps, DEV: +-38.4kHz, BBBW: 95.3kHz
		{0x53,    0x5A,    0x40, 	36,      2,       4,    14,   19,   24,	5},		//DR: 50kbps, DEV: +-25kHz, BBBW: 112.1kHz
		{0x53,    0x61,    0xC0, 	0,       1,       4,    18,   25,   32,	7}			//DR: 128kbps, DEV: +-64kHz, BBBW: 269.3kHz
	}; 
	#elif defined FREQUENCY_BAND_868
   const SEGMENT_VARIABLE (Parameters[4][10], U8, code) =
   {
   	//FR_S1	FR_S2,	FR_S3,	FR_ST,  	MAX_CH,  PR1,	PR2,  PR3,  PR4, 	ST
      {0x73, 	0x2C, 	0x60,  	45,     	14,      4,    10,   13,   16,	3},		//DR: 2.4kbps, DEV: +-38.4kHz, BBBW: 95.3kHz
      {0x73, 	0x2C, 	0x60,   	45,     	14,      4,    10,   13,   16,	3},		//DR: 9.6kbps, DEV: +-38.4kHz, BBBW: 102.2kHz
      {0x73, 	0x2E, 	0xE0,   	78,     	8,       4,    14,   19,   24,	5},		//DR: 50kbps, DEV: +-25kHz, BBBW: 112.8kHz
      {0x73, 	0x32, 	0x00,   	100,    	6,       4,    18,   25,   32,	7}			//DR: 128kbps, DEV: +-64kHz, BBBW: 269.3kHz
	};

	#elif defined FREQUENCY_BAND_915
   const SEGMENT_VARIABLE (Parameters[4][10], U8, code) =
   {
	   //FR_S1,  FR_S2,	FR_S3,	FR_ST,	MAX_CH,  PR1,  PR2,  PR3,	PR4,	ST	
      {0x75,    0x0A,   0x80,  	48,   	60,      4,    10,   13,  	16,	3},		//DR: 2.4kbps, DEV: +-38.4kHz, BBBW: 95.3kHz
      {0x75,    0x0A,   0x80,   	48,   	60,      4,    10,   13,  	16,	3},		//DR: 9.6kbps, DEV: +-38.4kHz, BBBW: 102.2kHz
      {0x75,    0x0A,   0x80,   	78,   	37,      4,    14,   19,  	24,	5},		//DR: 50kbps, DEV: +-25kHz, BBBW: 112.8kHz
      {0x75,    0x0A,   0x80,   	100,  	29,      4,    18,   25,  	32,	7}			//DR: 128kbps, DEV: +-64kHz, BBBW: 269.3kHz
   };
	#endif
#endif//ANTENNA_DIVERSITY_ENABLED
#endif//FOUR_CHANNEL_IS_USED

#ifdef MORE_CHANNEL_IS_USED
	#ifdef FREQUENCY_BAND_434
   const SEGMENT_VARIABLE (Parameters[4][8], U8, code) =
   {
		//FR_S1,  FR_S2,	FR_S3,	FR_ST,  ST,	CHNBR,	PREAL,	PREARV
		{0x53,    0x53,   0x40, 	33,     3,	50,		160,		0x40}, 		//DR: 2.4kbps, DEV: +-38.4kHz, BBBW: 85.1kHz
		{0x53,    0x53,   0x40, 	33,     3,	50,		160,		0x40},		//DR: 9.6kbps, DEV: +-38.4kHz, BBBW: 95.3kHz
		{0x53,    0x5A,   0x40, 	36,     5,	25,		139,		0x16},		//DR: 50kbps, DEV: +-25kHz, BBBW: 112.1kHz
		{0x53,    0x61,   0xC0, 	0,      7,	25,		193,		0x82},		//DR: 128kbps, DEV: +-64kHz, BBBW: 269.3kHz
	}; 
	#elif defined FREQUENCY_BAND_868
   const SEGMENT_VARIABLE (Parameters[4][8], U8, code) =
   {
   	//FR_S1 	FR_S2,	FR_S3,  FR_ST,  ST,	CHNBR,	PREAL,	PREARV
      {0x73, 	0x2C, 	0x60,   45,     3,	50,		160,		0x40},		//DR: 2.4kbps, DEV: +-38.4kHz, BBBW: 95.3kHz
      {0x73, 	0x2C, 	0x60,   45,     3,	50,		160,		0x40},		//DR: 9.6kbps, DEV: +-38.4kHz, BBBW: 102.2kHz
      {0x73, 	0x2E, 	0xE0,   78,     5,	25,		139,		0x16},		//DR: 50kbps, DEV: +-25kHz, BBBW: 112.8kHz
      {0x73, 	0x32, 	0x00,   100,    7,	25,		193,		0x82}			//DR: 128kbps, DEV: +-64kHz, BBBW: 269.3kHz
	};

	#elif defined FREQUENCY_BAND_915
   const SEGMENT_VARIABLE (Parameters[4][8], U8, code) =
   {
	   //FR_S1,  FR_S2,	FR_S3,  FR_ST,  ST,	CHNBR,	PREAL,	PREARV
      {0x75,    0x0A,   0x80,   48,   	 3,	50,		160,		0x40},		//DR: 2.4kbps, DEV: +-38.4kHz, BBBW: 95.3kHz
      {0x75,    0x0A,   0x80,   48,   	 3,	50,		160,		0x40},		//DR: 9.6kbps, DEV: +-38.4kHz, BBBW: 102.2kHz
      {0x75,    0x0A,   0x80,   78,   	 5,	25,		139,		0x16},		//DR: 50kbps, DEV: +-25kHz, BBBW: 112.8kHz
      {0x75,    0x0A,   0x80,   100,  	 7,	25,		193,		0x82}			//DR: 128kbps, DEV: +-64kHz, BBBW: 269.3kHz
   };
	#endif
#endif//MORE_CHANNEL_IS_USED



const SEGMENT_VARIABLE (FrequencyTable[50], U8, code) =
{	 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10,11,12,13,14,15,16,17,18,19,
	20,5,22,23,24,25,26,27,28,29,
	30,31,32,6,34,35,36,37,38,39,
	3,41,42,43,44,45,46,47,48,49
};	


const SEGMENT_VARIABLE (EZMacProByteTime[4], U16, code) =
{

   BYTE_TIME(2400),
   BYTE_TIME(9600),
   BYTE_TIME(50000),
   BYTE_TIME(128000L)
};
