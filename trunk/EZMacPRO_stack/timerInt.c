//================================================================================================
// timerInt.c
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
#include "si4432_v2.h"                 //
#include "hardware_defs.h"             // requires compiler_defs.h
#include "timerInt.h"                  // requires compiler_defs.h
#include "EZMacPro_defs.h"
#include "EZMacPro.h"                  // requires EZMacPro_defs.h
#include "EZMacProCallBack.h"          // requires compiler_defs.h
//---------------------------------------------------------------------------------------------------
//
// local function prototypes
//
//---------------------------------------------------------------------------------------------------
void timerIntTX_StateMachine (U8);
void timerIntRX_StateMachine (U8);
void timerIntWakeUp (void);
void timerIntSpiWriteReg (U8, U8);
U8 timerIntSpiReadReg (U8);
void timerIntTimeout (U32);
U8 timerIntRandom (void);
void timerIntNextRX_Channel(void);
#ifdef FOUR_CHANNEL_IS_USED
void timerIntIncrementError (U8);
#endif//FOUR_CHANNEL_IS_USED
void timerIntGotoNextStateUsingSECR(U8);
U8 timerIntHeaderError(void);
U8 timerIntBadAddrError(void);
U8 HopNextChannel(void);
void timerIntSetEnable2(U8);
void timerIntSetFunction1(U8);
void timerIntTouch (void);             // added for library support
//---------------------------------------------------------------------------------------------------
//
// local variables
//
//---------------------------------------------------------------------------------------------------
SEGMENT_VARIABLE(ChannelOccupiedInStartPeriod, U8, EZMAC_PRO_GLOBAL_MSPACE);
SEGMENT_VARIABLE(ChannelOccupiedCounter, U8, EZMAC_PRO_GLOBAL_MSPACE);

