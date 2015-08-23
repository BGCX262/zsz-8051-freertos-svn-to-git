//================================================================================================
// EZMacPro.c
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
#include "si4432_v2.h"
#include "compiler_defs.h"
#include "hardware_defs.h"                   // requires compiler_defs.h
#include "EZMacPro_defs.h"
#include "EZMacPro.h"                        // requires EZMacPro_defs.h
#include "EZMacPro_const.h"						// requires EZMacPro.h	
#ifndef B1_ONLY
#include "si4432_const_v2.h"
#include "si4431_const_a0.h"
#endif//B1_ONLY
#include "si443x_const_b1.h"



//---------------------------------------------------------------------------------------------------
// Global Variables
//
// MAC Registers
//
//---------------------------------------------------------------------------------------------------
SEGMENT_VARIABLE(EZMacProReg, EZMacProUnion, REGISTER_MSPACE);
SEGMENT_VARIABLE(EZMacProTimerMSB, U8, EZMAC_PRO_GLOBAL_MSPACE);
SEGMENT_VARIABLE(EZMacProCurrentChannel, U8, EZMAC_PRO_GLOBAL_MSPACE);

#ifdef EXTENDED_PACKET_FORMAT
SEGMENT_VARIABLE(TimeoutACK, U32, EZMAC_PRO_GLOBAL_MSPACE);
SEGMENT_VARIABLE(EZMacProSequenceNumber, U8, EZMAC_PRO_GLOBAL_MSPACE);
SEGMENT_VARIABLE(PreamRegValue, U8, EZMAC_PRO_GLOBAL_MSPACE);
#endif//EXTENDED_PACKET_FORMAT

#ifndef TRANSMITTER_ONLY_OPERATION
SEGMENT_VARIABLE(RxBuffer[RECEIVED_BUFFER_SIZE], U8 , BUFFER_MSPACE);
SEGMENT_VARIABLE(EZMacProReceiveStatus, U8, EZMAC_PRO_GLOBAL_MSPACE);
SEGMENT_VARIABLE(EZMacProRSSIvalue, U8, EZMAC_PRO_GLOBAL_MSPACE);
SEGMENT_VARIABLE(TimeoutPreamble, U16, EZMAC_PRO_GLOBAL_MSPACE);
SEGMENT_VARIABLE(TimeoutSyncWord, U32, EZMAC_PRO_GLOBAL_MSPACE);
SEGMENT_VARIABLE(TimeoutHeader, U16, EZMAC_PRO_GLOBAL_MSPACE);
SEGMENT_VARIABLE(TimeoutRX_Packet, U32, EZMAC_PRO_GLOBAL_MSPACE);
#ifndef B1_ONLY
SEGMENT_VARIABLE(RX_Freq_dev, U8, EZMAC_PRO_GLOBAL_MSPACE);
#endif//B1_ONLY
SEGMENT_VARIABLE(TimeoutChannelSearch, U16, EZMAC_PRO_GLOBAL_MSPACE);
#endif//TRANSMITTER_ONLY_OPERATION

#ifndef RECEIVER_ONLY_OPERATION
SEGMENT_VARIABLE(TimeoutTX_Packet, U32, EZMAC_PRO_GLOBAL_MSPACE);
#ifndef B1_ONLY
SEGMENT_VARIABLE(TX_Freq_dev, U8, EZMAC_PRO_GLOBAL_MSPACE);
#endif//B1_ONLY
#endif//RECEIVER_ONLY_OPERATION

#ifdef TRANSCIEVER_OPERATION
SEGMENT_VARIABLE(TimeoutLBTI, U32, EZMAC_PRO_GLOBAL_MSPACE);
SEGMENT_VARIABLE(BusyLBT, U8, EZMAC_PRO_GLOBAL_MSPACE);
SEGMENT_VARIABLE(EZMacProRandomNumber, U8, EZMAC_PRO_GLOBAL_MSPACE);
SEGMENT_VARIABLE(EZMacProLBT_Retrys, U8, EZMAC_PRO_GLOBAL_MSPACE);
#endif//TRANSCIEVER_OPERATION

#ifdef PACKET_FORWARDING_SUPPORTED
SEGMENT_VARIABLE(ForwardedPacketTable[FORWARDED_PACKET_TABLE_SIZE], ForwardedPacketTableEntry, FORWARDED_PACKET_TABLE_MSPACE);
#endif//PACKET_FORWARDING_SUPPOERTED

#ifdef ANTENNA_DIVERSITY_ENABLED
SEGMENT_VARIABLE(Selected_Antenna, U8, EZMAC_PRO_GLOBAL_MSPACE);
#endif//ANTENNA_DIVERSITY_ENABLED

#ifdef MORE_CHANNEL_IS_USED 
SEGMENT_VARIABLE(SelectedChannel, U8, EZMAC_PRO_GLOBAL_MSPACE);
SEGMENT_VARIABLE(maxChannelNumber, U8, EZMAC_PRO_GLOBAL_MSPACE);
#endif //MORE_CHANNEL_IS_USED


//---------------------------------------------------------------------------------------------------
//
// local function prototypes
//
//---------------------------------------------------------------------------------------------------
void macSpiWriteReg (U8, U8);
U8   macSpiReadReg (U8);
void macSpiWriteFIFO (U8, VARIABLE_SEGMENT_POINTER(, U8, BUFFER_MSPACE));
void macSpiReadFIFO (U8, VARIABLE_SEGMENT_POINTER(, U8, BUFFER_MSPACE));
void macTimeout (U32);
void SetRfParameters(U8);
void macSpecialRegisterSettings(U8);
void macUpdateDynamicTimeouts (U8, U8);
#ifdef PACKET_FORWARDING_SUPPORTED
void initForwardedPacketTable (void);
#endif //PACKET_FORWARDING_SUPPORTED
void macSetEnable2(U8);
void macSetFunction1(U8);
#ifdef TRANSCIEVER_OPERATION
void macUpdateLBTI (U8);
#endif//TRANSCIEVER_OPERATION
void timerIntTouch (void);             // added for library support
void extIntTouch (void);               // added for library support

//================================================================================================
//
// API Functions
//
//================================================================================================
//------------------------------------------------------------------------------------------------
// Function Name:	EZMacPRO_Init()
//						Initializes the EZRadioPRO device. The function has to be called in the power-on routine.
// Return Values : MAC_OK: if the MAC recognized the used chip
//						CHIPTYPE_ERROR: if the MAC didn't recognize the used chip
//------------------------------------------------------------------------------------------------
MacParams EZMacPRO_Init(void)
{
   U8 temp8;
   DISABLE_MAC_INTERRUPTS();                     // disable MAC interrupts
   //Set the init value of the MAC  regsiters
   for(temp8=0;temp8<EZ_LASTREG;temp8++)
      EZMacProReg.array[temp8] = 0x00;
	EZMacProReg.name.MCR    = 0x1C;
   EZMacProReg.name.SECR   = 0x50;
   EZMacProReg.name.TCR    = 0x38;
   EZMacProReg.name.RCR    = 0x04;
#ifdef FOUR_CHANNEL_IS_USED
   EZMacProReg.name.FR1    = 1;
   EZMacProReg.name.FR2    = 2;
   EZMacProReg.name.FR3    = 3;
#endif//FOUR_CHANNEL_IS_USED
#ifdef MORE_CHANNEL_IS_USED
	for(temp8=0;temp8<50;temp8++)
		EZMacProReg.array[4+temp8] = FrequencyTable[temp8]; 
#endif//MORE_CHANNEL_IS_USED
   EZMacProReg.name.PFCR   = 0x02;
   EZMacProReg.name.MPL    = 0x40;
   EZMacProReg.name.MSR    = 0x00;
   EZMacProReg.name.SCID   = 0xCD;
   EZMacProReg.name.SFID   = 0x01;
   EZMacProReg.name.PLEN   = 0x01;
#ifdef TRANSCIEVER_OPERATION 
   EZMacProReg.name.LBTLR  = 0x78; // -60 dBm
   EZMacProReg.name.LBTIR  = 0x8A;
#endif//TRANSCIEVER_OPERATION
   EZMacProReg.name.LBDR   = 0x14;
   EZMacProReg.name.LFTMR2 = 0x40;

   // read Si443x interrupts to clear
   macSpiReadReg(SI4432_INTERRUPT_STATUS_1);
   macSpiReadReg(SI4432_INTERRUPT_STATUS_2);
   CLEAR_MAC_EXT_INTERRUPT();          // clear INT0 flag

	//enable only the chip ready interrupt
   macSpiWriteReg(SI4432_INTERRUPT_ENABLE_1, 0x00);
   macSpiWriteReg(SI4432_INTERRUPT_ENABLE_2, SI4432_ENCHIPRDY);


   //EZMac state Software Reset
   EZMacProReg.name.MSR = EZMAC_PRO_WAKE_UP;
   //Send the Software Reset Command to the radio
   //use direct write since LBD and WUT initially disabled
   macSpiWriteReg (SI4432_OPERATING_AND_FUNCTION_CONTROL_1, (SI4432_SWRES|SI4432_XTON));

   // set time out to XTAL start-up time
   macTimeout (TIMEOUT_XTAL_START);

   ENABLE_MAC_INTERRUPTS();         // enable INT0  & T0 interrupt

   //wait until the MAC goes to Idle State

   while (EZMacProReg.name.MSR != EZMAC_PRO_IDLE);


   // clear Chip ready and POR interrupts
   macSpiWriteReg(SI4432_INTERRUPT_ENABLE_1, 0x00);
   macSpiWriteReg(SI4432_INTERRUPT_ENABLE_2, 0x00);



  /*Si443x configuration*/
	
#ifdef EXTENDED_PACKET_FORMAT
   if (( EZMacProReg.name.MCR & 0x80 ) == 0x80)//if CID is used 
      //header length 4 byte(CTRL+CID+SID+DID)
      macSpiWriteReg(SI4432_HEADER_CONTROL_2, SI4432_HDLEN_4BYTE | (SYNC_WORD_LENGTH - 1) << 1);
   else
      //header length 3 byte(CTRL+SID+DID)
      macSpiWriteReg(SI4432_HEADER_CONTROL_2, SI4432_HDLEN_3BYTE | (SYNC_WORD_LENGTH - 1 ) << 1);
#else //STANDARD_EZMAC_PACKET_FORMAT
   if (( EZMacProReg.name.MCR & 0x80 ) == 0x80)//if CID is used 
      //header length 3 byte(CID+SID+DID)
      macSpiWriteReg(SI4432_HEADER_CONTROL_2, SI4432_HDLEN_3BYTE | (SYNC_WORD_LENGTH - 1) << 1);
   else
      //header length 2 byte(SID+DID)
      macSpiWriteReg(SI4432_HEADER_CONTROL_2, SI4432_HDLEN_2BYTE | (SYNC_WORD_LENGTH - 1) << 1);
#endif//EXTENDED_PACKET_FORMAT
   //Clear the Header Control Register
   macSpiWriteReg(SI4432_HEADER_CONTROL_1, 0x00);
	//Set the SYNC WORD
  	macSpiWriteReg(SI4432_SYNC_WORD_3, 0x2D);
  	macSpiWriteReg(SI4432_SYNC_WORD_2, 0xD4);

   //Set the modem, frequency parameters and preamble length according to the init register value
   SetRfParameters(EZMacProReg.name.MCR);

   //set GPIO0, GPIO1 and GPIO2 to the init values
	//the init values are in the EZMacPro_defs.h 
   macSpiWriteReg(SI4432_GPIO0_CONFIGURATION, GPIO0_FUNCTION);
   macSpiWriteReg(SI4432_GPIO1_CONFIGURATION, GPIO1_FUNCTION);
   macSpiWriteReg(SI4432_GPIO2_CONFIGURATION, GPIO2_FUNCTION);

   //enable packet handler & CRC16
   macSpiWriteReg(SI4432_DATA_ACCESS_CONTROL, SI4432_ENPACTX | SI4432_ENPACRX | SI4432_ENCRC | SI4432_CRC_16);

   //FIFO mode, GFSK modulation, TX Data Clk is available via the GPIO
   macSpiWriteReg(SI4432_MODULATION_MODE_CONTROL_2, SI4432_MODTYP_GFSK | SI4432_FIFO_MODE | SI4432_TX_DATA_CLK_GPIO);



   //reset digital testbus, disable scan test
   macSpiWriteReg(SI4432_DIGITAL_TEST_BUS, 0x00);

   //select nothing to the Analog Testbus
   macSpiWriteReg(SI4432_ANALOG_TEST_BUS, 0x0B);



#ifdef PACKET_FORWARDING_SUPPORTED
   initForwardedPacketTable ();
#endif
#ifndef B1_ONLY
	//read out the device type register
	temp8 = 	macSpiReadReg (SI4432_DEVICE_VERSION);
	if (temp8 == 0x02) EZMacProReg.name.DTR = 0x00;				//revision V2
	else if (temp8 == 0x04) EZMacProReg.name.DTR = 0x01;		//revision A0
	else if (temp8 == 0x06) EZMacProReg.name.DTR = 0x02;		//revision B1	
	else 
	{	
		EZMacProReg.name.DTR = 0x02;									//default: rev B1
		temp8 = 1;
	}
#endif
#ifdef B1_ONLY
	EZMacProReg.name.DTR = 0x02;									//default: rev B1
#endif
	//set the non-default, special registers accoring to the device type
	macSpecialRegisterSettings(EZMacProReg.name.DTR);			

#ifndef B1_ONLY
	if (EZMacProReg.name.DTR == 0x00)//if rev V2 chip is used 
		//set crystal oscillator control test 
	  	macSpiWriteReg (SI4432_CRYSTAL_OSCILLATOR_CONTROL_TEST, 0x24);
#endif//B1_ONLY

#ifdef ANTENNA_DIVERSITY_ENABLED
		Selected_Antenna = 1;
		//switch ON the internal algorithm
		temp8 = macSpiReadReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2);
   	macSpiWriteReg (SI4432_OPERATING_AND_FUNCTION_CONTROL_2, temp8 | 0x80);
