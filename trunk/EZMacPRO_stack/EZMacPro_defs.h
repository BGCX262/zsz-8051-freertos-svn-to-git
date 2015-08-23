#ifndef  EZ_MAC_PRO_DEFS_H
#define  EZ_MAC_PRO_DEFS_H
//================================================================================================
// EZMacPRO_defs.h
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
//================================================================================================
// compiler_defs.h must be included before this header file
//------------------------------------------------------------------------------------------------
//                                  DEFINITIONS
//------------------------------------------------------------------------------------------------

#define EZMACPRO_VERSION	"2.0r"

//#define B1_ONLY

//#define FREQUENCY_BAND_434
//#define FREQUENCY_BAND_868
#define FREQUENCY_BAND_915

#define TRANSCIEVER_OPERATION
//#define TRANSMITTER_ONLY_OPERATION
//#define RECEIVER_ONLY_OPERATION

#define FOUR_CHANNEL_IS_USED
//#define MORE_CHANNEL_IS_USED

//#define ANTENNA_DIVERSITY_ENABLED
//#define STANDARD_EZMAC_PACKET_FORMAT
#define EXTENDED_PACKET_FORMAT
#define PACKET_FORWARDING_SUPPORTED

#define RECEIVED_BUFFER_SIZE 64

#define FORWARDED_PACKET_TABLE_SIZE 8

#define MAX_LBT_RETRIES       2

#define EZMACPRO_ADC_GAIN 0x00

#define EZMACPRO_ADC_AMP_OFFSET 0x00

#define GPIO0_FUNCTION 	0x14	// RX DATA

#ifdef ANTENNA_DIVERSITY_ENABLED
	#define GPIO1_FUNCTION	0x17	// ANT1 SW
	#define GPIO2_FUNCTION	0x18	// ANT2 SW
#else
 	#define GPIO1_FUNCTION	0x12	// TX state
	#define GPIO2_FUNCTION	0x15	// RX state
#endif


//------------------------------------------------------------------------------------------------
// Memory Model specifiers
//------------------------------------------------------------------------------------------------

#define REGISTER_MSPACE                SEG_IDATA
//#define REGISTER_MSPACE              SEG_DATA
#define BUFFER_MSPACE                  SEG_XDATA
#define FORWARDED_PACKET_TABLE_MSPACE  SEG_XDATA
#define TEST_CODE_MSPACE               SEG_XDATA
#define EZMAC_PRO_GLOBAL_MSPACE        SEG_DATA
#define BIT_MSPACE							SEG_BDATA

//------------------------------------------------------------------------------------------------
// Check invalid combinations
//------------------------------------------------------------------------------------------------
#ifdef   EXTENDED_PACKET_FORMAT
#ifdef      STANDARD_EZMAC_PACKET_FORMAT
#error         "Only one packet format can be selected!"
#endif      // STANDARD_EZMAC_PACKET_FORMAT
#endif   // EXTENDED_PACKET_FORMAT

#ifndef  EXTENDED_PACKET_FORMAT
#ifdef      PACKET_FORWARDING_SUPPORTED
#error         "Packet forwarding requires the extended packet configuration!"
#endif      // PACKET_FORWARDING_SUPPORTED
#endif   // EXTENDED_PACKET_FORMAT

#ifdef   FOUR_CHANNEL_IS_USED
#ifdef      MORE_CHANNEL_IS_USED
#error         "Only one channel configuration can be selected!"
#endif      // MORE_CHANNEL_IS_USED
#endif   // FOUR_CHANNEL_IS_USED

#ifdef   MORE_CHANNEL_IS_USED
#ifdef      ANTENNA_DIVERSITY_ENABLED
#error         "Antenna diversity is allowed only four channel!"
#endif      // ANTENNA_DIVERSITY_ENABLED
#endif   // MORE_CHANNEL_IS_USED


#ifdef   TRANSMITTER_ONLY_OPERATION
#ifdef      PACKET_FORWARDING_SUPPORTED
#error         "Packet Forwarding is not supported by Transmitter Only configuration!"
#endif      // PACKET_FORWARDING_SUPPORTED
#endif   // TRANSMITTER_ONLY_OPERATION


#ifdef   RECEIVER_ONLY_OPERATION
#ifdef      PACKET_FORWARDING_SUPPORTED
#error         "Packet Forwarding is not supported by Receiver Only configuration!"
#endif      // PACKET_FORWARDING_SUPPORTED
#endif   // RECEIVER_ONLY_OPERATION

#ifdef   TRANSMITTER_ONLY_OPERATION
#ifdef      RECEIVER_ONLY_OPERATION
#error         "Both Tranmitter Only and Receiver Only cannot be defined!"
#endif      // RECEIVER_ONLY_OPERATION
#endif   // TRANSMITTER_ONLY_OPERATION

#ifdef   TRANSCEIVER_OPERATION
#ifdef      TRANSMITTER_ONLY_OPERATION
#error         "Both Tranceiver and Transmitter Only cannot be defined!"
#endif      // TRANSMITTER_ONLY_OPERATION
#ifdef      RECEIVER_ONLY_OPERATION
#error         "Both Transceiver and Receiver Only cannot be defined!"
#endif      // RECEIVER_ONLY_OPERATION
#endif   // TRANSCEIVER_OPERATION

#ifdef 	FREQUENCY_BAND_434
#ifdef		FREQUENCY_BAND_868
#error			"Only one frequency band can be defined!"
#endif		//FREQUENCY_BAND_868
#ifdef		FREQUENCY_BAND_915
#error			"Only one frequency band can be defined!"
#endif		//FREQUENCY_BAND_915
#endif	//FREQUENCY_BAND_434	

#ifdef 	FREQUENCY_BAND_868
#ifdef		FREQUENCY_BAND_434
#error			"Only one frequency band can be defined!"
#endif		//FREQUENCY_BAND_434
#ifdef		FREQUENCY_BAND_915
#error			"Only one frequency band can be defined!"
#endif		//FREQUENCY_BAND_915
#endif	//FREQUENCY_BAND_868	

#ifdef 	FREQUENCY_BAND_915
#ifdef		FREQUENCY_BAND_434
#error			"Only one frequency band can be defined!"
#endif		//FREQUENCY_BAND_434
#ifdef		FREQUENCY_BAND_868
#error			"Only one frequency band can be defined!"
#endif		//FREQUENCY_BAND_868
#endif	//FREQUENCY_BAND_915	
			 
#if (FORWARDED_PACKET_TABLE_SIZE > 15)
#error "maximum FORWARDED_PACKET_TABLE_SIZE is 15 !"
#endif

#if (RECEIVED_BUFFER_SIZE > 64)
#error "The maximum size of the Received Data Buffer is 64!"
#endif

//------------------------------------------------------------------------------------------------

#endif //EZ_MAC_PRO_DEFS_H