//------------------------------------------------------------------------------------------------
// Function Name
//    timerIntT0_ISR()
//
// Return Value : None
// Parameters   : None
//
// This is the Interrupt Service Routing for the T0 timer. The T0 timer is used for all MAC
// time outs and MAC timing events. The T0 time base uses SYSCLK/4 for all supported SYSCLK
// frequencies. The time outs are calculated using macros or calculated by the initialization
// or register write API functions. Since some time outs require long periods a 24-bit timer
// has been implemented using a global variable for the most significant byte. If the MSB of the
// timer (EZMacProTimerMSB) is non-zero, it will be decrement and the ISR will be called again
// when the timer overflows.
//
// This function disables the timer interrupts before executing the state machines. If a
// timeout event is to initiate another timeout event, the timerIntTimeout() function should
// be used.
//
// The Basic States (Wake-up, Receive, and Transmit) are implemented using if..else if bit
// tests for the corresponding bit in the master control register. The detailed TX and RX
// state machines are implemented in separate functions.
//
// Conditional compile time switches remove the TX or RX state machines for the RX only and
// and TX only builds.
//
// The timer interrupt should not call functions from other modules. This would create cause
// a multiple call to segment warning and result in poor RAM usage.
//
//-----------------------------------------------------------------------------------------------
INTERRUPT(timerIntT0_ISR, INTERRUPT_TIMER0)
{
   U8 state;

   if(EZMacProTimerMSB == 0)
   {
      DISABLE_MAC_TIMER_INTERRUPT();
      STOP_MAC_TIMER();
      CLEAR_MAC_TIMER_INTERRUPT();
      state = EZMacProReg.name.MSR & 0x0F;

      if( EZMacProReg.name.MSR == EZMAC_PRO_WAKE_UP)
      {//if the MAC is in Wake up state call the WakeUp function
         timerIntWakeUp();
      }
#ifndef RECEIVER_ONLY_OPERATION
      else if((EZMacProReg.name.MSR & TX_STATE_BIT) == TX_STATE_BIT)
      {//if the MAC is in transmit state then call the transmit state machine
         timerIntTX_StateMachine(state);
      }
#endif
#ifndef TRANSMITTER_ONLY_OPERATION
      else if((EZMacProReg.name.MSR & RX_STATE_BIT) == RX_STATE_BIT)
      {//if the MAC is in receive state then call the receiver
         timerIntRX_StateMachine(state);
      }
#endif
      else
      {
      }
   }
   else
   {
      EZMacProTimerMSB--;
   }
}
//------------------------------------------------------------------------------------------------
// Function Name
//    timerIntWakeUp()
//
// Return Value : None
// Parameters   : None
//
// The EZMAC_PRO_WAKE_UP state is used when starting the radio XTAL after sleep mode or when
// resetting the radio using a SW Reset SPI command.
//
// If this function is called, the crystal has failed to start with the specified time. This
// indicates are hardware problem. Either the crystal is not connect, the SDN/ is asserted,
// or the radio is not powered.
//
//-----------------------------------------------------------------------------------------------
void timerIntWakeUp (void)
{
   // SWRESET timeout error
   DISABLE_MAC_INTERRUPTS();
   //call the woke - up error callback function
   EZMacPRO_WokeUp_Error();
	//go to WAKE UP ERROR state
   EZMacProReg.name.MSR = WAKE_UP_ERROR;

}
//------------------------------------------------------------------------------------------------
// Function Name
//    timerIntTX_StateMachine()
//
// Return Value : None
// Parameters   : U8 state - TX state, least significant nibble of MSR
//
// This function implements the detailed TX state machine. The state machine is implemented
// using switch...case... statements. For efficient compilation, the states are a series
// of sequential continuous integers (no missing values). The states are enumerated in the
// EZMacPro.h header file.
//
// Conditional compile time switches remove the unused TX states if LBT of ACK is not supported.
// These states are also removed from the enumeration at the begriming or end of the list.
//
// Conditional compile time switches remove this entire function for the Receiver only build.
//
//-----------------------------------------------------------------------------------------------
#ifndef RECEIVER_ONLY_OPERATION
void timerIntTX_StateMachine (U8 state)
{

#ifdef TRANSCIEVER_OPERATION
   U32 timeout;
#endif//TRANSCIEVER_OPERATION


#ifdef ANTENNA_DIVERSITY_ENABLED
#ifndef B1_ONLY
	U8 temp8;
#endif//B1_ONLY
#endif//ANTENNA_DIVERSITY_ENABLED

   switch (state)
   {
#ifdef TRANSCIEVER_OPERATION
      case TX_STATE_LBT_START_LISTEN:
			if(BusyLBT == 0)
			{//the channel was free during the first 0.5ms
				ChannelOccupiedInStartPeriod = 0;
            // start timer with fix 4.5ms timeout
				timerIntTimeout(LBT_FIXED_TIME_4500US);
				// go to the next state
            EZMacProReg.name.MSR = TX_STATE_BIT | TX_STATE_LBT_LISTEN;
        		// disable the reciever
        		timerIntSetFunction1(SI4432_XTON);
	    		// enable the receiver again
	    		timerIntSetFunction1( SI4432_RXON|SI4432_XTON);

				ENABLE_MAC_TIMER_INTERRUPT();
			}
			else
			{//the channnel was busy during the first 0.5ms
				BusyLBT = 0;
				ChannelOccupiedInStartPeriod = 1;
				ChannelOccupiedCounter = 0;
            // start timer with fix 1ms timeout
            timerIntTimeout(LBT_FIXED_TIME_1000US);
				//go to the next state
            EZMacProReg.name.MSR = TX_STATE_BIT | TX_STATE_LBT_LISTEN;
        		// disable the reciever
        		timerIntSetFunction1(SI4432_XTON);
	    		// enable the receiver again
	    		timerIntSetFunction1( SI4432_RXON|SI4432_XTON);
				ENABLE_MAC_TIMER_INTERRUPT();
			}
			break;
		
		
		case TX_STATE_LBT_LISTEN:
         if (ChannelOccupiedInStartPeriod == 0)
			{//the channel was free during the 0.5ms start period
							
	         if(BusyLBT == 0)
	         {// LBT passed, channel should be clear in the fix 4.5ms period
					//disable the receiver
	            timerIntSetFunction1(SI4432_XTON);
					// clear enable 2 interrupt
	            timerIntSetEnable2(0x00);
	            // enable ENPKSENT bit
	            timerIntSpiWriteReg(SI4432_INTERRUPT_ENABLE_1, SI4432_ENPKSENT);
	            // enable TX
	            timerIntSetFunction1( SI4432_TXON|SI4432_XTON);
               // start timer with transmit packet timeout
					timerIntTimeout(TimeoutTX_Packet);
					//go to the next state
	            EZMacProReg.name.MSR = TX_STATE_BIT | TX_STATE_WAIT_FOR_TX;
	            ENABLE_MAC_TIMER_INTERRUPT();
	         }
   			else
				{//the channel was busy during the 4.5ms
					BusyLBT = 0;		
					// multiple by fixed plus random number
            	timeout = LBT_FIXED_TIME_5000US + TimeoutLBTI * (U32)(timerIntRandom());
               // start timer with timeout
            	timerIntTimeout(timeout);
					//go to the next state
 		         EZMacProReg.name.MSR = TX_STATE_BIT | TX_STATE_LBT_RANDOM_LISTEN;
           		// disable the reciever
		    		timerIntSetFunction1(SI4432_XTON);
	   	 		// enable the receiver again
	    			timerIntSetFunction1( SI4432_RXON|SI4432_XTON);
					ENABLE_MAC_TIMER_INTERRUPT();	
				} 
			}
			else
			{//the channel was busy during the 0.5ms start period
			   if(BusyLBT == 0)
	         {// LBT passed, channel should be clear in the fix 1ms period
            	// multiple by fixed plus random number
            	timeout = LBT_FIXED_TIME_5000US + TimeoutLBTI * (U32)(timerIntRandom());
               // start timer with timeout
            	timerIntTimeout(timeout);
					//go to the next state
 		         EZMacProReg.name.MSR = TX_STATE_BIT | TX_STATE_LBT_RANDOM_LISTEN;
         	   ENABLE_MAC_TIMER_INTERRUPT();
	         }
				else
				{//the channel was busy in the fix 1ms period
					if (ChannelOccupiedCounter < 9)
					{ 
						BusyLBT = 0;
	      	   	ChannelOccupiedCounter++;
	               // start timer with fix 1ms timeout
      	      	timerIntTimeout(LBT_FIXED_TIME_1000US);
						// go to the next state
            		EZMacProReg.name.MSR = TX_STATE_BIT | TX_STATE_LBT_LISTEN;
              		// disable the reciever
		        		timerIntSetFunction1(SI4432_XTON);
			    		// enable the receiver again
			    		timerIntSetFunction1( SI4432_RXON|SI4432_XTON);
						ENABLE_MAC_TIMER_INTERRUPT();
					}
					else
					{//the channel was busy during the 10*1ms 
						BusyLBT = 0;
						EZMacProLBT_Retrys++;
						if(EZMacProLBT_Retrys < MAX_LBT_RETRIES)
	         		{//the channel was busy and the retries didn't reach the maximum value
	            		// disable the reciever
	            		timerIntSetFunction1(SI4432_XTON);
	            		// start timer with fix ETSI timeout
	            		timerIntTimeout(TIMEOUT_LBTI_ETSI);
							// go to the next state
	            		EZMacProReg.name.MSR = TX_STATE_BIT | TX_STATE_LBT_START_LISTEN;
	            		ENABLE_MAC_TIMER_INTERRUPT();
	            		// enable the receiver again
	            		timerIntSetFunction1( SI4432_RXON|SI4432_XTON);
	         		}
	         		else
	         		{//the channel was busy and the retries reach the maximum value
		           		//disable the receiver
	            		timerIntSetFunction1(SI4432_XTON);
							//go to the next state
	            		EZMacProReg.name.MSR = TX_STATE_BIT | TX_ERROR_CHANNEL_BUSY;
#ifdef FOUR_CHANNEL_IS_USED
							//increment error counter
	            		timerIntIncrementError(EZMAC_PRO_ERROR_CHANNEL_BUSY);
#endif //FOUR_CHANNEL_IS_USED
							//call the LBT error callback function
	            		EZMacProTX_ErrorLBT_Timeout();
	         		}
					}
				}

			}
			break;	

		case TX_STATE_LBT_RANDOM_LISTEN:
				if(BusyLBT == 0)
				{// the channel was free during the 5ms + random period
					//disable the receiver
	            timerIntSetFunction1(SI4432_XTON);
					// clear enable 2 interrupt
	            timerIntSetEnable2(0x00);
	            // enable ENPKSENT bit
	            timerIntSpiWriteReg(SI4432_INTERRUPT_ENABLE_1, SI4432_ENPKSENT);
	            // enable TX
	            timerIntSetFunction1( SI4432_TXON|SI4432_XTON);
	            // start timer with transmit packet timeout
	            timerIntTimeout(TimeoutTX_Packet);
					//go to the next state
	            EZMacProReg.name.MSR = TX_STATE_BIT | TX_STATE_WAIT_FOR_TX;
	            ENABLE_MAC_TIMER_INTERRUPT();
				}
				else
				{//the channel was busy during the 5ms + random period
					BusyLBT = 0;
					EZMacProLBT_Retrys++;
					if(EZMacProLBT_Retrys < MAX_LBT_RETRIES)
         		{//the channel was busy and the retries didn't reach the maximum value
            		// disable the reciever
            		timerIntSetFunction1(SI4432_XTON);
            		// start timer with fix ETSI timeout
            		timerIntTimeout(TIMEOUT_LBTI_ETSI);
						// go to the next state
            		EZMacProReg.name.MSR = TX_STATE_BIT | TX_STATE_LBT_START_LISTEN;
            		ENABLE_MAC_TIMER_INTERRUPT();
            		// enable the receiver again
            		timerIntSetFunction1( SI4432_RXON|SI4432_XTON);
         		}
         		else
         		{//the channel was busy and the retries reach the maximum value
	           		//disable the receiver
            		timerIntSetFunction1(SI4432_XTON);
						//go to the next state
            		EZMacProReg.name.MSR = TX_STATE_BIT | TX_ERROR_CHANNEL_BUSY;
#ifdef FOUR_CHANNEL_IS_USED
						//increment error counter
						timerIntIncrementError(EZMAC_PRO_ERROR_CHANNEL_BUSY);
#endif //FOUR_CHANNEL_IS_USED
						//call LBT error callback function
			   		EZMacProTX_ErrorLBT_Timeout();
         		}

				}			
			
			break;

#endif//TRANSCIEVER_OPERATION

      case TX_STATE_WAIT_FOR_TX:

         // TX transmit error - no ipksent interrupt before timeout
         // This indicates a problem with the hardware or timeout
			//if there is a TX error then switch back the internal antenna diversity algorthm
#ifdef ANTENNA_DIVERSITY_ENABLED
#ifndef B1_ONLY
			//if revision V2 or A0 chip is used 
			if ((EZMacProReg.name.DTR == 0x00) || (EZMacProReg.name.DTR == 0x01))
			{
			
				//switch ON the internal algorithm
				temp8 = timerIntSpiReadReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2);
				timerIntSpiWriteReg (SI4432_OPERATING_AND_FUNCTION_CONTROL_2, temp8 | 0x80);
				//the gpios control the rf chip automatically
				timerIntSpiWriteReg(SI4432_GPIO1_CONFIGURATION, 0x17);
				timerIntSpiWriteReg(SI4432_GPIO2_CONFIGURATION, 0x18);
			}
#endif//B1_ONLY
#endif//ANTENNA_DIVERSITY_ENABLED
			//go to the TX ERROR STATE
         EZMacProReg.name.MSR = TX_STATE_BIT | TX_ERROR_STATE;

         break;

#ifdef EXTENDED_PACKET_FORMAT
#ifndef TRANSMITTER_ONLY_OPERATION
      case TX_STATE_WAIT_FOR_ACK:
			//call the no ack callback function
         EZMacProTX_ErrorNoAck();
			//disbale the interrupts
         timerIntSpiWriteReg( SI4432_INTERRUPT_ENABLE_1, 0x00);
         timerIntSetEnable2(0x00);
         //disable the receiver
         timerIntSetFunction1(SI4432_XTON);
			//go to TX ERROR NO ACK state
         EZMacProReg.name.MSR = TX_STATE_BIT | TX_ERROR_NO_ACK;
         break;
#endif//TRANSMITTER_ONLY_OPERATION
#endif//EXTENDED_PACKET_FORMAT
      case TX_ERROR_STATE:
         DISABLE_MAC_INTERRUPTS();        // clear EX0 & ET0
         // disable all Si443x interrupt sources
         timerIntSpiWriteReg(SI4432_INTERRUPT_ENABLE_1, 0x00);
         timerIntSetEnable2(0x00);
          // clear interrupts
         timerIntSpiReadReg(SI4432_INTERRUPT_STATUS_1);
         timerIntSpiReadReg(SI4432_INTERRUPT_STATUS_2);
         STOP_MAC_TIMER();                // stop Timer
         CLEAR_MAC_TIMER_INTERRUPT();     // clear flag
         CLEAR_MAC_EXT_INTERRUPT();
#ifndef B1_ONLY
			if(EZMacProReg.name.DTR == 0)//if rev V2 chip is used
		      // this register setting is need for good current consumption in Idle mode (only rev V2)
   	      timerIntSpiWriteReg (SI4432_CRYSTAL_OSCILLATOR_CONTROL_TEST, SI4432_BUFOVR);
#endif
         break;
#ifdef TRANSCIEVER_OPERATION
		case TX_ERROR_CHANNEL_BUSY:
			break;
#endif //TRANSCIEVER_OPERATION
  } // end switch
}
#endif // RECEIVER_ONLY_OPERATION not defined
//------------------------------------------------------------------------------------------------
// Function Name
//    timerIntRX_StateMachine()
//
// Return Value : None
// Parameters   : U8 state - RX state, least significant nibbble of MSR
//
// This function implements the detailed RX state machine. The state machine is implemented
// using switch...case... statements. For efficient compilation, the states are a series
// of sequential continuous integers (no missing values). The states are enumerated in the
// EZMacPro.h header file.
//
// Conditional compile time switches remove the unused RX states if LBT, ACK, or packet
// forwarding is not supported. These states are also removed from the enumeration at the
// begriming or end of the list.
//
// Conditional compile time switches remove this entire function for the Transmitter only build.
//
//-----------------------------------------------------------------------------------------------
#ifndef TRANSMITTER_ONLY_OPERATION
void timerIntRX_StateMachine (U8 state)
{
	U8 temp8;
#ifdef   PACKET_FORWARDING_SUPPORTED
   U32 timeout;
#endif//PACKET_FORWARDING_SUPPORTED
   switch (state)
   {
#ifdef FOUR_CHANNEL_IS_USED
      case RX_STATE_FREQUENCY_SEARCH:
         // jump to the next channel if search mechanism is enabled
         if((EZMacProReg.name.RCR & 0x04) == 0x04)
         {
	           timerIntNextRX_Channel();
         }
			// start timer with channel search timeout
         timerIntTimeout(TimeoutChannelSearch);
         ENABLE_MAC_TIMER_INTERRUPT();
			break;
#endif //FOUR_CHANNEL_IS_USED

#ifdef MORE_CHANNEL_IS_USED
		case RX_STATE_FREQUENCY_SEARCH:
			//check the channel number
			if (SelectedChannel < (maxChannelNumber - 1))
			{
				//jump to the next channel
				SelectedChannel++;
				//switch off the receiver
			  	timerIntSetFunction1(SI4432_XTON);
			  	timerIntSpiWriteReg (SI4432_FREQUENCY_HOPPING_CHANNEL_SELECT,EZMacProReg.array[FR0+SelectedChannel]);
				//switch on the receiver
				timerIntSetFunction1(SI4432_XTON|SI4432_RXON);
				// start timer with channel search timeout
				timerIntTimeout(TimeoutChannelSearch);
			  	ENABLE_MAC_TIMER_INTERRUPT();
			}
			else
			{
				//jump to the first channel
				SelectedChannel = 0;
				//switch off the receiver
			   timerIntSetFunction1(SI4432_XTON);
				timerIntSpiWriteReg (SI4432_FREQUENCY_HOPPING_CHANNEL_SELECT,EZMacProReg.array[FR0+SelectedChannel]);
				//switch on the receiver
		  		timerIntSetFunction1(SI4432_XTON|SI4432_RXON);
				// start timer with channel search timeout
		   	timerIntTimeout(TimeoutChannelSearch);
		   	ENABLE_MAC_TIMER_INTERRUPT();	
			}			
			break;
#endif//MORE_CHANNEL_IS_USED


		case RX_STATE_WAIT_FOR_PACKET:
	 		//set the dinamic plength if it is needed otherwise set the fix length 
		   if ((EZMacProReg.name.MCR & 0x04) == 0x04)
			{
				temp8 = timerIntSpiReadReg(SI4432_HEADER_CONTROL_2);
				temp8 &= ~0x08;
				timerIntSpiWriteReg(SI4432_HEADER_CONTROL_2, temp8);
			}
			else
			{
				timerIntSpiWriteReg(SI4432_TRANSMIT_PACKET_LENGTH, EZMacProReg.name.PLEN);
			}	
			
#ifdef MORE_CHANNEL_IS_USED
      case RX_STATE_WAIT_FOR_PREAMBLE:
#endif//MORE_CHANNEL_IS_USED

      case RX_STATE_WAIT_FOR_SYNC:
         // RX error - HW error or bad timeout calculation 
         //switch off the receiver
         timerIntSetFunction1(SI4432_XTON);
         // clear interrupt enable 1
         timerIntSpiWriteReg(SI4432_INTERRUPT_ENABLE_1, 0x00);
#ifdef FOUR_CHANNEL_IS_USED
         //Enable the Preamble valid interrupt
         timerIntSetEnable2(SI4432_ENPREAVAL);
         // jump to the next channel if search mechanism is enabled
         if((EZMacProReg.name.RCR & 0x04) == 0x04)
         {
            timerIntNextRX_Channel();
         }
			// start timer with channel search timeout
         timerIntTimeout(TimeoutChannelSearch);
         ENABLE_MAC_TIMER_INTERRUPT();

#endif//FOUR_CHANNEL_IS_USED

#ifdef MORE_CHANNEL_IS_USED
     		//enable the preamble valid interrupt
     		timerIntSetEnable2(SI4432_ENPREAVAL);
			//determine the next channel number
			if (SelectedChannel < (maxChannelNumber - 1))
			{
				SelectedChannel++;
			}
			else
			{
				SelectedChannel = 0;
			}
			//jump to the next channel
			timerIntSpiWriteReg (SI4432_FREQUENCY_HOPPING_CHANNEL_SELECT,EZMacProReg.array[FR0+SelectedChannel]);
			// start timer with channel search timeout
			timerIntTimeout(TimeoutChannelSearch);
         ENABLE_MAC_TIMER_INTERRUPT();

#endif//MORE_CHANNEL_IS_USED
			//enable the receiver
         timerIntSetFunction1(SI4432_RXON|SI4432_XTON);
			//go to the next channel
         EZMacProReg.name.MSR = RX_STATE_BIT | RX_STATE_FREQUENCY_SEARCH;
         break;

      case RX_STATE_WAIT_FOR_HEADERS:
         if (timerIntHeaderError())
         {
/*#ifdef FOUR_CHANNEL_IS_USED
				//increment the error counter
            timerIntIncrementError(EZMAC_PRO_ERROR_BAD_ADDR);
#endif //FOUR_CHANNEL_IS_USED
				// Disable the receiver
            timerIntSetFunction1(SI4432_XTON);

#ifdef FOUR_CHANNEL_IS_USED
            // Enable the preamble valid interrupt
            timerIntSetEnable2(SI4432_ENPREAVAL);
            // jump to the next channel if search mechanism is enabled
            if((EZMacProReg.name.RCR & 0x04) == 0x04)
            {
               timerIntNextRX_Channel();
					// start timer with channel search timeout
               timerIntTimeout(TimeoutChannelSearch);
               ENABLE_MAC_TIMER_INTERRUPT();
            }
#endif//FOUR_CHANNEL_IS_USED

#ifdef MORE_CHANNEL_IS_USED
		      //enable the preamble valid interrupt
   		   timerIntSetEnable2(SI4432_ENPREAVAL);
				//selected the next channel
				if (SelectedChannel < (maxChannelNumber - 1))
				{
					SelectedChannel++;
				}
				else
				{
					SelectedChannel = 0;
				}
				//jump to the next channel
				timerIntSpiWriteReg (SI4432_FREQUENCY_HOPPING_CHANNEL_SELECT,EZMacProReg.array[FR0+SelectedChannel]);
	   		//start timer with channel search timeout
				timerIntTimeout(TimeoutChannelSearch);
	         ENABLE_MAC_TIMER_INTERRUPT();
#endif//MORE_CHANNEL_IS_USED

            //Enable the receiver again
            timerIntSetFunction1(SI4432_RXON | SI4432_XTON);
				//go to the next state
            EZMacProReg.name.MSR = RX_STATE_BIT | RX_STATE_FREQUENCY_SEARCH;
            ENABLE_MAC_EXT_INTERRUPT();*/

				if ((EZMacProReg.name.MCR & 0x04) == 0x04)
				{
					temp8 = timerIntSpiReadReg(SI4432_HEADER_CONTROL_2);
					temp8 |= 0x08;
					timerIntSpiWriteReg(SI4432_HEADER_CONTROL_2, temp8);
				}

				timerIntSpiWriteReg(SI4432_TRANSMIT_PACKET_LENGTH, 5);

            timerIntSpiWriteReg(SI4432_INTERRUPT_ENABLE_1, SI4432_ENPKVALID|SI4432_ENCRCERROR);
            //set the rx packet time out
            timerIntTimeout(TimeoutRX_Packet);

            EZMacProReg.name.MSR = RX_STATE_BIT | RX_STATE_WAIT_FOR_PACKET;
            ENABLE_MAC_INTERRUPTS();
         }
         else
         {
             //Enable SI4432_ENPKVALID and SI4432_ENCRCERROR interrupts
            timerIntSpiWriteReg(SI4432_INTERRUPT_ENABLE_1, SI4432_ENPKVALID|SI4432_ENCRCERROR);
            // start timer with packet receive timeout
            timerIntTimeout(TimeoutRX_Packet);
				// go to the next state
            EZMacProReg.name.MSR = RX_STATE_BIT | RX_STATE_WAIT_FOR_PACKET;
            ENABLE_MAC_INTERRUPTS();
         }
         break;
#ifdef EXTENDED_PACKET_FORMAT
#ifndef RECEIVER_ONLY_OPERATION
      case RX_STATE_WAIT_FOR_SEND_ACK:
         // SW error 
#ifdef ANTENNA_DIVERSITY_ENABLED
#ifndef B1_ONLY
			//if revision V2 or A0 chip is used 
			if ((EZMacProReg.name.DTR == 0x00) || (EZMacProReg.name.DTR == 0x01))
			{
				//switch ON the internal algorithm
				temp8 = timerIntSpiReadReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2);
				timerIntSpiWriteReg (SI4432_OPERATING_AND_FUNCTION_CONTROL_2, temp8 | 0x80);
				//the gpios control the rf chip automatically
				timerIntSpiWriteReg(SI4432_GPIO1_CONFIGURATION, 0x17);
				timerIntSpiWriteReg(SI4432_GPIO2_CONFIGURATION, 0x18);
			}
#endif//B1_ONLY
#endif//ANTENNA_DIVERSITY_ENABLED
#ifndef PACKET_FORWARDING_SUPPORTED
			//set back the preamble length
#ifdef FOUR_CHANNEL_IS_USED
      	timerIntSpiWriteReg(SI4432_PREAMBLE_LENGTH, PreamRegValue);
#endif//FOUR_CHANNEL_IS_USED
#ifdef MORE_CHANNEL_IS_USED
			temp8 = timerIntSpiReadReg(SI4432_HEADER_CONTROL_2);
			timerIntSpiWriteReg(SI4432_HEADER_CONTROL_2, (temp8 | 0x01));
      	timerIntSpiWriteReg(SI4432_PREAMBLE_LENGTH, PreamRegValue);
#endif//MORE_CHANNEL_IS_USED
#endif//PACKET_FORWARDING_SUPPORTED
			//go to the RX ERROR STATE
         EZMacProReg.name.MSR = RX_STATE_BIT | RX_ERROR_STATE;
         break;
#endif//RECEIVER_ONLY_OPERATION
#endif//EXTENDED_PACKET_FORMAT

#ifdef   PACKET_FORWARDING_SUPPORTED
#ifdef   TRANSCIEVER_OPERATION
      case RX_STATE_FORWARDING_LBT_START_LISTEN:
			if(BusyLBT == 0)
			{//the channel was free during the first 0.5ms
				ChannelOccupiedInStartPeriod = 0;
            // start timer with fix 4.5ms timeout
            timerIntTimeout(LBT_FIXED_TIME_4500US);
				//go to the next state
            EZMacProReg.name.MSR = RX_STATE_BIT | RX_STATE_FORWARDING_LBT_LISTEN;
				// disable the receiver
        		timerIntSetFunction1(SI4432_XTON);
	    		// enable the receiver again
	    		timerIntSetFunction1( SI4432_RXON|SI4432_XTON);
				ENABLE_MAC_TIMER_INTERRUPT();
			}
			else
			{//the channnel was busy during the first 0.5ms
				BusyLBT = 0;
				ChannelOccupiedInStartPeriod = 1;
				ChannelOccupiedCounter = 0;
	         // start timer with fix 1ms timeout
            timerIntTimeout(LBT_FIXED_TIME_1000US);
				// go to the next state
            EZMacProReg.name.MSR = RX_STATE_BIT | RX_STATE_FORWARDING_LBT_LISTEN;
				// disable the receiver
        		timerIntSetFunction1(SI4432_XTON);
	    		// enable the receiver again
	    		timerIntSetFunction1( SI4432_RXON|SI4432_XTON);
				ENABLE_MAC_TIMER_INTERRUPT();
			}
			break;
		
		
		case RX_STATE_FORWARDING_LBT_LISTEN:
         if (ChannelOccupiedInStartPeriod == 0)
			{//the channel was free during the 0.5ms start period
							
	         if(BusyLBT == 0)
	         {// LBT passed, channel should be clear in the fix 4.5ms period
					//disable the receiver
	            timerIntSetFunction1(SI4432_XTON);
					// clear enable 2 interrupt
	            timerIntSetEnable2(0x00);
	            // enable ENPKSENT bit
	            timerIntSpiWriteReg(SI4432_INTERRUPT_ENABLE_1, SI4432_ENPKSENT);
	            // enable TX
	            timerIntSetFunction1( SI4432_TXON|SI4432_XTON);
	            // start timer with packet transmit timeout
	            timerIntTimeout(TimeoutTX_Packet);
					//go to the next state
	            EZMacProReg.name.MSR = RX_STATE_BIT | RX_STATE_FORWARDING_WAIT_FOR_TX;
	            ENABLE_MAC_TIMER_INTERRUPT();
	         }
   			else
				{//the channel was busy during the 4.5ms
					BusyLBT = 0;		
					// multiple by fixed plus random number
            	timeout = LBT_FIXED_TIME_5000US + TimeoutLBTI * (U32)(timerIntRandom());
            	// start timer with timeout
            	timerIntTimeout(timeout);
					//got to the next state
 		         EZMacProReg.name.MSR = RX_STATE_BIT | RX_STATE_FORWARDING_LBT_RANDOM_LISTEN;
         	   
					// disable the receiver
        			timerIntSetFunction1(SI4432_XTON);
	    			// enable the receiver again
	    			timerIntSetFunction1( SI4432_RXON|SI4432_XTON);
					ENABLE_MAC_TIMER_INTERRUPT();	
				} 
			}
			else
			{//the channel was busy during the 0.5ms start period
			   if(BusyLBT == 0)
	         {// LBT passed, channel should be clear in the fix 1ms period
            	// multiple by fixed plus random number
            	timeout = LBT_FIXED_TIME_5000US + TimeoutLBTI * (U32)(timerIntRandom());
            	// start timer with timeout
            	timerIntTimeout(timeout);
					//go to the next state
 		         EZMacProReg.name.MSR = RX_STATE_BIT | RX_STATE_FORWARDING_LBT_RANDOM_LISTEN;
         	   ENABLE_MAC_TIMER_INTERRUPT();
	         }
				else
				{//the channel was busy in the fix 1ms period
					if (ChannelOccupiedCounter < 9)
					{ 
						BusyLBT = 0;
	      	   	ChannelOccupiedCounter++;
						// start timer with fix 1ms timeout
      	      	timerIntTimeout(LBT_FIXED_TIME_1000US);
						//go to the next state
            		EZMacProReg.name.MSR = RX_STATE_BIT | RX_STATE_FORWARDING_LBT_LISTEN;
            		// disable the receiver
        				timerIntSetFunction1(SI4432_XTON);
	    				// enable the receiver again
	    				timerIntSetFunction1( SI4432_RXON|SI4432_XTON);
						ENABLE_MAC_TIMER_INTERRUPT();
					}
					else
					{//the channel was busy during the 10*1ms 
						BusyLBT = 0;
						EZMacProLBT_Retrys++;
						if(EZMacProLBT_Retrys < MAX_LBT_RETRIES)
	         		{//the channel was busy and the retries didn't reach the maximum value
	            		// disable the reciever
	            		timerIntSetFunction1(SI4432_XTON);
	            		// start timer with fix ETSI timeout
	            		timerIntTimeout(TIMEOUT_LBTI_ETSI);
							//go to the next state
	            		EZMacProReg.name.MSR = RX_STATE_BIT | RX_STATE_FORWARDING_LBT_START_LISTEN;
	            		ENABLE_MAC_TIMER_INTERRUPT();
	            		// enable the receiver again
	            		timerIntSetFunction1( SI4432_RXON|SI4432_XTON);
	         		}
	         		else
	         		{//the channel was busy and the retries reach the maximum value
		           		//disable the receiver
	            		timerIntSetFunction1(SI4432_XTON);
							//go to the next state
	            		EZMacProReg.name.MSR = RX_STATE_BIT | RX_ERROR_FORWARDING_WAIT_FOR_TX;
#ifdef  FOUR_CHANNEL_IS_USED
							//increment error counter
	            		timerIntIncrementError(EZMAC_PRO_ERROR_CHANNEL_BUSY);
#endif//FOUR_CHANNEL_IS_USED
							//call the LBT error callback function
							EZMacProTX_ErrorLBT_Timeout();
	         		}
					}
				}

			}
			break;	

		case RX_STATE_FORWARDING_LBT_RANDOM_LISTEN:
				if(BusyLBT == 0)
				{// the channel was free during the 5ms + random period
					//disable the receiver
	            timerIntSetFunction1(SI4432_XTON);
					// clear enable 2 interrupt
	            timerIntSetEnable2(0x00);
	            // enable ENPKSENT bit
	            timerIntSpiWriteReg(SI4432_INTERRUPT_ENABLE_1, SI4432_ENPKSENT);
	            // enable TX
	            timerIntSetFunction1( SI4432_TXON|SI4432_XTON);
	            // start timer with fix transmit packet timeout
	            timerIntTimeout(TimeoutTX_Packet);
					//go to the next state
	            EZMacProReg.name.MSR = RX_STATE_BIT | RX_STATE_FORWARDING_WAIT_FOR_TX;
	            ENABLE_MAC_TIMER_INTERRUPT();
				}
				else
				{//the channel was busy during the 5ms + random period
					BusyLBT = 0;
					EZMacProLBT_Retrys++;
					if(EZMacProLBT_Retrys < MAX_LBT_RETRIES)
         		{//the channel was busy and the retries didn't reach the maximum value
            		// disable the reciever
            		timerIntSetFunction1(SI4432_XTON);
            		// start timer with fix ETSI timeout
            		timerIntTimeout(TIMEOUT_LBTI_ETSI);
						// go to the next state
            		EZMacProReg.name.MSR = RX_STATE_BIT | RX_STATE_FORWARDING_LBT_START_LISTEN;
            		ENABLE_MAC_TIMER_INTERRUPT();
            		// enable the receiver again
            		timerIntSetFunction1( SI4432_RXON|SI4432_XTON);
         		}
         		else
         		{//the channel was busy and the retries reach the maximum value
	           		//disable the receiver
            		timerIntSetFunction1(SI4432_XTON);
						//go to the next state
            		EZMacProReg.name.MSR = RX_STATE_BIT | RX_ERROR_FORWARDING_WAIT_FOR_TX;
#ifdef FOUR_CHANNEL_IS_USED
						//increment the error counter
            		timerIntIncrementError(EZMAC_PRO_ERROR_CHANNEL_BUSY);
#endif//FOUR_CHANNEL_IS_USED
          			//cll the LBT error callback function
						EZMacProTX_ErrorLBT_Timeout();
         		}

				}			
			
			break;

#endif   // TRANSCIEVER_OPERATION
      case RX_STATE_FORWARDING_WAIT_FOR_TX:
         // TX timeout - HW error or bad timeout calculation
         //if there is a TX error then switch back the internal antenna diversity algorthm
#ifdef ANTENNA_DIVERSITY_ENABLED
#ifndef B1_ONLY
			//if revision V2 or A0 chip is used 
			if ((EZMacProReg.name.DTR == 0x00) || (EZMacProReg.name.DTR == 0x01))
			{
			
				//switch ON the internal algorithm
				temp8 = timerIntSpiReadReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2);
				timerIntSpiWriteReg (SI4432_OPERATING_AND_FUNCTION_CONTROL_2, temp8 | 0x80);
				//the gpios control the rf chip automatically
				timerIntSpiWriteReg(SI4432_GPIO1_CONFIGURATION, 0x17);
				timerIntSpiWriteReg(SI4432_GPIO2_CONFIGURATION, 0x18);
			}