#endif//ANTENNA_DIVERSITY_ENABLED

   // stop Si443x xtal, clear TX, RX , PLL
   // use direct write since LBD and WUT initially disabled
   macSpiWriteReg (SI4432_OPERATING_AND_FUNCTION_CONTROL_1, 0x00);

   // Dummy function calls to support library
   timerIntTouch ();
   extIntTouch ();

	//after EZMAC init the next state is SLEEP 
   EZMacProReg.name.MSR = EZMAC_PRO_SLEEP;
#ifndef B1_ONLY
	if(temp8 == 0)
		//if the MAC recognize the chip type
		return MAC_OK;
	else
		//if the MAC doesn't recognize the chip type
		return CHIPTYPE_ERROR; 
#endif//B1_ONLY
#ifdef B1_ONLY
	return MAC_OK;
#endif//B1_ONLY
}
//-----------------------------------------------------------------------------------------------
// Function Name :	EZMAcPRO_Sleep()
//							Switch the MAC from IDLE to SLEEP mode. It turns off the crystal oscillator 
//							on the radio. This function can be call from only Idle state.
// Return Values :	MAC_OK: the operation was successful.
//							STATE_ERROR: the operation is ignored because the MAC was not in IDLE mode
//-----------------------------------------------------------------------------------------------
MacParams EZMacPRO_Sleep(void)
{
   if (EZMacProReg.name.MSR == EZMAC_PRO_IDLE)
   {
      DISABLE_MAC_INTERRUPTS();         // disable MAC interrupts, just in case

		if (EZMacProReg.name.DTR == 0x00) //if rev V2 chip is used
      	//this register setting is needed (only rev V2)
      	macSpiWriteReg (SI4432_CRYSTAL_OSCILLATOR_CONTROL_TEST, 0x24);
  

      // disable Si443x interrupt sources
      macSpiWriteReg(SI4432_INTERRUPT_ENABLE_1, 0x00);
      macSetEnable2(0x00);

      // stop Si443x xtal, clear TX, RX , PLL
      macSetFunction1(0x00);
		// next state after this function is SLEEP
      EZMacProReg.name.MSR = EZMAC_PRO_SLEEP;

      return MAC_OK;
   }
   else
      return STATE_ERROR;
}
//------------------------------------------------------------------------------------------------
// Function Name :	EZMacPRO_Wake_Up
//							Switches the MAC from SLEEP mode into IDLE mode. Turns on the crystal oscillator
//							of the radio, so the current consumption increases.The MAC has to be in SLEEP mode
//							when calling the function.
// Return Values : 	MAC_OK: the operation was successful.
//							STATE_ERROR: the operation is ignored because the MAC was not in SLEEP mode.
//------------------------------------------------------------------------------------------------
MacParams EZMacPRO_Wake_Up(void)
{

   if (EZMacProReg.name.MSR == EZMAC_PRO_SLEEP) 
   {
      DISABLE_MAC_INTERRUPTS();        // disable MAC interrupts, just in case

      // disable Si443x interrupt 2 sources except ENLBDI & SI4432_ENWUT
      macSpiWriteReg(SI4432_INTERRUPT_ENABLE_1, 0x00);
      // enable chip ready interrupt
      macSetEnable2(SI4432_ENCHIPRDY);

      // clear interrupts
      macSpiReadReg(SI4432_INTERRUPT_STATUS_1);
      macSpiReadReg(SI4432_INTERRUPT_STATUS_2);
      CLEAR_MAC_EXT_INTERRUPT();

      // start Si443x XTAL
      macSetFunction1(SI4432_XTON);
		// start timer with XTAL startup timeout
      macTimeout (TIMEOUT_XTAL_START);
		// next state is WAKE UP state
      EZMacProReg.name.MSR = EZMAC_PRO_WAKE_UP;

      ENABLE_MAC_INTERRUPTS();

      return MAC_OK;
   }
   else
      return STATE_ERROR;
}
//------------------------------------------------------------------------------------------------
// Function Name	:	EZMacPRO_Idle()
//							This is the only function that aborts the ongoing reception and transmission.
//							It could be used to reset the state of the MAC. Calling this function, 
//							EZMAC PRO goes into IDLE state.The function cannot be called in SLEEP mode.
//
// Return Values	: 	MAC_OK: the MAC is set into IDLE mode, and no transmission or reception was aborted.
//							STATE_ERROR: the MAC is set into IDLE mode, and transmission or reception was aborted
//------------------------------------------------------------------------------------------------
MacParams EZMacPRO_Idle(void)
{
	//if the MAC in sleep state
   if (EZMacProReg.name.MSR == EZMAC_PRO_SLEEP)
   {
      return STATE_ERROR;
   }

   DISABLE_MAC_TIMER_INTERRUPT();        // clear EX0 & ET0

   // disable all Si443x interrupt sources
   macSpiWriteReg(SI4432_INTERRUPT_ENABLE_1, 0x00);
   macSetEnable2(0x00);

   // clear interrupts
   macSpiReadReg(SI4432_INTERRUPT_STATUS_1);
   macSpiReadReg(SI4432_INTERRUPT_STATUS_2);

   STOP_MAC_TIMER();                // stop Timer
   CLEAR_MAC_TIMER_INTERRUPT();     // clear flag
   CLEAR_MAC_EXT_INTERRUPT();
#ifndef B1_ONLY 
	if (EZMacProReg.name.DTR == 0x00) //if the rev V2 chip is used
 		// this register setting is need for good current consumption in Idle mode (only rev V2)
   	macSpiWriteReg (SI4432_CRYSTAL_OSCILLATOR_CONTROL_TEST, SI4432_BUFOVR);
#endif//B1_ONLY
   // shut-down radio except for xtal
   macSetFunction1(SI4432_XTON);
   macSpiWriteReg (SI4432_OPERATING_AND_FUNCTION_CONTROL_2, 0x00 );


   if (EZMacProReg.name.MSR & (TX_STATE_BIT|RX_STATE_BIT))
   {//if the ongoing reception and transmission aborts
		// the next state is IDLE state	
      EZMacProReg.name.MSR = EZMAC_PRO_IDLE;
      return STATE_ERROR;
   }
   else
   {
		// the next state is IDLE state
      EZMacProReg.name.MSR = EZMAC_PRO_IDLE;
      return MAC_OK;
   }
}
//------------------------------------------------------------------------------------------------
// Function Name:	EZMacPRO_Transmit()
//						It starts to transmit a packet.All the parameters have to be set before calling 
//						this function.	EZMAC PRO has to be in IDLE mode when calling this function.
//
// Return Values: MAC_OK: the transmission started correctly.
//						STATE_ERROR: the operation ignored (the transmission has not been started), because the
//						EZMAC PRO was not in IDLE mode.
//------------------------------------------------------------------------------------------------
#ifndef RECEIVER_ONLY_OPERATION
MacParams EZMacPRO_Transmit(void)
{

   U8 temp8;
	//if the MAC is not in Idle state
   if (EZMacProReg.name.MSR != EZMAC_PRO_IDLE)
         return STATE_ERROR;

   DISABLE_MAC_INTERRUPTS();        // clear EX0 & ET0
#ifndef B1_ONLY 
	if(EZMacProReg.name.DTR == 0) //if the rev V2 chip is used
 	{
   	//this register setting is needed (only rev V2)
   	macSpiWriteReg (SI4432_CRYSTAL_OSCILLATOR_CONTROL_TEST, 0x24);
	   //set the TX deviation (only rev V2)
	   macSpiWriteReg (SI4432_FREQUENCY_DEVIATION, TX_Freq_dev);
	}
#endif//B1_ONLY
#ifdef ANTENNA_DIVERSITY_ENABLED
#ifndef B1_ONLY
	//if revision V2 or A0 chip is used 
	if ((EZMacProReg.name.DTR == 0x00) || (EZMacProReg.name.DTR == 0x01))
  	{
		
		//switch OFF the internal algorithm
		temp8 = macSpiReadReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2);
	   macSpiWriteReg (SI4432_OPERATING_AND_FUNCTION_CONTROL_2, temp8 & 0x7F);
		
  		//select the TX antenna
   	if ( Selected_Antenna == 1 )
   	{
   	   //select antenna 1
   	   macSpiWriteReg(SI4432_GPIO1_CONFIGURATION, 0x1D);
   	   macSpiWriteReg(SI4432_GPIO2_CONFIGURATION, 0x1F);
   	}
   	else
   	{
   	   //select antenna 2
     		macSpiWriteReg(SI4432_GPIO1_CONFIGURATION, 0x1F);
     	 	macSpiWriteReg(SI4432_GPIO2_CONFIGURATION, 0x1D);
   	}
	}
