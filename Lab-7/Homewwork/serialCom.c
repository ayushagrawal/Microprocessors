// Baud Rate = 1200
// External Clock = 24MHz
// Value = 256 - (24e6/(12*32*1200))
// Timer 1 Reload Value = CCH

/********* IMPORTING THE REQUIRED LIBRARY *************/

#include "AT89c5131.h"

/********* Defining the Function for timer initializing **********/

void timer1_init()
{
	// Required Conditions:
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

void main()
{
	timer1_init();
}