#endif//B1_ONLY
#endif//ANTENNA_DIVERSITY_ENABLED
			EZMacProReg.name.MSR = TX_STATE_BIT | RX_ERROR_STATE;
         break;
#endif   // PACKET_FORWARDING_SUPPORTED

      case RX_ERROR_STATE:
         DISABLE_MAC_INTERRUPTS();        // clear EX0 & ET0
         // disable all Si443x interrupt sources
         timerIntSpiWriteReg(SI4432_INTERRUPT_ENABLE_1, 0x00);
         timerIntSetEnable2(0x00);
          // clear interrupts
         timerIntSpiReadReg(SI4432_INTERRUPT_STATUS_1);
         timerIntSpiReadReg(SI4432_INTERRUPT_STATUS_2);
         STOP_MAC_TIMER();                // stop Timer
         CLEAR_MAC_TIMER_INTERRUPT();     // clear flag
         CLEAR_MAC_EXT_INTERRUPT();
#ifndef B1_ONLY
			if(EZMacProReg.name.DTR == 0)//if rev V2 chip is used 
	         // this register setting is need for good current consumption in Idle mode (only rev V2)
   	      timerIntSpiWriteReg (SI4432_CRYSTAL_OSCILLATOR_CONTROL_TEST, SI4432_BUFOVR);