#endif//B1_ONLY
#endif//ANTENNA_DIVERSITY_ENABLED

   //select the TX frequency
#ifdef FOUR_CHANNEL_IS_USED
   //if AFCH bit is cleared in the Transmit Control Register
   if( (EZMacProReg.name.TCR & 0x04) == 0x04)
   {
      //select the first frequency channel according to Frequency Register 0
      macSpiWriteReg (SI4432_FREQUENCY_HOPPING_CHANNEL_SELECT,EZMacProReg.name.FR0);
      EZMacProCurrentChannel = 0;
   }
	else
	{
      //select the proper frequency register according to FSR register
      temp8 = EZMacProReg.name.FSR;
  	   //in case of four channel is only for channel is allowed
		if(temp8>3) temp8 = 0;
      macSpiWriteReg (SI4432_FREQUENCY_HOPPING_CHANNEL_SELECT,EZMacProReg.array[FR0+temp8]);
      EZMacProCurrentChannel = temp8;
	}
#endif//FOUR_CHANNEL_IS_USED
#ifdef MORE_CHANNEL_IS_USED
      //select the proper frequency register according to FSR register
      temp8 = EZMacProReg.name.FSR;
      macSpiWriteReg (SI4432_FREQUENCY_HOPPING_CHANNEL_SELECT,EZMacProReg.array[FR0+temp8]);
      EZMacProCurrentChannel = temp8;
#endif//MORE_CHANNEL_IS_USED

#ifdef EXTENDED_PACKET_FORMAT
   //Assemble the CTRL byte
	//set radius field
   temp8 = (EZMacProReg.name.MCR >>3) & 0x03;                          
   if ( (EZMacProReg.name.TCR & 0x80) == 0x80 )
      temp8 |= 0x04;           //ACK request
	//set the Sequence number
   if ( EZMacProSequenceNumber < 15)
      EZMacProSequenceNumber++;
   else
      EZMacProSequenceNumber = 0;

   temp8 |= (EZMacProSequenceNumber<<4);

   // set the transmit headers
	if (( EZMacProReg.name.MCR & 0x80 ) == 0x80)//if CID is used
	{
		macSpiWriteReg(SI4432_TRANSMIT_HEADER_3, temp8); // CTRL
      macSpiWriteReg(SI4432_TRANSMIT_HEADER_2, EZMacProReg.name.SCID);
      macSpiWriteReg(SI4432_TRANSMIT_HEADER_1, EZMacProReg.name.SFID);
      macSpiWriteReg(SI4432_TRANSMIT_HEADER_0, EZMacProReg.name.DID);
	}
	else
	{//if CID is not used
      macSpiWriteReg(SI4432_TRANSMIT_HEADER_3, temp8); // CTRL
      macSpiWriteReg(SI4432_TRANSMIT_HEADER_2, EZMacProReg.name.SFID);
      macSpiWriteReg(SI4432_TRANSMIT_HEADER_1, EZMacProReg.name.DID);
	}
#endif   // EXTENDED_PACKET_FORMAT



#ifdef TRANSCIEVER_OPERATION
   if(((EZMacProReg.name.TCR & 0x08)==0x08) && ((EZMacProReg.name.TCR & 0x04)==0x00))// LBT enabled and the AFCH is not enabled
   {
      //Set Listen Before Talk Limit to RSSI threshold register
      macSpiWriteReg(SI4432_RSSI_THRESHOLD, EZMacProReg.name.LBTLR);

      // clear interrupts
      macSpiReadReg(SI4432_INTERRUPT_STATUS_1);
      macSpiReadReg(SI4432_INTERRUPT_STATUS_2);


      // disable all Si443x interrupt enable 1 sources
      macSpiWriteReg(SI4432_INTERRUPT_ENABLE_1, 0x00);
      // enable RSSI interrupt
      macSetEnable2(SI4432_ENRSSI);
		//clear the LBT variable
      EZMacProLBT_Retrys = 0;
      BusyLBT = 0;
		// go to next state
      EZMacProReg.name.MSR = TX_STATE_BIT | TX_STATE_LBT_START_LISTEN;

      //first timeout is fix 0.5ms
		// start timer with LBT ETSI fix timeout
      macTimeout(TIMEOUT_LBTI_ETSI);
      // enable RX
      macSetFunction1 (SI4432_RXON|SI4432_XTON);
      ENABLE_MAC_INTERRUPTS();
      return MAC_OK;
   }
