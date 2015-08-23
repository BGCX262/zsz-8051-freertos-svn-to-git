//================================================================================================
// EZMacProCallBack.c
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
#include "si4432_v2.h"                 //
#include "compiler_defs.h"
#include "hardware_defs.h"             // requires compiler_defs.h
#include "EZMacPro_defs.h"
#include "EZMacPro.h"                  // requires EZMacPro_defs.h
#include "EZMacProCallBack.h"          // requires compiler_defs.h
//---------------------------------------------------------------------------------------------------
// Global Variables
//
// MAC Registers
//
//---------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------
//
// local function prototypes
//
//---------------------------------------------------------------------------------------------------
//================================================================================================
//
// Callback Functions
//
//================================================================================================

//------------------------------------------------------------------------------------------------
// Function Name: EZMacPRO_LFTimerExpired()
//						The callback function is called if the low-frequency timer expired. It is called
//						from the external interruptroutine.
//						The timer has to start by setting the time period in the LFTMR0 ... 3 registers.
//
// Return Values: None
//
// Parameters   : None
//-----------------------------------------------------------------------------------------------
void EZMacPRO_LFTimerExpired(void)
{
}
//------------------------------------------------------------------------------------------------
// Function Name: EZMacPRO_LowBattery()
//						The callback function is called when the Low Battery Detector circuit in the radio
//						detects that the power supply is below the LBDT threshold. The function is called
//						from the external interrupt routine.
//
// Return Values: None
//
// Parameters   : None
//-----------------------------------------------------------------------------------------------
void EZMacPRO_LowBattery(void)
{
}
//------------------------------------------------------------------------------------------------
// Function Name: EZMacPRO_WokeUp()
//						The callback function is called after EZMAC PRO performs the SLEEP to IDLE state
//						transition correctly and the crystal of the radio is stabilized. The function is
//						called from the external interrupt routine
//
// Return Values: None
//
// Parameters   : None
//-----------------------------------------------------------------------------------------------
void EZMacPRO_WokeUp(void)
{
}
//------------------------------------------------------------------------------------------------
// Function Name: EZMacPRO_WokeUp_Error()
//						The callback function is called when the SLEEP to IDLE transition does not finish
//						within timeout (the crystal of the radio did not stabilize within timeout).
//						After EZMAC PRO calls this callback function, it goes into wake up error state,
//						called WAKE_UP_ERROR. This state is equivalent with SLEEP state.
//						The function is called from the timer interrupt routine
//
// Return Values: None
//
// Parameters   : None
//-------------------------------------------------------------------------------
void EZMacPRO_WokeUp_Error(void)
{
}
//------------------------------------------------------------------------------------------------
// Function Name: EZMacPRO_SyncWordReceived()
//						The callback function is called when the synchron word of the packet is received. 
//						This is a fix time in each packet and can be used for time synchronization.
//						The function is called form the external interrupt routine.
//
// Return Values: None
//
// Parameters   : None
//------------------------------------------------------------------------------------------------
#ifndef TRANSMITTER_ONLY_OPERATION
void EZMacPRO_SyncWordReceived(void)
{
}
#endif // TRANSMITTER_ONLY_OPERATION not defined
//------------------------------------------------------------------------------------------------
// Function Name: EZMacPRO_PacketReceived()
//						The callback function is called when EZMAC PRO receives a valid packet. 
//						The input parameter (rssi) of the callback function is the RSSI of the received packet
//						measured after the synchron word is received. The function is called from the external
//						interrupt routine.
//
// Return Values: None
//
// Parameters   : rssi- the Receive Signal Strength 
//------------------------------------------------------------------------------------------------
#ifndef TRANSMITTER_ONLY_OPERATION
void EZMacPRO_PacketReceived(U8 rssi)
{
   rssi=rssi;                          // touch rssi to eliminate warning
}
#endif // TRANSMITTER_ONLY_OPERATION not defined
//------------------------------------------------------------------------------------------------
// Function Name: EZMacPRO_PacketSent()
//						The callback function is called after EZMAC PRO transmits a packet correctly.
//						If acknowledgement is also requested, the callback function is called after
//						the acknowledgement has arrived. The function is called from the external interrupt routine.
//
// Return Values: None
//
// Parameters   : None
//------------------------------------------------------------------------------------------------
#ifndef RECEIVER_ONLY_OPERATION
void EZMacPRO_PacketSent(void)
{
}
#endif // RECEIVER_ONLY_OPERATION not defined
//------------------------------------------------------------------------------------------------
// Function Name: EZMacProTX_ErrorLBT_Timeout()
//						The callback function is called if the Listen Before Mechanism failed to access 
//						the channel after the maximum number of retry times (defined by the MAX_LBT_RETRIES definition).
//						After EZMAC PRO calls this callback function, it goes into LBT error state,
//						called TX_ERROR_CHANNEL_BUSY. This state is equivalent to the IDLE state. 
//						The function is called from the timer interrupt routine.
//
// Return Values: None
//
// Parameters   : None
//------------------------------------------------------------------------------------------------
#ifdef TRANSCIEVER_OPERATION
void EZMacProTX_ErrorLBT_Timeout (void)
{
}
#endif//TRANSCIEVER_OPERATION
//------------------------------------------------------------------------------------------------
// Function Name: EZMacProTX_ErrorNoAck()
//						The callback function is called if EZMAC PRO did not receive the acknowledgement
//						packet within timeout. After this callback function is called, EZMAC PRO goes 
//						into ACK error state, called TX_ERROR_NO_ACK. This state is equivalent to the IDLE state.
//						The function is called from the timer interrupt routine.
//
// Return Values: None
//
// Parameters   : None
//------------------------------------------------------------------------------------------------
#ifdef EXTENDED_PACKET_FORMAT
#ifdef TRANSCIEVER_OPERATION
void EZMacProTX_ErrorNoAck (void)
{
}
#endif//TRANSCIEVER_OPERATION
#endif//EXTENDED_PACKET_FORMAT
//------------------------------------------------------------------------------------------------
// Function Name: EZMacPRO_PacketForwarding()
//						The callback function is called when EZMAC PRO receives a packet that has to be forwarded. 
//						Prior to the packet forwarding, the upper software layer has the ability to read and 
//						change the content of the payload. It provides the possibility that the upper software layer
//						adds routing information into the packet if needed. All other necessary modification
//						on the packet (e.g. decreasing the radius field) and packet forwarding management is done
//						by the MAC itself. The function is called from the external interrupt routine.
//
// Return Values: None
//
// Parameters   : None
//------------------------------------------------------------------------------------------------
#ifdef PACKET_FORWARDING_SUPPORTED
void EZMacPRO_PacketForwarding(void)
{
}
#endif//PACKET_FORWARDING_SUPPORTED

//------------------------------------------------------------------------------------------------
// Function Name: EZMacPRO_CRCError()
//						The callback function is called if EZMacPRO and EZHOP received a packet with
//						incorrect CRC. After this callback function is called EZMacPRO and EZHOP go into
//						receive state. The function is called from the external interrupt routine. 
//
// Return Values: None
//
// Parameters   : None
//------------------------------------------------------------------------------------------------

void EZMacPRO_CRCError(void)
{
}

//================================================================================================
// end EZMacProCalBacks.c
//================================================================================================