#endif//B1_ONLY
         break;
      default:
         break;
   }  // end switch
}
#endif   // TRANSMITTER_ONLY_OPERATION not defined
//================================================================================================
//
// spi Functions for timerInt.c module
//
//================================================================================================
//
// Notes:
//
// The spi functions in this module are for use in the timerInt thread. The SPI is used by the
// main thread, as well as the external interrupt INT0 thread, and the T0 interrupt. The SPI
// functions are duplicated for the timerInt module. If the same SPI functions were used, the
// linker would generate a multiple call to segment warning. The linker would not be able to
// overlay the threads separately, local data may be corrupted be a reentrant function call,
// the SPI transfer may be corrupted.
//
// These SPI functions may be interrupted by a high priority interrupt, so the double buffered
// transfers are managed with this in mind.
//
// The double buffered transfer maximizes the data throughput and eliminates any software
// delay between bytes. The clock is continuous for the two byte transfer. Instead of using the
// SPIF flag for each byte, the TXBMT is used to keep the transmit buffer full, then the SPIBSY
// bit is used to determine when all bits have been transferred. The SPIF flag should not be
// polled in double buffered transfers.
//
//------------------------------------------------------------------------------------------------
// Function Name
//    timerIntSpiWriteReg()
//
// Return Value   : None
// Parameters :
//    U8 reg - register address from the si4432.h file.
//    U8 value - value to write to register
//
// Notes:
//    Write uses a Double buffered transfer.
//    This function is not included in the Transmitter only configuration.
//
//-----------------------------------------------------------------------------------------------
void timerIntSpiWriteReg (U8 macReg, U8 value)
{
   NSS = 0;                            // drive NSS low
   SPIF = 0;                           // clear SPIF
   SPI_DAT = (macReg | 0x80);          // write reg address
   while(!TXBMT);                      // wait on TXBMT
   SPI_DAT = value;                    // write value
   while(!TXBMT);                      // wait on TXBMT
   while((SPI_CFG & 0x80) == 0x80);    // wait on SPIBSY

   SPIF = 0;                           // leave SPIF cleared
   NSS = 1;                            // drive NSS high
}
//------------------------------------------------------------------------------------------------
// Function Name
//    timerIntSpiReadReg()
//
// Return Value : U8 value - value returned from the si4432 register
// Parameters   : U8 reg - register address from the si4432.h file.
//
// Notes:
//    Read uses a Double buffered transfer.
//    This function is not included for the Transmitter only configuration.
//
//-----------------------------------------------------------------------------------------------
U8 timerIntSpiReadReg (U8 macReg)
{
   U8 value;

   NSS = 0;                            // drive NSS low
   SPIF = 0;                           // cleat SPIF
   SPI_DAT = ( macReg );               // write reg address
   while(!TXBMT);                      // wait on TXBMT
   SPI_DAT = 0x00;                     // write anything
   while(!TXBMT);                      // wait on TXBMT
   while((SPI_CFG & 0x80) == 0x80);    // wait on SPIBSY
   value = SPI_DAT;                    // read value
   SPIF = 0;                           // leave SPIF cleared
   NSS = 1;                            // drive NSS low

   return value;
}
//================================================================================================
//
// Timer Functions for timerInt.c module
//
//================================================================================================
//------------------------------------------------------------------------------------------------
// Function Name
//    timerIntTimeout()
//
// Return Value : None
// Parameters   : U32 longTime
//
// Notes:
//
// This function is called when a timeout event must initiate a subsequent timeout event.
// A 32-bit union is used to provide word and byte access. The upper word is stored in
// EZMacProTimerMSB. The The lower word is first negated then written to the TL0 and TH0 sfrs.
//
// This function is not included for the Transmitter only configuration.
//
//-----------------------------------------------------------------------------------------------
#ifndef TRANSMITTER_ONLY_OPERATION
void timerIntTimeout (U32 longTime)
{
   UU32 time;

   ET0 = 0;                            // disable Timer interrupts
   TR0 = 0;                            // stop timer if already running

   time.U32 = longTime;

   EZMacProTimerMSB = time.U16[MSB];

   time.U16[LSB] = - time.U16[LSB];

   TL0 = time.U8[b0];               // write LSB first
   TH0 = time.U8[b1];               // write MSB last

   TF0 = 0;                            // clear flag
   TR0 = 1;                            // run timer
}
#endif//TRANSMITTER_ONLY_OPERATION
//================================================================================================
//-----------------------------------------------------------------------------------------------
// Function Name
//    timerIntRandom()
//
// Return Value : U8 4-bit random number
// Parameters   : None
//
// Notes:
//
// This function provides a pseudo random number.
// It uses 8-bit multiply and shift to generate the next random number.
//
// The constants used are based on the largest possible primes that will
// satisfy the linear congruent criteria.
//
// The pseudo random sequence will repeat every 256 times.
// The sequence always starts at the same point. If the application
// requires a different sequence each time on reset, a truly random
// seed may be required.
//
// This function is only included if LISTEN_BEFORE_TALK_SUPPORTED is defined.
//
//-----------------------------------------------------------------------------------------------
#ifdef TRANSCIEVER_OPERATION
U8 timerIntRandom (void)
{              
	U8 temp8; 
						                       // 61 is largest prime less than 256/4
   EZMacProRandomNumber *= 245;        // 4 * 61 + 1 = 245
   EZMacProRandomNumber += 251;        // 251 is the largest prime < 256

	temp8 = EZMacProRandomNumber>>4;
	if (temp8 == 0)
		return 1;
	else
		return temp8;	
}
#endif //TRANSCIEVER_OPERATION
//------------------------------------------------------------------------------------------------
// Function Name
//    timerIntNextRX_Channel()
//
// Return Value : None
// Parameters   : None
//
// Notes:
//
// This function will advance to the next channel. The channel is incremented and then checked
// against the frequency mask in the MAC RCR register. Setting all mask bits is not permitted.
//
// This function is not included for the Transmitter only configuration.
//
//-----------------------------------------------------------------------------------------------
#ifndef TRANSMITTER_ONLY_OPERATION
#ifndef MORE_CHANNEL_IS_USED
void timerIntNextRX_Channel (void)
{
   U8  mask;
   U8  n;

   n = EZMacProCurrentChannel;

   //initialize mask
   mask = 0x08;
   mask <<= n;

   do
   {
      n++;                             // increment n
      n &= 0x03;                       // wrap modulo 4
      mask <<=1 ;                      // shift mask left
      if(mask == 0x80) mask = 0x08;    // wrap mask to bits 3-6

      // continue until unmasked bit is found
   } while ((EZMacProReg.name.RCR & mask) == mask);

   //switch off the receiver
   timerIntSetFunction1(SI4432_XTON);
   timerIntSpiWriteReg (SI4432_FREQUENCY_HOPPING_CHANNEL_SELECT,EZMacProReg.array[FR0+n]);
   //switch on the receiver
   timerIntSetFunction1(SI4432_XTON|SI4432_RXON);

   EZMacProCurrentChannel = n;
}
#endif//MORE_CHANNEL_IS_USED
#endif // TRANSMITTER_ONLY_OPERATION not defined