#endif //TRANSCIEVER_OPERATION
   // go straight to transmit without LBT
   // enable ENPKSENT bit
   macSpiWriteReg (SI4432_INTERRUPT_ENABLE_1, SI4432_ENPKSENT);
   // disable enable 2 using function
   macSetEnable2 (0x00);
   // clear interrupt status
   macSpiReadReg(SI4432_INTERRUPT_STATUS_1);
   macSpiReadReg(SI4432_INTERRUPT_STATUS_2);
   // enable TX
   macSetFunction1 (SI4432_TXON|SI4432_XTON);
	// start timer with packet transmit timeout
   macTimeout(TimeoutTX_Packet);
   EZMacProReg.name.MSR = TX_STATE_BIT | TX_STATE_WAIT_FOR_TX;
   ENABLE_MAC_INTERRUPTS();

   return MAC_OK;
}
#endif // RECEIVER_ONLY_OPERATION not defined
//------------------------------------------------------------------------------------------------
// Function Name : EZMacPRO_Receive()
//						 It starts searching for a new packet on the defined frequencies. If the receiver finds
//						 RF activity on a channel, it tries to receive and process the packet. 
//					 	 The search can be stopped by the EZMacPRO_Idle() function.
//						 All the parameters have to be set before calling this function. 
//						 EZMAC PRO has to be in IDLE mode when calling this function.
//
// Return Values : MAC_OK: the search mechanism started correctly.
//						 STATE_ERROR: the operation is ignored (the search mechanism has not been started) because the
//										  EZMAC PRO was not in IDLE mode.
//-----------------------------------------------------------------------------------------------
#ifndef TRANSMITTER_ONLY_OPERATION
MacParams EZMacPRO_Receive(void)
{
   U8 temp8;

   if (EZMacProReg.name.MSR != EZMAC_PRO_IDLE)
         return STATE_ERROR;
   else
   {
      DISABLE_MAC_INTERRUPTS();        // just in case
#ifndef B1_ONLY
		if(EZMacProReg.name.DTR == 0) //if the revV2 chip is used
	 	{
	      //this register setting is needed (only rev V2)
   	   macSpiWriteReg (SI4432_CRYSTAL_OSCILLATOR_CONTROL_TEST, 0x24);
   	   //set the RX deviation (only rev V2)
   	   macSpiWriteReg (SI4432_FREQUENCY_DEVIATION, RX_Freq_dev);
		}
#endif//B1_ONLY

#ifdef FOUR_CHANNEL_IS_USED
      //Frequency search mechanism is disabled
      if((EZMacProReg.name.RCR & 0x04)== 0x00)
      {
		   //select the proper frequency register according to FSR register
	      temp8 = EZMacProReg.name.FSR;
   	   if(temp8>3) temp8 = 0;
			macSpiWriteReg (SI4432_FREQUENCY_HOPPING_CHANNEL_SELECT,EZMacProReg.array[FR0+temp8]);
   	   EZMacProCurrentChannel = temp8;
      }
      //Frequency search mechanism is enabled
      else
      {
         //select the first not masked frequency
         if ((EZMacProReg.name.RCR & 0x08) != 0x08) macSpiWriteReg (SI4432_FREQUENCY_HOPPING_CHANNEL_SELECT,EZMacProReg.name.FR0);
         else if ((EZMacProReg.name.RCR & 0x10) != 0x10) macSpiWriteReg (SI4432_FREQUENCY_HOPPING_CHANNEL_SELECT,EZMacProReg.name.FR1);
         else if ((EZMacProReg.name.RCR & 0x20) != 0x20) macSpiWriteReg (SI4432_FREQUENCY_HOPPING_CHANNEL_SELECT,EZMacProReg.name.FR2);
         else if ((EZMacProReg.name.RCR & 0x40) != 0x40) macSpiWriteReg (SI4432_FREQUENCY_HOPPING_CHANNEL_SELECT,EZMacProReg.name.FR3);
      }
#endif //FOUR_CHANNEL_IS_USED

      //disable Interrupt Enable 1
      macSpiWriteReg (SI4432_INTERRUPT_ENABLE_1, 0x00);

      //enable preamble valid interrupt
      macSetEnable2(SI4432_ENPREAVAL);

#ifdef MORE_CHANNEL_IS_USED
		SelectedChannel = 0;
		macSpiWriteReg(SI4432_FREQUENCY_HOPPING_CHANNEL_SELECT,EZMacProReg.array[FR0+SelectedChannel]);
#endif //MORE_CHANNEL_IS_USED

      // clear interrupt status
      macSpiReadReg(SI4432_INTERRUPT_STATUS_1);
      macSpiReadReg(SI4432_INTERRUPT_STATUS_2);

      // clear RX FIFO
      temp8 = macSpiReadReg (SI4432_OPERATING_AND_FUNCTION_CONTROL_2);
      temp8 |= SI4432_FFCLRRX;
      macSpiWriteReg (SI4432_OPERATING_AND_FUNCTION_CONTROL_2, temp8);
      temp8 &= ~SI4432_FFCLRRX;
      macSpiWriteReg (SI4432_OPERATING_AND_FUNCTION_CONTROL_2, temp8);

      // enable RX
      macSetFunction1(SI4432_RXON|SI4432_XTON);
		// start timer with channel search timeout
		macTimeout(TimeoutChannelSearch);
		// go to next state
      EZMacProReg.name.MSR = RX_STATE_BIT | RX_STATE_FREQUENCY_SEARCH;
      ENABLE_MAC_INTERRUPTS();

      return MAC_OK;

   }
}
#endif // TRANSMITTER_ONLY_OPERATION
//------------------------------------------------------------------------------------------------
// Function Name:    EZMacPRO_Reg_Write
//							Writes the value into the register identified by name. MacRegs type is predefined; 
//							the names of the available registers are listed in the Register Assignment section. 
//							When required, this function also updates the radio register setting directly.
//							This function is also available to be called during SLEEP mode.
//							None of the registers can be set during packet transmission or reception!
//
// Return Values:		MAC_OK: The register is set correctly.
//							NAME_ERROR: The register name is unknown.
//							VALUE_ERROR: The value is �out of the range: for this register.
//							STATE_ERROR: The requested operation is currently unavailable and ignored because either
//											transmission or reception is currently in progress. 
//							INCONSISTENT_SETTING: If the data rate is changed and the current Frequency ID
//														is not supported by the new data rate, the value of the
//														inconsistent Frequency ID is automatically changed to a 0. 
//														In this case, the function returns with 'INCONSISTENT_SETTING'.
//
// Parameters   :    name: MAC register name
//             		value: MAC register value
//-----------------------------------------------------------------------------------------------
MacParams EZMacPRO_Reg_Write(MacRegs  name, U8 value)
{
   U8 temp8;

   // register name check
      if (name>EZ_LASTREG)
         return NAME_ERROR;

   // state check
   if (EZMacProReg.name.MSR & (TX_STATE_BIT|RX_STATE_BIT))
      return STATE_ERROR;

   switch (name)
   {
	   // order must match enumerations
      // mandatory elements listed first
		case    MCR:                          // Master Control Register
			//Set the header length according to CID control bit
#ifdef EXTENDED_PACKET_FORMAT
   		if (( value & 0x80 ) == 0x80)//if CID is used 
      		//header length 4 byte(CTRL+CID+SID+DID)
 			   macSpiWriteReg(SI4432_HEADER_CONTROL_2, SI4432_HDLEN_4BYTE | (SYNC_WORD_LENGTH - 1) << 1);
   		else
      		//header length 3 byte(CTRL+SID+DID)
      		macSpiWriteReg(SI4432_HEADER_CONTROL_2, SI4432_HDLEN_3BYTE | (SYNC_WORD_LENGTH - 1 ) << 1);
#else //STANDARD_EZMAC_PACKET_FORMAT
   		if (( value & 0x80 ) == 0x80)//if CID is used 
      		//header length 3 byte(CID+SID+DID)
      		macSpiWriteReg(SI4432_HEADER_CONTROL_2, SI4432_HDLEN_3BYTE | (SYNC_WORD_LENGTH - 1) << 1);
   		else
      		//header length 2 byte(SID+DID)
      		macSpiWriteReg(SI4432_HEADER_CONTROL_2, SI4432_HDLEN_2BYTE | (SYNC_WORD_LENGTH - 1) << 1);
#endif//EXTENDED_PACKET_FORMAT

         //Master Control Register(set modem, frequency parameters and preamble length according to the data rate)
         SetRfParameters( value );
         macUpdateDynamicTimeouts(value, EZMacProReg.name.MPL);

#ifdef FOUR_CHANNEL_IS_USED 
			//in case of four channel, check the frequency number is correct or not
	      temp8 = (value & 0x60) >> 5;
         if ( EZMacProReg.name.FR0 >= Parameters[temp8][MAX_CHANNEL_NUMBER])
         {
            EZMacProReg.name.FR0 = 0;
            return INCONSISTENT_SETTING;
         }
         if ( EZMacProReg.name.FR1 >= Parameters[temp8][MAX_CHANNEL_NUMBER])
         {
            EZMacProReg.name.FR1 = 0;
            return INCONSISTENT_SETTING;
         }
         if ( EZMacProReg.name.FR2 >= Parameters[temp8][MAX_CHANNEL_NUMBER])
         {
            EZMacProReg.name.FR2 = 0;
            return INCONSISTENT_SETTING;
         }

         if ( EZMacProReg.name.FR3 >= Parameters[temp8][MAX_CHANNEL_NUMBER])
         {
            EZMacProReg.name.FR3 = 0;
            return INCONSISTENT_SETTING;
         }
#endif //FOUR_CHANNEL_IS_USED
#ifdef TRANCIEVER_OPERATION
         macUpdateLBTI(EZMacProReg.name.LBTIR);
#endif //TRANCIEVER_OPERATION
         //static packet length used
         if (( value & 0x04 ) == 0x00)
         {
            temp8 = macSpiReadReg(SI4432_HEADER_CONTROL_2);
            macSpiWriteReg(SI4432_HEADER_CONTROL_2, temp8|SI4432_FIXPKLEN);
         }

         break;
      case    SECR:                         // State & Error Counter Control Register
         break;
      case    TCR:                          // Transmit Control Register
         //Transmit Control Register(set output power of the radio
   		temp8 = value >> 4;
#ifndef B1_ONLY
			if(EZMacProReg.name.DTR == 0) //check the device typ
		         macSpiWriteReg(SI4432_TX_POWER,temp8); // revV2
			else if(EZMacProReg.name.DTR == 1) 
		         macSpiWriteReg(SI4432_TX_POWER,(temp8|0x08)); //revA0
			else if(EZMacProReg.name.DTR == 2)
					macSpiWriteReg(SI4432_TX_POWER,(temp8|0x18)); //revB1
#endif//B1_ONLY
#ifdef B1_ONLY
			macSpiWriteReg(SI4432_TX_POWER,(temp8|0x18)); //revB1
#endif//B1_ONLY
		break;
         break;
      case    RCR:                          // Receiver Control Register
      //Receive Control Register
         if ((value & 0x78) == 0x78)
            return VALUE_ERROR;
         break;
      case    FR0:                          // Frequency Register 0
         //break;
      case    FR1:                          // Frequency Register 1
         //break;
      case    FR2:                          // Frequency Register 2
         //break;
      case    FR3:                          // Frequency Register 3
         //break;
#ifdef FOUR_CHANNEL_IS_USED
         //Frequency registers(check the setting value is correct)
         // /get the setting data rate
         temp8 = (EZMacProReg.name.MCR>>5)&0x03;
         //check the enabled channel number
         if (value >=  Parameters[temp8][MAX_CHANNEL_NUMBER])
            return VALUE_ERROR;
			break;
#endif //FOUR_CHANNEL_IS_USED

#ifdef MORE_CHANNEL_IS_USED
      case    FR4:                          // Frequency Register 4
         //break;
      case    FR5:                          // Frequency Register 5
         //break;
      case    FR6:                          // Frequency Register 6
         //break;
      case    FR7:                          // Frequency Register 7
			//break;
      case    FR8:                          // Frequency Register 8
         //break;
      case    FR9:                          // Frequency Register 9
         //break;
      case    FR10:                         // Frequency Register 10
         //break;
      case    FR11:                         // Frequency Register 11
			//break;
      case    FR12:                         // Frequency Register 12
         //break;
      case    FR13:                          // Frequency Register 13
         //break;
      case    FR14:                          // Frequency Register 14
         //break;
      case    FR15:                          // Frequency Register 15
			//break;
      case    FR16:                          // Frequency Register 16
         //break;
      case    FR17:                          // Frequency Register 17
         //break;
      case    FR18:                          // Frequency Register 18
         //break;
      case    FR19:                          // Frequency Register 19
			//break;
      case    FR20:                          // Frequency Register 20
         //break;
      case    FR21:                          // Frequency Register 21
         //break;
      case    FR22:                          // Frequency Register 22
         //break;
      case    FR23:                          // Frequency Register 23
			//break;
      case    FR24:                          // Frequency Register 24
         //break;
      case    FR25:                          // Frequency Register 25
         //break;
      case    FR26:                          // Frequency Register 26
         //break;
      case    FR27:                          // Frequency Register 27
			//break;
      case    FR28:                          // Frequency Register 28
         //break;
      case    FR29:                          // Frequency Register 29
         //break;
      case    FR30:                          // Frequency Register 30
         //break;
      case    FR31:                          // Frequency Register 31
			//break;
      case    FR32:                          // Frequency Register 32
         //break;
      case    FR33:                          // Frequency Register 33
         //break;
      case    FR34:                          // Frequency Register 34
         //break;
      case    FR35:                          // Frequency Register 35
			//break;
      case    FR36:                          // Frequency Register 36
         //break;
      case    FR37:                          // Frequency Register 37
         //break;
      case    FR38:                          // Frequency Register 38
         //break;
      case    FR39:                          // Frequency Register 39
			//break;
      case    FR40:                          // Frequency Register 40
         //break;
      case    FR41:                          // Frequency Register 41
         //break;
      case    FR42:                          // Frequency Register 42
         //break;
      case    FR43:                          // Frequency Register 43
			//break;
      case    FR44:                          // Frequency Register 44
         //break;
      case    FR45:                          // Frequency Register 45
         //break;
      case    FR46:                          // Frequency Register 46
         //break;
      case    FR47:                          // Frequency Register 47
			//break;
		case	  FR48:									// Frequency Register 48
			//break;
		case	  FR49:									// Frequency Register 49
			break;
#endif //MORE_CHANNEL_IS_USED
		case 	  FSR:									// Frequency Select Register
#ifdef FOUR_CHANNEL_IS_USED
			if(value >= 4)
				return VALUE_ERROR;
#endif
#ifdef MORE_CHANNEL_IS_USED
			temp8 = (EZMacProReg.name.MCR >> 5) & 0x03;
			if(value >= Parameters[temp8] [5])
				return VALUE_ERROR;
#endif
			break;	
#ifdef FOUR_CHANNEL_IS_USED
		case    EC0:                          // Error Counter of Frequency 0
         break;
      case    EC1:                          // Error Counter of Frequency 1
         break;
      case    EC2:                          // Error Counter of Frequency 2
         break;
      case    EC3:                          // Error Counter of Frequency 3
         break;
#endif //FOUR_CHANNEL_IS_USED
      case    PFCR:                         // Packet Filter Control Register
         break;
      case    SFLT:                         // Sender ID Filter
         break;
      case    SMSK:                         // Sender ID Filter Mask
         break;
      case    MCA_MCM:                      // Multicast Address / Multicast Mask
         break;
      case    MPL:                          // Maximum Packet Length
          if ( value > RECEIVED_BUFFER_SIZE )
            return VALUE_ERROR;
         else
            macUpdateDynamicTimeouts(EZMacProReg.name.MCR, value);          // max payload always updates timouts
         break;
      case    MSR:                          // MAC Status Register
         //MAC Status Register is read only
         return NAME_ERROR;
         break;
      case    RSR:                          // Receive Status Register
         //Receive Status Register is read only
         return NAME_ERROR;
         break;
		case	  RFSR:								// Received Frequency Status Register
	      //Received Frequency Status Register is read only
         return NAME_ERROR;
         break;
	 	
      case    RSSI:                         // Received Signal Strength Indicator
         //Received Signal Strength Indicator is read  only
         return NAME_ERROR;
         break;
      case    SCID :                        // Self Customer ID
			if (( EZMacProReg.name.MCR & 0x80 ) == 0x80)//if CID is used 	
         	//save the SCID to the Transmit Header3 register of the radio
         	macSpiWriteReg(SI4432_TRANSMIT_HEADER_3, value);
         else
				return NAME_ERROR;
         break;
      case    SFID:                         // Self ID
         if (value == 255)
            return VALUE_ERROR;
			if (( EZMacProReg.name.MCR & 0x80 ) == 0x80)//if CID is used 
         	//save the SFID to the Transmit Header2 register of the radio
         	macSpiWriteReg(SI4432_TRANSMIT_HEADER_2, value);
			else
         	//save the SFID to the Transmit Header3 register of the radio
         	macSpiWriteReg(SI4432_TRANSMIT_HEADER_3, value);
         break;
      case    RCTRL:                        // Received Control Byte
         //Received CTRL read only
         return NAME_ERROR;
         break;
      case    RCID:                         // Received Customer ID
         //Received Customer ID is read only
         return NAME_ERROR;
         break;
      case    RSID:                         // Received Sender ID
         //Received Sender ID is read only
         return NAME_ERROR;
         break;
      case    DID:                          // Destination ID
			if (( EZMacProReg.name.MCR & 0x80 ) == 0x80)//if CID is used 
         	//save the DID to the Transmit Header1 register of the radio
         	macSpiWriteReg(SI4432_TRANSMIT_HEADER_1, value);
			else
         	//save the DID to the Transmit Header2 register of the radio
         	macSpiWriteReg(SI4432_TRANSMIT_HEADER_2, value);
         break;
      case    PLEN:                         // Payload Length
         //set the payload length
         if ( value > RECEIVED_BUFFER_SIZE )
            return VALUE_ERROR;
         else
         {
            macSpiWriteReg(SI4432_TRANSMIT_PACKET_LENGTH,value);   //save the PLEN to the transmit packet length register
         }
         break;

      // optional elemets listed last using compiler switches
#ifdef TRANSCIEVER_OPERATION
      case    LBTIR:                        // Listen Before Talk  Interval Register
         macUpdateLBTI(value);
         break;
      case    LBTLR:                        // Listen Before Talk Limit Register
         break;
#endif //TRANCIEVER_OPERATION
   case    LFTMR0:                       // Low Frequency Timer Setting Register 0
      //Low Frequency Timer Setting Register0
      macSpiWriteReg(SI4432_WAKE_UP_TIMER_PERIOD_3, value); //Set Wake-Up-Timer Mantissa
      break;
   case    LFTMR1:                       // Low Frequency Timer Setting Register 1
      //Low Frequency Timer Setting Register1
      macSpiWriteReg(SI4432_WAKE_UP_TIMER_PERIOD_2, value); //Set Wake-Up_Timer Mantissa
      break;
   case    LFTMR2:                       // Low Frequency Timer Setting Register 2
       //Set Wake-Up_timer Exponent
       macSpiWriteReg(SI4432_WAKE_UP_TIMER_PERIOD_1, value & 0x3F);
      if ((value & 0x80) == 0x80)//if the Wake-Up-Timer is enabled
      {
         temp8 = macSpiReadReg(SI4432_INTERRUPT_ENABLE_2);
         temp8 |= SI4432_ENWUT;
         macSpiWriteReg(SI4432_INTERRUPT_ENABLE_2, temp8);

         temp8 = macSpiReadReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_1);
         temp8 |= SI4432_ENWT;
         if ((value & 0x40) == 0x00)
         {
            temp8 |= SI4432_X32KSEL;
         }
         macSpiWriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_1, temp8);
         ENABLE_MAC_EXT_INTERRUPT();
      }
		else
		{
         temp8 = macSpiReadReg(SI4432_INTERRUPT_ENABLE_2);
         temp8 &= ~SI4432_ENWUT;
         macSpiWriteReg(SI4432_INTERRUPT_ENABLE_2, temp8);

         temp8 = macSpiReadReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_1);
         temp8 &= ~SI4432_ENWT;
         macSpiWriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_1, temp8);
		}
      break;
   case    LBDR:                         // Low Battery Detect Register
      if ((value & 0x80) == 0x80)   //if enalbe the low battery detect
      {
			temp8 = value & 0x1F;
			macSpiWriteReg(SI4432_LOW_BATTERY_DETECTOR_THRESHOLD, temp8);     //set battery voltage threshold
         
			temp8 = macSpiReadReg(SI4432_INTERRUPT_ENABLE_2);
         macSpiWriteReg(SI4432_INTERRUPT_ENABLE_2, temp8 | SI4432_ENLBDI);    //enable the low battery interrupt
			
			temp8 = macSpiReadReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_1);
         macSpiWriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_1, temp8|SI4432_ENLBD);
      }
		else
		{
			temp8 = macSpiReadReg(SI4432_INTERRUPT_ENABLE_2);
         macSpiWriteReg(SI4432_INTERRUPT_ENABLE_2, temp8 & ~SI4432_ENLBDI);    //disable the low battery interrupt
			
			temp8 = macSpiReadReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_1);
         macSpiWriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_1, temp8 & ~SI4432_ENLBD); //disable the low battery detect
		}
      break;
   case    ADCTSR:                       // ADC and Temperature Sensor Register
            //set ADC configuration register
      macSpiWriteReg(SI4432_ADC_CONFIGURATION, (value & 0xFC) | EZMACPRO_ADC_GAIN);

		//set ADC sensor Amplifier register(EZMACPRO_ADC_AMP_OFFSET is definition in EZMacPro_defs.h)
      macSpiWriteReg(SI4432_ADC_SENSOR_AMPLIFIER_OFFSET, EZMACPRO_ADC_AMP_OFFSET);
      //set the Temperature Sensor range
      macSpiWriteReg(SI4432_TEMPERATURE_SENSOR_CONTROL, ((value & 0x03)<<6)|0x20);
      break;

   case    ADCTSV:                       // ADC/Temperature Value Register
      return NAME_ERROR;
      break;
	case	  DTR:
