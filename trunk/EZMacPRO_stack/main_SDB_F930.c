//================================================================================================
// main_SDB_F930.c
//------------------------------------------------------------------------------------------------
// Copyright 2010 Silicon Laboratories, Inc.
// http://www.silabs.com
//
// C File Description:
//
//  This file is specific to the F930 hardware.
//
// Target:
//    C8051F930
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
#include "hardware_defs.h"             // requires compiler_defs.h
#include "EZMacPro_defs.h"
#include "EZMacPro.h"                  // requires EZMacPro_defs.h
//-------------------------------------------- -------------------------------------------------------
#ifndef C8051F930_DEFS_H
#error "Please specify c8051930_defs.h in hardware_defs.h"
#endif
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------

#define SPI_CKR_VALUE 0x00             // SPI CLK SYSCLK/2

//------------------------------------------------------------------------------------------------
// Internal Function Prototypes
//------------------------------------------------------------------------------------------------
 void MCU_Init(void);
//------------------------------------------------------------------------------------------------
// SPI ISR proto required in main for SDCC support
//------------------------------------------------------------------------------------------------
INTERRUPT_PROTO(externalIntISR, INTERRUPT_INT0);
INTERRUPT_PROTO(timerIntT0_ISR, INTERRUPT_TIMER0);

//------------------------------------------------------------------------------------------------
// application buffer
//------------------------------------------------------------------------------------------------

SEGMENT_VARIABLE(buffer[1], U8, BUFFER_MSPACE);

//================================================================================================
// Functions
//================================================================================================
//------------------------------------------------------------------------------------------------
// main ()
//------------------------------------------------------------------------------------------------
void _sdcc_external_startup( void )
{
	PCA0MD &= ~0x40;        			   //Clear watchdog timer enable // 关闭使能看门狗	
	PCA0MD =0x00;
}

void main (void)
{
   PCA0MD   &= ~0x40;                  // disable F930 watchdog

   MCU_Init();

   EA = 1;                             // enable global interrupts

   EZMacPRO_Init();
	EZMacPRO_Sleep();	
   EZMacPRO_Wake_Up();
   while(EZMacProReg.name.MSR!=EZMAC_PRO_IDLE);
	
	//dummy calls
	
	EZMacPRO_Idle();
   EZMacPRO_Transmit();
   EZMacPRO_Receive();
   EZMacPRO_Idle();
   EZMacPRO_Reg_Write(0, 0);
   EZMacPRO_Reg_Read(1, buffer);
   EZMacPRO_TxBuf_Write(1, buffer);
   EZMacPRO_RxBuf_Read(1, buffer);

   while(1)
   {
			
	  
   }

}
//===============================================================================================
//
// Init Functions
//
//===============================================================================================
//------------------------------------------------------------------------------------------------
//
// Init_MCU()
//
//------------------------------------------------------------------------------------------------
//
// P0.0  -  skipped, PB0
// P0.1  -  skipped  PB1
// P0.2  -  skipped, analog input, XTAL
// P0.3  -  skipped, analog input, XTAL
// P0.4  -  UART TX, push-pull output
// P0.5  -  UART RX, open drain input
// P0.6  -  skipped, /INT0 external interrupt input, IRQ from radio
// P0.7  -  skipped

// P1.0  -  SCK  (SPI1), Push-Pull,  Digital
// P1.1  -  MISO (SPI1), Open-Drain, Digital
// P1.2  -  MOSI (SPI1), Push-Pull,  Digital
// P1.3  -  NSS, Push-Pull,  Digital
// P1.4  -  skipped LED1
// P1.5  -  skipped LED2
// P1.6  -  skipped LED3
// P1.7  -  skipped LED4
//
// P2.0  -  skipped, PB3
// P2.1  -  skipped  PB4
//
//------------------------------------------------------------------------------------------------
void MCU_Init (void)
{
   // Init LPOSC to 10 MHZ clock for now
   CLKSEL    = 0x14;

   // Init Ports
   P0SKIP  |= 0xCF;                    // skip P0.0-3 & 0.6-7
   P0MDIN  &= ~0x0C;                   // P0.2 & P0.3 analog input for XTAL
   P0MDOUT |= 0x10;                    // UART TX Push-Pull
   XBR0    |= 0x01;                    // Enable UART

   XBR1    |= 0x40;                    // Enable SPI1 (3 wire mode)
   P1MDOUT |= 0x01;                    // Enable SCK push pull
   P1MDOUT |= 0x04;                    // Enable MOSI push pull
   P1SKIP  |= 0x08;                    // skip NSS
   P1MDOUT |= 0x08;                    // Enable NSS push pull
   P1SKIP  |= 0xF0;                    // skip LEDs
   P1MDOUT |= 0xF0;                    // Enable LEDS push pull

   P2SKIP  |= 0x03;                    // skip PB3 & 4

   SFRPAGE   = CONFIG_PAGE;
   P0DRV     = 0x10;                   // TX high current mode
   P1DRV     = 0xFD;                   // MOSI, SCK, NSS, LEDs high current mode
   SFRPAGE   = LEGACY_PAGE;

   XBR2    |= 0x40;                     // enable Crossbar

   IT01CF    = 0x06;                   // INT0 mapped to P0.6 active low
   TCON &= ~0x03;                      // clear IE0 & IT0

   // Init 16 MHz crystal
#if (SYSCLK_MHZ == 16000000)
    FLSCL     = 0x40;
#endif

   OSCXCN    = 0x67;
   while ((OSCXCN & 0x80) == 0);
   CLKSEL    = 0x01 | SYSCLK_DIV;     // SYSCLK_DIV in hardware_defs.h

   // Init SPI
   SPI_CFG   = 0x40;                   // master mode
   SPI_CN    = 0x00;                   // 3 wire master mode
   SPI_CKR   = SPI_CKR_VALUE;          // initialize SPI prescaler
   SPI_CN   |= 0x01;                   // enable SPI
   NSS = 1;                            // set NSS high

   // Init T0
   CKCON &= ~0x07;                     // clear T0M & SCAx bits
   CKCON |= 0x01;                      // use SYSCLK/4 (1 us tick for 4 MHZ clk)
   TMOD &= ~0x0f;                      // clear T1 bits in TMOD
   TMOD |=  0x01;                      // set TMOD for 16 bit timer

   // Init UART
   SCON0 = 0x10;                       // SCON0: 8-bit variable bit rate, no receiver
   CKCON |= 0x08;                      // T1 Baud Generator uses SYSCLK, (prescaler used by MAC)
   //TH1 = T1_RELOAD;                    // reload value calculated from BAUD
   //TL1 = T1_RELOAD;                    // also load into TL1
   TMOD &= ~0xf0;                      // clear T1 bits in TMOD
   TMOD |=  0x20;                      // set TMOD for 8 bit reload
   TR1 = 1;                            // START Timer1
   TI0 = 1;                            // Transciever ready
   // Init LEDs
   LED1 = 0;
   LED2 = 0;
   LED3 = 0;
   LED4 = 0;
}
//===============================================================================================
//
// UART Functions
//
//===============================================================================================
//------------------------------------------------------------------------------------------------
//
// UART0_Init
//
//------------------------------------------------------------------------------------------------
/*

*/
