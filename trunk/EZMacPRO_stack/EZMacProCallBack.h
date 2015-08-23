#ifndef  EZ_MAC_PRO_CALL_BACK_H
#define  EZ_MAC_PRO_CALL_BACK_H
//================================================================================================
// EZMacProCallBack.h
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
// defines
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
// Public Variables (API)
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
// Public function prototypes (API)
//------------------------------------------------------------------------------------------------
void EZMacPRO_LFTimerExpired(void);
void EZMacPRO_WokeUp(void);
void EZMacPRO_WokeUp_Error(void);
void EZMacPRO_SyncWordReceived(void);
void EZMacPRO_PacketReceived(U8);
void EZMacPRO_PacketForwarding(void);
void EZMacPRO_PacketSent(void);
void EZMacPRO_LowBattery(void);
void EZMacPRO_CRCError(void);
void EZMacProTX_ErrorLBT_Timeout(void);
void EZMacProTX_ErrorNoAck(void);
//=================================================================================================
//=================================================================================================
#endif //EZ_MAC_PRO_CALL_BACK_H