#ifndef B1_ONLY
		if (value > 2)
			return VALUE_ERROR;
		macSpecialRegisterSettings(value);
#endif//B1_ONLY
		break;	

   }
   //Register update
   EZMacProReg.array[name] = value;
   return MAC_OK;

}
//------------------------------------------------------------------------------------------------
// Function Name:    EZMacPRO_Reg_Read
// 						Gives back the value (over value pointer) of the register identified by name. 
//							MacRegs type is predefined. This function may also be called in SLEEP mode. 
// Return Value :    MAC_OK: The operation was succesfull
//             		NAME_ERROR: The register name is invalid
//
// Parameters   :    name: register name
//             		value: register value
//
//
//-----------------------------------------------------------------------------------------------
MacParams EZMacPRO_Reg_Read (MacRegs name, U8 *value)
{
	
	// register name check
   if (name>EZ_LASTREG)
      return NAME_ERROR;

   //this 3 registers are write only
   if ((name == LFTMR0) || (name == LFTMR1) ||(name == LFTMR2))
   {
      return NAME_ERROR;
   }

   if (name == RSSI)
      // state check
      if ((EZMacProReg.name.MSR & RX_STATE_BIT)==RX_STATE_BIT)
         //if the MAC in these state then the RSSI will be read from the chip
         EZMacProReg.name.RSSI = macSpiReadReg(SI4432_RECEIVED_SIGNAL_STRENGTH_INDICATOR);

	
	if (name == ADCTSV)
	{
   	EZMacProReg.name.ADCTSV = macSpiReadReg(SI4432_ADC_VALUE);
	}

	if (name == LBDR)
		if ((EZMacProReg.name.LBDR & 0x80) == 0x80)
		{	
			EZMacProReg.name.LBDR &= 0x80;
			EZMacProReg.name.LBDR |= macSpiReadReg(SI4432_BATTERY_VOLTAGE_LEVEL);

		}
		else EZMacProReg.name.LBDR = 0x00;

		
  // gives back the register content
  *value = EZMacProReg.array[name];
  return MAC_OK;
}
//------------------------------------------------------------------------------------------------
// Function Name: EZMacPRO_TxBuf_Write
//						The function copies length number of payload bytes into the transmit FIFO of the radio chip.
//						There is no dedicated transmit buffer in the source code.Upon calling this function,
//						it clears the TX FIFO of the radio first.If variable packet length is used and
//						the length is not greater than the RECEIVED_BUFFER_SIZE definition 
//						(it cannot be greater than 64), then EZMAC PRO copies length number of payload
//						bytes into the TX FIFO of the radio, sets the PLEN register of EZMAC PRO,
//						and also sets the packet length register of the radio. If fix packet length is used,
//						then the PLEN register has to set first, because the function copies only
//						Payload Length number of bytes into the FIFO even if the length is greater.
//						Also it fills the FIFO with extra 0x00 bytes if the length is smaller than
//						the value of the Payload Length register in fix packet length mode.
//						The function cannot be called during transmission and reception.
//
// Return Values: MAC_OK: The operation performed correctly.
//						STATE_ERROR: The operation is ignored because transmission and reception are in progress.
//
// Parameters   : length: payload length
//             	buffer: payload content
//
//-----------------------------------------------------------------------------------------------
#ifndef RECEIVER_ONLY_OPERATION
MacParams EZMacPRO_TxBuf_Write(U8 length, VARIABLE_SEGMENT_POINTER(payload, U8, BUFFER_MSPACE))
{
   U8 temp8;

   // state check
   if (EZMacProReg.name.MSR & (TX_STATE_BIT|RX_STATE_BIT))
      return STATE_ERROR;

   // if the given packet is bigger then the RECEIVED_BUFFER_SIZE
   if ( length > RECEIVED_BUFFER_SIZE )
      return VALUE_ERROR;


   // clear RX FIFO
   temp8 = macSpiReadReg (SI4432_OPERATING_AND_FUNCTION_CONTROL_2);
   temp8 |= SI4432_FFCLRTX;
   macSpiWriteReg (SI4432_OPERATING_AND_FUNCTION_CONTROL_2, temp8);
   temp8 &= ~SI4432_FFCLRTX;
   macSpiWriteReg (SI4432_OPERATING_AND_FUNCTION_CONTROL_2, temp8);


   //if dynamic payload length mode is set
   if ((EZMacProReg.name.MCR & 0x04) == 0x04)
   {

      //set the transmit packet length
      macSpiWriteReg(SI4432_TRANSMIT_PACKET_LENGTH, length);
      EZMacProReg.name.PLEN = length;
      //set the payload content
      macSpiWriteFIFO(length,payload);
   }
   //if static payload length mode is set
   else
   {

      if (length < EZMacProReg.name.PLEN) //if payload length smaller than the fix payload length
      {
         //set the payload content
         macSpiWriteFIFO(length,payload);

         //fill the remain payload bytes with zero
         for(temp8=length;temp8 < EZMacProReg.name.PLEN;temp8++)
            macSpiWriteReg(SI4432_FIFO_ACCESS,0x00);

      }
      else  //if the payload length equal or bigger than the fix payload length
      {
         macSpiWriteFIFO(EZMacProReg.name.PLEN,payload);
      }


   }
   return MAC_OK;
}
#endif //RECEIVER_ONLY_OPERATION not defined
//------------------------------------------------------------------------------------------------
// Function Name: EZMacPRO_RxBuf_Read
//						After a successful packet reception EZMAC PRO copies the received data bytes into
//						the receive data buffer. The receive data buffer is declared in the EZMacPro.c file as
//						SEGMENT_VARIABLE(RxBuffer[RECEIVED_BUFFER_SIZE], U8, BUFFER_MSPACE);
//						The length of the receive buffer is defined by the RECEIVED_BUFFER_SIZE definition in the
//						EZMacPro_defs.h. It can be adjusted for the application needs, but it can not be greater than 64bytes.
//						The receive buffer is declared to be placed into the XDATA memory, it also can be adjusted by
//						changing the BUFFER_MSPACE definition. Upon calling the EZMacPRO_RxBuf_Read() function, it
//						copies received data from the receive data buffer to payload. Also it gives back the number of
//						received bytes by length.
//						The function cannot be called during transmission and reception.
//
// Return Values: MAC_OK: The operation was succesfull
//             	STATE_ERROR: The operation is ignored, because reception or transmission is ongoing.
//
// Parameters   : length: received payload length
//            		payload: received payload content
//
//-----------------------------------------------------------------------------------------------
#ifndef TRANSMITTER_ONLY_OPERATION
MacParams EZMacPRO_RxBuf_Read(VARIABLE_SEGMENT_POINTER(length, U8, BUFFER_MSPACE), VARIABLE_SEGMENT_POINTER(payload, U8, BUFFER_MSPACE))
{
   U8 temp8 = 0;

	// state check
   if (EZMacProReg.name.MSR & (TX_STATE_BIT|RX_STATE_BIT))
      return STATE_ERROR;

   *length = EZMacProReg.name.PLEN;

   while(temp8 < EZMacProReg.name.PLEN)
   {
      *payload++ = RxBuffer[temp8];
      temp8++;
   }
   return MAC_OK;
}
#endif // TRANSMITTER_ONLY_OPERATION not defined
//================================================================================================
//
// spi Functions for EZMacPro.c module
//
//================================================================================================
//
// Notes:
//
// The spi functions in this module are for use in the main thread. The EZMacPro API calls should
// only be used in the main thread. The SPI is used by the main thread, as well as the external
// interrupt INT0 thread, and the T0 interrupt. Since all SPI tranfers are multiple bytes. It is
// important that MAC interrupts are disabled when using the SPI from the main thread.
//
// These SPI functions may be interrupted by other interrupts, so the double buffered transfers
// are managed with this in mind.
//
// The double buffered transfer maximizes the data throughput and elimiates any software
// delay between bytes. The clock is continuous for the two byte transfer. Instead of using the
// SPIF flag for each byte, the TXBMT is used to keep the transmit buffer full, then the SPIBSY
// bit is used to determine when all bits have been transfered. The SPIF flag should not be
// polled in double buffered transfers.
//
//------------------------------------------------------------------------------------------------
// Function Name: macSpiWriteReg()
//						Write a register of the radio. 
// Return Values: None
// Parameters	 :	U8 reg - register address from the si4432.h file.
//    				U8 value - value to write to register
// Notes:
//
//    MAC interrupts are preserved and restored.
//    Write uses a Double buffered transfer.
//-----------------------------------------------------------------------------------------------
void macSpiWriteReg (U8 reg, U8 value)
{
   U8 restoreInts;

   // Disable MAC interrupts
   restoreInts = IE & 0x03;           // save EX0 & ET0 state
   IE &= ~0x03;                       // clear EX0 & ET0

   // Send SPI data using double buffered write
   NSS = 0;                            // drive NSS low
   SPIF = 0;                           // clear SPIF
   SPI_DAT = (reg | 0x80);             // write reg address
   while(!TXBMT);                      // wait on TXBMT
   SPI_DAT = value;                    // write value
   while(!TXBMT);                      // wait on TXBMT
   while((SPI_CFG & 0x80) == 0x80);    // wait on SPIBSY

   SPIF = 0;                           // leave SPIF cleared
   NSS = 1;                            // drive NSS high

   // Restore MAC interrupts
   IE |= restoreInts;                  // restore EX0 & ET0
}