//------------------------------------------------------------------------------------------------
// Function Name
//    timerIntIncrementError()
//
// Return Value : None
// Parameters   : U8 mask - SECR error mask
//
// Notes:
//
// This function will increment the appropriate error counter register if the error type is
// enabled in the MAC SECR register. The error codes for the SECR register are defined in the
// EZMacPro.h header file. First the error code is compared to the error mask in the SECR
// register. The counting is enabled for the particular type of error, the error count is
// incremented. The error count corresponding to the current frequency is incremented.
//
// This function is not included for the Transmitter only configuration.
//
//------------------------------------------------------------------------------------------------
#ifdef FOUR_CHANNEL_IS_USED
#ifndef TRANSMITTER_ONLY_OPERATION
void timerIntIncrementError (U8 mask)
{
   // mask is SECR error bit mask
   mask &= 0x0F;                          // ignore upper nibble


   if((EZMacProReg.name.SECR & mask) == mask )
   {
      if(EZMacProReg.array[EC0+EZMacProCurrentChannel] < 255)
         EZMacProReg.array[EC0+EZMacProCurrentChannel]++;
   }
}
#endif // TRANSMITTER_ONLY_OPERATION not defined
#endif //FOUR_CHANNEL_IS_USED
//------------------------------------------------------------------------------------------------
// Function Name
//    timerIntHeaderError()
//
// Return Value : U8 error - (1 error, 0 no error)
// Parameters   : None
//
// Notes:
//
// This function provides SW header filtering for the received packet after receiving the header
// bytes. The function will return a 1 (True) if there is a header error. So the function is
// named for the error.
//
// The filters and error counters are enabled separately. So the filters are checked in the
// order they are received and grouped according to the errors. This function calls the
// BadAddrError function to test the address filters and increments the bad address if
// there is an address error.
//
// Note that the Si4432 HW filters could be used only if the multi-cast filter is not used
// and there is no mechanism to distinguish between CID, DID, and SID header errors.
//
// This function is not included for the Transmitter only configuration.
//
//------------------------------------------------------------------------------------------------
#ifndef TRANSMITTER_ONLY_OPERATION
U8 timerIntHeaderError(void)
{
   U8 rcid;
   U8 packetLength;

   if(EZMacProReg.name.PFCR & 0x02) // PFEN=1 - promiscuous mode
      return 0;


	if (( EZMacProReg.name.MCR & 0x80 ) == 0x80)//if CID is used 
   {
		//if the Customer ID filter is enabled then CID will be checked
   	if ((EZMacProReg.name.PFCR & 0x80) == 0x80)
   	{

#ifdef EXTENDED_PACKET_FORMAT
			rcid = timerIntSpiReadReg(SI4432_RECEIVED_HEADER_2);
#else  // STANDARD_EZMAC_PACKET_FORMAT
		   rcid = timerIntSpiReadReg(SI4432_RECEIVED_HEADER_3);
#endif//EXTENDED_PACKET_FORMAT 
		   if(rcid != EZMacProReg.name.SCID)
		   {
#ifdef FOUR_CHANNEL_IS_USED
				//increment error counter
		   	timerIntIncrementError(EZMAC_PRO_ERROR_BAD_CID);
#endif //FOUR_CHANNEL_IS_USED
		 	   return 1;
		   }
		}
	}

   if(timerIntBadAddrError())
   {
#ifdef FOUR_CHANNEL_IS_USED
		//increment error counter
      timerIntIncrementError(EZMAC_PRO_ERROR_BAD_ADDR);
#endif //FOUR_CHANNEL_IS_USED
		return 1;
   }

   //if the Packet Length filter is enabled then Received Packet length will be checked
   if((EZMacProReg.name.PFCR & 0x04) == 0x04)
   {
      packetLength = timerIntSpiReadReg(SI4432_RECEIVED_PACKET_LENGTH);

      if (packetLength > EZMacProReg.name.MPL)
      {
         return 1;
         // no error count
      }
   }

   return 0;
}
#endif // TRANSMITTER_ONLY_OPERATION not defined
//------------------------------------------------------------------------------------------------
// Function Name
//    timerIntBadAddrError()
//
// Return Value : U8 error - (1 error, 0 no error)
// Parameters   : None
//
// Notes:
//
// This function applies the address (SID & DID) filters and returns a 1 (True) if there is a
// address error.
//
// Note that if packed forwarding is enabled the DID filter is not applied until after the
// packet is received and considered for forwarding, in the externalInt.c module.
//
// This function is not included for the Transmitter only configuration.
//
//------------------------------------------------------------------------------------------------
#ifndef TRANSMITTER_ONLY_OPERATION
U8 timerIntBadAddrError(void)
{
   U8 rsid;

#ifndef PACKET_FORWARDING_SUPPORTED
  U8 rdid;
#endif

   //if the Sender filter is enabled then SID will be checked
   if ((EZMacProReg.name.PFCR & 0x40) == 0x40)
   {

#ifdef EXTENDED_PACKET_FORMAT
		if (( EZMacProReg.name.MCR & 0x80 ) == 0x80)//if CID is used 
         rsid = timerIntSpiReadReg(SI4432_RECEIVED_HEADER_1);
      else
         rsid = timerIntSpiReadReg(SI4432_RECEIVED_HEADER_2);
#else //STANDARD_EZMAC_PACKET_FORMAT
      if (( EZMacProReg.name.MCR & 0x80 ) == 0x80)//if CID is used 
         rsid = timerIntSpiReadReg(SI4432_RECEIVED_HEADER_2);
      else
         rsid = timerIntSpiReadReg(SI4432_RECEIVED_HEADER_3);
#endif //EXTENDED_PACKET_FORMAT

      if ((EZMacProReg.name.SFLT & EZMacProReg.name.SMSK) != (rsid & EZMacProReg.name.SMSK))
      {
         return 1;
      }
   }


#ifdef PACKET_FORWARDING_SUPPORTED
   // Don't check DID
   return 0;
#else

   //if the Destination filter is enabled then DID will be checked
   if ((EZMacProReg.name.PFCR & 0x20) == 0x20)
   {
      //read DID from appropriate header byte
#ifdef EXTENDED_PACKET_FORMAT
	if (( EZMacProReg.name.MCR & 0x80 ) == 0x80)//if CID is used 
      rdid = timerIntSpiReadReg(SI4432_RECEIVED_HEADER_0);
	else
      rdid = timerIntSpiReadReg(SI4432_RECEIVED_HEADER_1);
#else // STANDARD_EZMAC_PACKET_FORMAT
	if (( EZMacProReg.name.MCR & 0x80 ) == 0x80)//if CID is used 
      rdid = timerIntSpiReadReg(SI4432_RECEIVED_HEADER_1);
	else
      rdid = timerIntSpiReadReg(SI4432_RECEIVED_HEADER_2);
#endif//EXTENEDED_PACKET_FORMAT
      if (rdid == EZMacProReg.name.SFID)
      {
         EZMacProReceiveStatus |= 0x80;
         return 0;
      }

      //if the Broadcast filter is enabled then DID will be checked
      if((EZMacProReg.name.PFCR & 0x08) == 0x08)
      {
         if (rdid == 0xFF)
         {
            EZMacProReceiveStatus |= 0x20;
            return 0;
         }
      }

      //if the Multi-cast filter is enabled
      if((EZMacProReg.name.PFCR & 0x10) == 0x10)
      {
         // Multi-cast address mode
         if ((EZMacProReg.name.PFCR & 0x01) == 0x01)
         {
            if(rdid == EZMacProReg.name.MCA_MCM)
            {
               EZMacProReceiveStatus |= 0x40;
               return 0;
            }
         }
         else // multi-cast mask mode
         {
            if ((rdid & EZMacProReg.name.MCA_MCM) == (EZMacProReg.name.SFID & EZMacProReg.name.MCA_MCM))
            {
               return 0;
            }
         }
      }

      return 1; // none of above
   }
   else
   {
      return 0; // no DID Error - passes everything else
   }
#endif // PACKET_FORWARDING_SUPPORTED
}
#endif // TRANSMITTER_ONLY_OPERATION not defined

