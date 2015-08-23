#ifndef HARDWARE_DEFS_H
#define HARDWARE_DEFS_H
//================================================================================================
// hardware_defs.h
//------------------------------------------------------------------------------------------------
// Copyright 2010 Silicon Laboratories, Inc.
// http://www.silabs.com
//
// Header File Description:
//    This file is an alias for the hardware specific header file.
//
// Target:
//    Supported C8051Fxxx MCUs.
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
// Specify which hardware file to include here.
//---------------------------------------------------------------------------------------------------
#include "c8051f930_defs.h"
//#include "c8051f340_defs.h"
//================================================================================================
//================================================================================================
#ifdef C8051F930_DEFS_H
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
// C8051F930 Pin definitions for Software Development Board
//------------------------------------------------------------------------------------------------
// P0.6  -  RF_IRQ EI0
// P1.0  -  SCK  (SPI0), Push-Pull,  Digital
// P1.1  -  MISO (SPI0), Open-Drain, Digital
// P1.2  -  MOSI (SPI0), Push-Pull,  Digital
// P1.3  -  NSS  (SPI0), Push-Pull,  Digital
//------------------------------------------------------------------------------------------------
// C8051F930 Hardware Macros
//------------------------------------------------------------------------------------------------
#define INTERRUPT_SPI            INTERRUPT_SPI0
#define SPIF                     SPIF1
#define TXBMT                    TXBMT1
#define SPI_DAT                  SPI1DAT
#define SPI_CN                   SPI1CN
#define SPI_CFG                  SPI1CFG
#define SPI_CKR                  SPI1CKR

//------------------------------------------------------------------------------------------------
// C8051F930 Hardware bit definitions (using compiler_def.h macros)
//------------------------------------------------------------------------------------------------

#ifdef SOFTWARE_DEVELOPMENT_BOARD
SBIT(NSS, SFR_P1, 3);
SBIT(IRQ, SFR_P0, 6);
SBIT(PB1, SFR_P0, 0);
SBIT(PB2, SFR_P0, 1);
SBIT(PB3, SFR_P2, 0);
SBIT(PB4, SFR_P2, 1);
SBIT(LED1, SFR_P1, 4);
SBIT(LED2, SFR_P1, 5);
SBIT(LED3, SFR_P1, 6);
SBIT(LED4, SFR_P1, 7);
#endif//SOFTWARE_DEVELOPMENT_BOARD
#ifdef ZH_S_ZH
SBIT(NSS, SFR_P1, 3);
SBIT(IRQ, SFR_P0, 6);
SBIT(PB1, SFR_P0, 0);
SBIT(PB2, SFR_P0, 1);
SBIT(PB3, SFR_P2, 0);
SBIT(PB4, SFR_P2, 1);
SBIT(LED1, SFR_P1, 4);
SBIT(LED2, SFR_P1, 5);
SBIT(LED3, SFR_P1, 6);
SBIT(LED4, SFR_P1, 7);
#endif//ZH_S_ZH


#ifdef EZLINK_MODULE
SBIT(NSS, SFR_P1, 3);
SBIT(IRQ, SFR_P0, 6);
SBIT(SDN, SFR_P0, 1);
SBIT(RC_LED, SFR_P2, 0);      
SBIT(TR_LED, SFR_P1, 6);      
SBIT(PB,     SFR_P0, 7);      
#endif//EZLINK_MODULE
//------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------
// C8051F930 Hardware bit definitions (using compiler_def.h macros)
//------------------------------------------------------------------------------------------------
#define ENABLE_MAC_INTERRUPTS()        IE|=0x03
#define DISABLE_MAC_INTERRUPTS()       IE&=~0x03

#define ENABLE_MAC_EXT_INTERRUPT()     EX0=1
#define DISABLE_MAC_EXT_INTERRUPT()    EX0=0
#define CLEAR_MAC_EXT_INTERRUPT()      IE0=0

#define ENABLE_MAC_TIMER_INTERRUPT()   ET0=1
#define DISABLE_MAC_TIMER_INTERRUPT()  ET0=0
#define CLEAR_MAC_TIMER_INTERRUPT()    TF0=0
#define STOP_MAC_TIMER()               TR0=0
//------------------------------------------------------------------------------------------------
// Supported SYSCLK values
//------------------------------------------------------------------------------------------------
#ifdef SOFTWARE_DEVELOPMENT_BOARD
//#define SYSCLK_MHZ 16
//#define SYSCLK_MHZ  8
#define SYSCLK_MHZ  4
#endif//SOFTWARE_DEVELOPMENT_BOARD

#ifdef ZH_S_ZH
//#define SYSCLK_MHZ 16
//#define SYSCLK_MHZ  8
#define SYSCLK_MHZ  20
#endif//ZH_S_ZH


#ifdef EZLINK_MODULE
#define SYSCLK_MHZ  5
#endif//EZLINK_MODULE

//------------------------------------------------------------------------------------------------
// SYSCLK divider
//------------------------------------------------------------------------------------------------

#ifdef SOFTWARE_DEVELOPMENT_BOARD 

#if (SYSCLK_MHZ == 2)
#define SYSCLK_DIV 0x30
#elif (SYSCLK_MHZ == 4)
#define SYSCLK_DIV 0x20
#elif  (SYSCLK_MHZ == 8)
#define SYSCLK_DIV 0x10
#elif  (SYSCLK_MHZ == 16)
#define SYSCLK_DIV 0x00
#else
#error "SYSCLK_MHZ undefined"
#endif

#endif//SOFTWARE_DEVELOPMENT_BOARD

#ifdef ZH_S_ZH

#if (SYSCLK_MHZ == 2)
#define SYSCLK_DIV 0x30
#elif (SYSCLK_MHZ == 4)
#define SYSCLK_DIV 0x20
#elif  (SYSCLK_MHZ == 8)
#define SYSCLK_DIV 0x10
#elif  (SYSCLK_MHZ == 16)
#define SYSCLK_DIV 0x00
#elif  (SYSCLK_MHZ == 20)
#define SYSCLK_DIV 0x00
#else
#error "SYSCLK_MHZ undefined"
#endif

#endif//ZH_S_ZH

//------------------------------------------------------------------------------------------------
// UART Baud Rate
//------------------------------------------------------------------------------------------------
#define SYSCLK                         (SYSCLK_MHZ*1000000L)

//#define BAUD                           (38400L)
//#if (SYSCLK/BAUD/2/1)<256
//   #define T1_RELOAD                   -((SYSCLK/BAUD/1+1)/2)
//#else
// #error "error: baud rate too low!"
//#endif
//------------------------------------------------------------------------------------------------
// SPI Clock Rate
//------------------------------------------------------------------------------------------------

#define SPI_CKR_VALUE 0x00             // SPI CLK SYSCLK/2

//-----------------------------------------------------------------------------------------------
// end C8051F930_DEFS_H
//================================================================================================
#endif
//================================================================================================
// End
//================================================================================================
#endif //HARDWARE_DEFS_H
