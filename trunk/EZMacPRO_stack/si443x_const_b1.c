//================================================================================================
// si443x_const_b1.c
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
#include "si443x_const_b1.h"


const SEGMENT_VARIABLE (RfSettingsB1[NUMBER_OF_SAMPLE_SETTING][NUMBER_OF_PARAMETER], U8, code) =
{
//	IFBW, COSR, CRO2, CRO1, CRO0, CTG1, CTG0, TDR1, TDR0, MMC1, TXFDEV,	RXFDEV 	AFC, AFC Limiter
	{0x1C, 0x41, 0x60, 0x27, 0x52, 0x00, 0x04, 0x13, 0xA9, 0x24, 0x3D, 	0x3D,		0x40,	0x1A},  		//DR: 2.4kbps, DEV: +-38.4kHz, BBBW: 85.1kHz
	{0x1D, 0xD0, 0x00, 0x9D, 0x49, 0x00, 0x29, 0x4e, 0xa5, 0x24, 0x3D, 	0x3D,		0x40,	0x1E},		//DR: 9.6kbps, DEV: +-38.4kHz, BBBW: 95.3kHz
	{0x05, 0x50, 0x01, 0x99, 0x9A, 0x03, 0x35, 0x0C, 0xCD, 0x04, 0x28, 	0x28,		0x40,	0x28},		//DR: 50kbps, DEV: +-25kHz, BBBW: 112.1kHz
	{0x83, 0x5e, 0x01, 0x5D, 0x86, 0x02, 0xBB, 0x20, 0xc5, 0x04, 0x66, 	0x66,		0x40,	0x50}			//DR: 128kbps, DEV: +-64kHz, BBBW: 269.3kHz
};