//------------------------------------------------------------------------------------------------
// Function Name: macSpiReadReg()
//						Read a register from the radio.
//
// Return Value : U8 value - value returned from the si4432 register
// Parameters   : U8 reg - register address from the si4432.h file.
//
//-----------------------------------------------------------------------------------------------
U8 macSpiReadReg (U8 reg)
{
   U8 value;
   U8 restoreInts;

   // Disable MAC interrupts
   restoreInts = IE & 0x03;             // save EX0 & ET0 state
   IE &= ~0x03;                         // clear EX0 & ET0

   // Send SPI data using double buffered write
   NSS = 0;                            // dsrive NSS low
   SPIF = 0;                           // cleat SPIF
   SPI_DAT = ( reg );                  // write reg address
   while(!TXBMT);                      // wait on TXBMT
   SPI_DAT = 0x00;                     // write anything
   while(!TXBMT);                      // wait on TXBMT
   while((SPI_CFG & 0x80) == 0x80);    // wait on SPIBSY
   value = SPI_DAT;                    // read value
   SPIF = 0;                           // leave SPIF cleared
   NSS = 1;                            // drive NSS low

   // Restore MAC interrupts
   IE |= restoreInts;                  // restore EX0 & ET0

   return value;
}

//------------------------------------------------------------------------------------------------
// Function Name: macSpiWriteFIFO()
//						Write the FIFO of the radio.
//
// Return Value : None
// Parameters   :	n - the length of trasnmitted bytes
//						buffer - the transmitted bytes   
//
//-----------------------------------------------------------------------------------------------
#ifndef RECEIVER_ONLY_OPERATION
void macSpiWriteFIFO (U8 n, VARIABLE_SEGMENT_POINTER(buffer, U8, BUFFER_MSPACE))
{
   U8 restoreInts;

   // Disable MAC interrupts
   restoreInts = (IE & 0x03);          // save EX0 & ET0 state
   IE &= ~0x03;                        // clear EX0 & ET0

   NSS = 0;                            // drive NSS low
   SPIF = 0;                           // clear SPIF
   SPI_DAT = (0x80 | SI4432_FIFO_ACCESS);

   while(n--)
   {
      while(!TXBMT);                   // wait on TXBMT
      SPI_DAT = *buffer++;             // write buffer
   }

   while(!TXBMT);                      // wait on TXBMT
   while((SPI_CFG & 0x80) == 0x80);    // wait on SPIBSY

   SPIF = 0;                           // leave SPI  cleared
   NSS = 1;                            // drive NSS high

    // Restore MAC interrupts
    IE |= restoreInts;                  // restore EX0 & ET0
}
#endif


//================================================================================================
// Timer Functions for EZMacPro.c module
//
// Parameters   : U32 longTime
// Notes:
// This function is called when a interrupt event must initiate a timeout event.
// A 32-bit union is used to provide word and byte access. The upper word is stored in
// EZMacProTimerMSB. The The lower word is first negated then written to the TL0 and TH0 sfrs.
//================================================================================================
void macTimeout (U32 longTime)
{
   UU32 time;
   U8 restoreInts;

   // Disable MAC interrupts
   restoreInts = EX0;                  // save EX0 state
   EX0 = 0;                            // clear EX0

   ET0 = 0;                            // disable Timer interrupts
   TR0 = 0;                            // stop timer if already running

   time.U32 = longTime;

   EZMacProTimerMSB = time.U16[MSB];

   time.U16[LSB] = - time.U16[LSB];

   TL0 = time.U8[b0];               // write LSB first
   TH0 = time.U8[b1];               // write MSB last

   TF0 = 0;                            // clear flag
   TR0 = 1;                            // run timer

   IE |= restoreInts;                  // restore EX0
}

