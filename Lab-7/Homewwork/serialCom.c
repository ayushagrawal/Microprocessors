// Baud Rate = 1200
// External Clock = 24MHz
// Value = 256 - (24e6/(12*32*1200))
// Timer 1 Reload Value = CCH

/********* IMPORTING THE REQUIRED LIBRARY *************/

#include "AT89c5131.h"

/* Global Variables */
sbit LED0 = P1^7;
sfr acc = 0xE0;
char character = 'A';

/********* Defining the Function for Timer 1 Initialization **********/
// NOTE : Timer 1 is automatically used by serial communication in the way it is configured
void timer1_init()
{
	// Setup Parameters:
	// 1. Mode 2 - Auto Reload Mode
	// 2. TH1 = E6H for baud rate of 1200 Hz
	// 3. No Interrupt
	// 4. Gate disabled and Timer Mode on
	// 5. Enable the timer
	
	// TMOD = 0010 xxxx (1. & 4.)
	TMOD = TMOD | 0x20;		// Making only the desired bits to be '1'
	TMOD = TMOD & 0x2F;		// Making only the desired bits to be '0'
	
	// Setting up the baud rate (2.)
	TH1  = 0xCC;
	TL1  = 0xCC;
	
	// Disabling the Interrupt (3.)
	// IE = xxxx 0xxx
	IEN0 =  IEN0 & 0xF7;
	
	// Enabling the Timer1
	// TCON = 1xxx xxxx
	TCON = TCON | 0x80;
}

void serial_init()
{
	// Setup Parameters:
	// 1. Enable the Interrupt
	// 2. Enable even parity
	// 3. Enable the recieving pin
	
	// Configuring the Serial Register(2. & 3.)
	// SM0 = 1 for sending the Data Bit
	// SM1 = 1 for variable baud rate
	// SM2 = 0 for not having multiprocessor communication 
	// REN = 1 Recieve Enable
	SCON = SCON | 0xD0;
	
	// Eabling the Interrupt (1.)
	// IE = 1xx1 xxxx
	IEN0 =  IEN0 | 0x90;
	
	// NOTE: 'RI' AND 'TI' MUST BE CLEARED MANUALLY
}

/******* Setting up the Serial Communication Interrupt ********/

void serial_interrupt() interrupt 4									// (Address-3)/8 (Address = 23H)
{
	// This Interrupt occurs when either TI = '1' or RI = '1'
	// Things to perform:
	// 1. Transmit 'A' if TI = '1'
	
	if(TI == 1)
	{
		TI = 0;
		acc = character + 0;
		TB8 = ~PSW^0;
		SBUF = character;
		LED0 = 1;					// Cleared at the end to ensure that sufficient delay is there
	}
	
	
	LED0 = 0;
}

void main()
{
	P1 = 0xF0;			// Setting the port LEDs initially to zero
	timer1_init();
	serial_init();
	while(1);
}