//------------------------------------------------------------------------------------------------
// Function Name
//    timerIntSetEnable2()
//
// Return Value : None
// Parameters   : U8 value
//
// Notes:
//
// This function is used instead of timerIntSpiWriteReg() when writing to the
// SI4432_INTERRUPT_ENABLE_2 register. This function adds support for the low frequency timer
// and low battery detector, if the build options are defined. If not, the register is written
// directly.
//
// This function is not included for the Transmitter only configuration.
//
//------------------------------------------------------------------------------------------------
void timerIntSetEnable2(U8 value)
{
   if((EZMacProReg.name.LFTMR2 & 0x80)==0x80)
   {
      value |= SI4432_ENWUT;
   }

   if((EZMacProReg.name.LBDR & 0x80)==0x80)
      value |= SI4432_ENLBDI;

   timerIntSpiWriteReg(SI4432_INTERRUPT_ENABLE_2, value);
}
//------------------------------------------------------------------------------------------------
// Function Name
//    timerIntSetFunction1()
//
// Return Value : None
// Parameters   : U8 value
//
// This function is used instead of timerIntSpiWriteReg() when writing to the
// SI4432_OPERATING_AND_FUNCTION_CONTROL_1 register. This function adds support for the
// low frequency timer and low battery detector, if the build options are defined. If not,
// the register is written directly.
//
// This function is not included for the Transmitter only configuration.
//
//------------------------------------------------------------------------------------------------
#ifndef TRANSMITTER_ONLY_OPERATION
void timerIntSetFunction1(U8 value)
{
   if((EZMacProReg.name.LFTMR2 & 0x80)==0x80)
   {
      value |= SI4432_ENWT;
      ENABLE_MAC_EXT_INTERRUPT();
   }
   if((EZMacProReg.name.LFTMR2 & 0x40)==0x00)
      value |= SI4432_X32KSEL;

   if((EZMacProReg.name.LBDR & 0x80)==0x80)
      value |= SI4432_ENLBD;

   timerIntSpiWriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_1, value);
}
#endif//TRANMITTER_ONLY_OPERATION
//------------------------------------------------------------------------------------------------
// timerIntTouch() added for Keil library support
//------------------------------------------------------------------------------------------------
void timerIntTouch (void)
{
}
//================================================================================================
// end timerInt.c
//================================================================================================