//------------------------------------------------------------------------------------------------
// Function Name: SetRfParameters
//
// Return Value : None
// Parameters   : mcr: Master Control Register value
//
// Note: Set the modem, frequency parameters and preamble according to data rate
//
//-----------------------------------------------------------------------------------------------
void SetRfParameters(U8 mcr)
{
   U8 dataRate;
   U8 numFreq;
#ifdef MORE_CHANNEL_IS_USED
	U8 temp8;
#endif//MORE_CHANNEL_IS_USED
   dataRate = (mcr >> 5) & 0x03;
   numFreq = mcr & 0x03;
#ifndef B1_ONLY
   //set the modem parameters
	switch(EZMacProReg.name.DTR)
	{
		case 0://rev V2 
			macSpiWriteReg(SI4432_IF_FILTER_BANDWIDTH, RfSettingsV2[dataRate][0] );
		   macSpiWriteReg(SI4432_CLOCK_RECOVERY_OVERSAMPLING_RATIO, RfSettingsV2[dataRate][1]);
		   macSpiWriteReg(SI4432_CLOCK_RECOVERY_OFFSET_2, RfSettingsV2[dataRate][2]);
		   macSpiWriteReg(SI4432_CLOCK_RECOVERY_OFFSET_1, RfSettingsV2[dataRate][3]);
		   macSpiWriteReg(SI4432_CLOCK_RECOVERY_OFFSET_0, RfSettingsV2[dataRate][4]);
		   macSpiWriteReg(SI4432_CLOCK_RECOVERY_TIMING_LOOP_GAIN_1, RfSettingsV2[dataRate][5]);
		   macSpiWriteReg(SI4432_CLOCK_RECOVERY_TIMING_LOOP_GAIN_0, RfSettingsV2[dataRate][6]);
		   macSpiWriteReg(SI4432_TX_DATA_RATE_1, RfSettingsV2[dataRate][7]);
		   macSpiWriteReg(SI4432_TX_DATA_RATE_0, RfSettingsV2[dataRate][8]);
		   macSpiWriteReg(SI4432_MODULATION_MODE_CONTROL_1,RfSettingsV2[dataRate][9]);
			macSpiWriteReg(SI4432_FREQUENCY_DEVIATION,RfSettingsV2[dataRate][10]);
	   	macSpiWriteReg(SI4432_AFC_LOOP_GEARSHIFT_OVERRIDE, RfSettingsV2[dataRate][12]);
	   	macSpiWriteReg(SI4432_CHARGEPUMP_CURRENT_TRIMMING_OVERRIDE, RfSettingsV2[dataRate][13]);
#ifndef RECEIVER_ONLY_OPERATION
	   	TX_Freq_dev = RfSettingsV2[dataRate][10];
#endif//RECEIVER_ONLY_OPERATION
#ifndef TRANSMITTER_ONLY_OPERATION
			RX_Freq_dev = RfSettingsV2[dataRate][11];
#endif//TRANSMITTER_ONLY_OPERATION
	   	macSpiWriteReg(SI4432_PREAMBLE_DETECTION_CONTROL, PREAMBLE_DETECTION_THRESHOLD<<3); 
			break;	
		case 1://rev A0
		   macSpiWriteReg(SI4432_IF_FILTER_BANDWIDTH, RfSettingsA0[dataRate][0] );
	   	macSpiWriteReg(SI4432_CLOCK_RECOVERY_OVERSAMPLING_RATIO, RfSettingsA0[dataRate][1]);
	   	macSpiWriteReg(SI4432_CLOCK_RECOVERY_OFFSET_2, RfSettingsA0[dataRate][2]);
		   macSpiWriteReg(SI4432_CLOCK_RECOVERY_OFFSET_1, RfSettingsA0[dataRate][3]);
		   macSpiWriteReg(SI4432_CLOCK_RECOVERY_OFFSET_0, RfSettingsA0[dataRate][4]);
		   macSpiWriteReg(SI4432_CLOCK_RECOVERY_TIMING_LOOP_GAIN_1, RfSettingsA0[dataRate][5]);
		   macSpiWriteReg(SI4432_CLOCK_RECOVERY_TIMING_LOOP_GAIN_0, RfSettingsA0[dataRate][6]);
		   macSpiWriteReg(SI4432_TX_DATA_RATE_1, RfSettingsA0[dataRate][7]);
		   macSpiWriteReg(SI4432_TX_DATA_RATE_0, RfSettingsA0[dataRate][8]);
		   macSpiWriteReg(SI4432_MODULATION_MODE_CONTROL_1,RfSettingsA0[dataRate][9]);
			macSpiWriteReg(SI4432_FREQUENCY_DEVIATION,RfSettingsA0[dataRate][10]);
	   	macSpiWriteReg(SI4432_AFC_LOOP_GEARSHIFT_OVERRIDE, RfSettingsA0[dataRate][12]);
	   	macSpiWriteReg(SI4431_AFC_LIMIT, RfSettingsA0[dataRate][13]);
	   	macSpiWriteReg(SI4432_PREAMBLE_DETECTION_CONTROL, (PREAMBLE_DETECTION_THRESHOLD<<3)|0x02);
			break;	
		case 2: //rev B1
		default:
		   macSpiWriteReg(SI4432_IF_FILTER_BANDWIDTH, RfSettingsB1[dataRate][0] );
  			macSpiWriteReg(SI4432_CLOCK_RECOVERY_OVERSAMPLING_RATIO, RfSettingsB1[dataRate][1]);
	  		macSpiWriteReg(SI4432_CLOCK_RECOVERY_OFFSET_2, RfSettingsB1[dataRate][2]);
	   	macSpiWriteReg(SI4432_CLOCK_RECOVERY_OFFSET_1, RfSettingsB1[dataRate][3]);
			macSpiWriteReg(SI4432_CLOCK_RECOVERY_OFFSET_0, RfSettingsB1[dataRate][4]);
	   	macSpiWriteReg(SI4432_CLOCK_RECOVERY_TIMING_LOOP_GAIN_1, RfSettingsB1[dataRate][5]);
	   	macSpiWriteReg(SI4432_CLOCK_RECOVERY_TIMING_LOOP_GAIN_0, RfSettingsB1[dataRate][6]);
	   	macSpiWriteReg(SI4432_TX_DATA_RATE_1, RfSettingsB1[dataRate][7]);
	   	macSpiWriteReg(SI4432_TX_DATA_RATE_0, RfSettingsB1[dataRate][8]);
	   	macSpiWriteReg(SI4432_MODULATION_MODE_CONTROL_1,RfSettingsB1[dataRate][9]);
			macSpiWriteReg(SI4432_FREQUENCY_DEVIATION,RfSettingsB1[dataRate][10]);
   		macSpiWriteReg(SI4432_AFC_LOOP_GEARSHIFT_OVERRIDE, RfSettingsB1[dataRate][12]);
   		macSpiWriteReg(SI4431_AFC_LIMIT, RfSettingsB1[dataRate][13]);
			macSpiWriteReg(SI4432_AFC_TIMING_CONTROL, 0x0A);
	   	macSpiWriteReg(SI4432_PREAMBLE_DETECTION_CONTROL, (PREAMBLE_DETECTION_THRESHOLD<<3)|0x02);
			break;
	}
#endif//B1_ONLY
#ifdef B1_ONLY
   macSpiWriteReg(SI4432_IF_FILTER_BANDWIDTH, RfSettingsB1[dataRate][0] );
	macSpiWriteReg(SI4432_CLOCK_RECOVERY_OVERSAMPLING_RATIO, RfSettingsB1[dataRate][1]);
	macSpiWriteReg(SI4432_CLOCK_RECOVERY_OFFSET_2, RfSettingsB1[dataRate][2]);
  	macSpiWriteReg(SI4432_CLOCK_RECOVERY_OFFSET_1, RfSettingsB1[dataRate][3]);
	macSpiWriteReg(SI4432_CLOCK_RECOVERY_OFFSET_0, RfSettingsB1[dataRate][4]);
  	macSpiWriteReg(SI4432_CLOCK_RECOVERY_TIMING_LOOP_GAIN_1, RfSettingsB1[dataRate][5]);
  	macSpiWriteReg(SI4432_CLOCK_RECOVERY_TIMING_LOOP_GAIN_0, RfSettingsB1[dataRate][6]);
  	macSpiWriteReg(SI4432_TX_DATA_RATE_1, RfSettingsB1[dataRate][7]);
  	macSpiWriteReg(SI4432_TX_DATA_RATE_0, RfSettingsB1[dataRate][8]);
  	macSpiWriteReg(SI4432_MODULATION_MODE_CONTROL_1,RfSettingsB1[dataRate][9]);
	macSpiWriteReg(SI4432_FREQUENCY_DEVIATION,RfSettingsB1[dataRate][10]);
	macSpiWriteReg(SI4432_AFC_LOOP_GEARSHIFT_OVERRIDE, RfSettingsB1[dataRate][12]);
	macSpiWriteReg(SI4431_AFC_LIMIT, RfSettingsB1[dataRate][13]);
	macSpiWriteReg(SI4432_AFC_TIMING_CONTROL, 0x0A);
  	macSpiWriteReg(SI4432_PREAMBLE_DETECTION_CONTROL, (PREAMBLE_DETECTION_THRESHOLD<<3)|0x02);
#endif//B1_ONLY
   //set frequency parameters (center frequency and frequency hopping step size)
   macSpiWriteReg(SI4432_FREQUENCY_BAND_SELECT, Parameters[dataRate][START_FREQUENCY_1]);
   macSpiWriteReg(SI4432_NOMINAL_CARRIER_FREQUENCY_1, Parameters[dataRate][START_FREQUENCY_2]);
   macSpiWriteReg(SI4432_NOMINAL_CARRIER_FREQUENCY_0, Parameters[dataRate][START_FREQUENCY_3]);
   macSpiWriteReg(SI4432_FREQUENCY_HOPPING_STEP_SIZE, Parameters[dataRate][STEP_FREQUENCY]);



   //set preamble length according to number of used channel
#ifdef FOUR_CHANNEL_IS_USED
   if (numFreq == 0)
   {
      //if one channel is used
      macSpiWriteReg(SI4432_PREAMBLE_LENGTH, (Parameters[dataRate][PREAMBLE_IF_ONE_CHANNEL])<<1);
   }
   else if (numFreq == 1)
   {
      //if two channel is used
      macSpiWriteReg(SI4432_PREAMBLE_LENGTH, (Parameters[dataRate][PREAMBLE_IF_TWO_CHANNEL])<<1);
   }
   else if (numFreq == 2)
   {
      //if three channel is used
      macSpiWriteReg(SI4432_PREAMBLE_LENGTH, (Parameters[dataRate][PREAMBLE_IF_THREE_CHANNEL])<<1);
   }
   else // default four channels
   {
      //if four channel is used
      macSpiWriteReg(SI4432_PREAMBLE_LENGTH, (Parameters[dataRate][PREAMBLE_IF_FOUR_CHANNEL])<<1);
   }
#endif//FOUR_CHANNEL_IS_USED
#ifdef MORE_CHANNEL_IS_USED
		maxChannelNumber = Parameters[dataRate][CHANNEL_NUMBERS];
		temp8 = macSpiReadReg(SI4432_HEADER_CONTROL_2);
		macSpiWriteReg(SI4432_HEADER_CONTROL_2, (temp8 | 0x01));
      macSpiWriteReg(SI4432_PREAMBLE_LENGTH, Parameters[dataRate][PREAMBLE_LENGTH_REG_VALUE]);
#endif//MORE_CHANNEL_IS_USED

}

//------------------------------------------------------------------------------------------------
// Function Name: macSpecialRegisterSettings
//
// Return Value : None
// Parameters   : chiptype - Device Type Register value
//
//
//-----------------------------------------------------------------------------------------------
void macSpecialRegisterSettings(U8 chiptype)
{

	switch(chiptype)
	{
#ifndef B1_ONLY
		case 0://rev V2
		   //these settings need for good RF link(V2 specific settings)
		   macSpiWriteReg(SI4432_CLOCK_RECOVERY_GEARSHIFT_OVERRIDE, 0x03);
	  		//set VCO
	   	macSpiWriteReg(SI4432_VCO_CURRENT_TRIMMING, 0x7F);
	   	macSpiWriteReg(SI4432_DIVIDER_CURRENT_TRIMMING, 0x40);
	   	//set the AGC
	   	macSpiWriteReg(SI4432_AGC_OVERRIDE_2, 0x0B);
	   	//set ADC reference voltage to 0.9V
	   	macSpiWriteReg(SI4432_DELTASIGMA_ADC_TUNING_2, 0x04);
		   //set the crystal capacitance bank
	 		macSpiWriteReg(SI4432_CRYSTAL_OSCILLATOR_LOAD_CAPACITANCE, 0xDD);
			break;
		case 1: //rev A0	
		   //set VCO
		   macSpiWriteReg(SI4432_VCO_CURRENT_TRIMMING, 0x01);
		   macSpiWriteReg(SI4432_DIVIDER_CURRENT_TRIMMING, 0x00);

	   	macSpiWriteReg(SI4432_CHARGEPUMP_TEST, 0x01);
	   	//set the Modem test register
	   	macSpiWriteReg(SI4432_MODEM_TEST, 0xC1);   
		   //set the crystal capacitance bank
	 		macSpiWriteReg(SI4432_CRYSTAL_OSCILLATOR_LOAD_CAPACITANCE, 0xDD);
			break;
#endif //B1_ONLY
		case 2:
		default:	//rev B1
			// Set AGC Override1 Register
	   	macSpiWriteReg(SI4432_AGC_OVERRIDE_1, 0x60);
			//set the crystal capacitance bank
			macSpiWriteReg(SI4432_CRYSTAL_OSCILLATOR_LOAD_CAPACITANCE, 0x6D);
			break;
	}
}
//------------------------------------------------------------------------------------------------
// Function Name: macUpdateDynamicTimeouts
//						This function calculate all of the timeouts for the EZMac stack.
//
// Return Value : None
// Parameters   : mcr - Master Control Register value
//						mpl - Maximum Payload size
//------------------------------------------------------------------------------------------------
void macUpdateDynamicTimeouts (U8 mcr, U8 mpl)
{
   U16 n;
   U8 preamble;
   U8 header;

   U16 byteTime;

   // look up byte time
   n = ((mcr >> 5)&0x03);
   byteTime = EZMacProByteTime[n];
	//determine the preamble length
#ifdef FOUR_CHANNEL_IS_USED
   preamble = Parameters[n][PREAMBLE_IF_ONE_CHANNEL + (mcr & 0x03)];
#endif	//FOUR_CHANNEL_IS_USED
#ifdef MORE_CHANNEL_IS_USED
	preamble = Parameters[n][PREAMBLE_LENGTH];
#endif  //MORE_CHANNEL_IS_USED
	//determine the header length
#ifdef EXTENDED_EZMAC_PACKET_FORMAT
	if((mcr & 0x80) == 0x80)			// if CID is used
	   header = 4;                  	// CTRL+CID+SID+DID 
	else
		header = 3;							// CTRL+SID+DID
#else
	if((mcr & 0x80) == 0x80)			// if CID is used
	   header = 3;                  	// CID+SID+DID	
	else
		header = 2;							// SID+DID
#endif//EXTENDED_EZMAC_PACKET_FORMAT

   // if DNPL
   if(( mcr & 0x04) == 0x04)
   {
     header++;                        // add one for length
   }


#ifndef TRANSMITTER_ONLY_OPERATION
   // update preamble timeout (use <<1 instead of *2)
   TimeoutPreamble = (byteTime << 1)<<1;
	// update the sync word timeout
   TimeoutSyncWord = (preamble + 2) * (U32)byteTime;
	// update the channel search timeout	
	TimeoutChannelSearch = byteTime * Parameters[n][SEARCH_TIME];
   // update header timeout
	
	if((mcr & 0x80) == 0x80)
	{
		n = ((mcr >> 5)&0x03);

#ifdef STANDARD_EZMAC_PACKET_FORMAT
		if(n == 3)
		{ 
	   	TimeoutHeader = (U16)byteTime * (header-3);
		}
		else if(n == 2) 
		{
	   	TimeoutHeader = (U16)byteTime * (header-2);
		}
#endif //STANDARD_EZMAC_PACKET

#ifdef EXTENDED_PACKET_FORMAT
		if(n == 3)
		{ 
	   	TimeoutHeader = (U16)byteTime * (header-2);
		}
		else if(n == 2) 
		{
	   	TimeoutHeader = (U16)byteTime * (header-1);
		}

#endif
		else if(n < 2)
		{
	   	TimeoutHeader = (U16)byteTime * header;
		}
	}
	else
	{
		n = ((mcr >> 5)&0x03);
		if(n == 3)
		{ 
	   	TimeoutHeader = ((U16)byteTime >> 1) * (header-2);
		}
		else if(n == 2) 
		{
	   	TimeoutHeader = (U16)byteTime * (header-1);
		}
		else if(n < 2)
		{
	   	TimeoutHeader = (U16)byteTime * header;
		}
	}
   // calculate TimeoutRX_Packet using mpl
   n = header + mpl + 2 + 3;
   TimeoutRX_Packet = n * (U32)(byteTime);
#endif   // TRANSMITTER_ONLY_OPERATION

#ifndef RECEIVER_ONLY_OPERATION
	// calculate TimeoutTX_Packet using MPL
   n = preamble + SYNC_WORD_LENGTH + header + mpl + 2 + 8;
   TimeoutTX_Packet = n * (U32)(byteTime);
#endif


#ifdef EXTENDED_PACKET_FORMAT
   // if DNPL
   if((mcr & 0x04) == 0x04)
   {
      // use fixed one by payload for ACK
      mpl = 1;
   }

   n = preamble + SYNC_WORD_LENGTH + header + mpl + 2 + 3;
   //ACK time out: the SW make up the ACK packet(in 4 Mhz ~1.6ms) + 200 us(PLL settling time) + n * byte time
   TimeoutACK = (n * (U32)(byteTime));
   TimeoutACK += (U32)(MAKE_UP_THE_ACK_PACKET + PLL_SETTLING_TIME);

#ifdef PACKET_FORWARDING_SUPPORTED
   // use n for radius
   n = (mcr & 0x18)>>3;

   if(n)
   {
      TimeoutACK = ((n + 1) * TimeoutACK) + (n * TimeoutTX_Packet) + (n * MAX_LBT_WAITING_TIME * MAX_LBT_RETRIES);
   }
#endif//PACKET_FORWARDING_SUPPORTED
#endif//EXTENDED_PACKET_FORMAT

}
//------------------------------------------------------------------------------------------------
// Function Name: initForwardedPacketTable
//						This function resets the forwarding table.
// Return Value : None
// Parameters   : None
//------------------------------------------------------------------------------------------------
#ifdef PACKET_FORWARDING_SUPPORTED
void initForwardedPacketTable (void)
{
   U8 i;
   for (i = 0; i < FORWARDED_PACKET_TABLE_SIZE; i++)
   {
      ForwardedPacketTable[i].sid = 0;
      ForwardedPacketTable[i].seq = 0;
   }
}
#endif

//------------------------------------------------------------------------------------------------
// Function Name: macSetEnable2
//						This function can be use to set the Interrupt Enable2 register of the radio
// Return Value : None
// Parameters   : value - which bit should be set
//------------------------------------------------------------------------------------------------
void macSetEnable2(U8 value)
{

   if((EZMacProReg.name.LFTMR2 & 0x80)==0x80)
      value |= SI4432_ENWUT;

   if((EZMacProReg.name.LBDR & 0x80)==0x80)
      value |= SI4432_ENLBDI;

   macSpiWriteReg(SI4432_INTERRUPT_ENABLE_2, value);
}
//------------------------------------------------------------------------------------------------
// Function Name: macSetFunction1
//						This function can be use to set the Operation Function Control1 register of the radio
// Return Value : None
// Parameters   : value -which bit should be set
//
//------------------------------------------------------------------------------------------------
void macSetFunction1(U8 value)
{

   if((EZMacProReg.name.LFTMR2 & 0x80)==0x80)
      value |= SI4432_ENWT;

   if((EZMacProReg.name.LFTMR2 & 0x40)==0x00)
      value |= SI4432_X32KSEL;

   if((EZMacProReg.name.LBDR & 0x80)==0x80)
      value |= SI4432_ENLBD;

   macSpiWriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_1, value);
}
//------------------------------------------------------------------------------------------------
// Function Name: macUpdateLBTI
//						This function update the Listen Before Talk timeout.
//
// Return Value : None
// Parameters   : lbti - Listen Before Talk Interval Register value 
//
//------------------------------------------------------------------------------------------------

#ifdef   TRANSCIEVER_OPERATION
void macUpdateLBTI (U8 lbti)
{
   U8 rate;

   // check fixed or byte time LBT
   if((lbti & 0x80) == 0x80)
   {
    // set timeout to 100 us
    TimeoutLBTI = TIMEOUT_US(100);
   }
   else
   {
    // get data rate from MCR
    rate = ((EZMacProReg.name.MCR >> 5)&0x03);
    //look-up byte time
    TimeoutLBTI = EZMacProByteTime[rate];
   }

   // multiply by LBTI register
   TimeoutLBTI *= (U32)(lbti& 0x7F);
}
#endif //TRANSCIEVER_OPERATION


//================================================================================================
// end EZMacPro.c
//================================================================================================
