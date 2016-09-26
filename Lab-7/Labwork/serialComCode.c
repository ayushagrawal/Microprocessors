// Baud Rate = 1200
// External Clock = 24MHz
// Value = 256 - (24e6/(12*32*1200))
// Timer 1 Reload Value = CCH

/********* IMPORTING THE REQUIRED LIBRARY *************/

#include "AT89c5131.h"
#define LCD_data  P2	    					// LCD Data port

//Function Declarations
void LCD_Init();
void LCD_DataWrite(char dat) reentrant;
void LCD_CmdWrite(char cmd) reentrant;
void LCD_Ready();
void sdelay(int delay);

sbit LCD_rs = P0^0;  								// LCD Register Select
sbit LCD_rw = P0^1;  								// LCD Read/Write
sbit LCD_en = P0^2;  								// LCD Enable
sbit LCD_busy = P2^7;								// LCD Busy Flag


/* Global Variables */
sbit LED0 = P1^7;
sbit LED1 = P1^6;
sfr acc = 0xE0;
char character;
char firstLineCounter = 0;
char line1cursor;

char transmission = 0;		// Zero indicates that th transmission is off

char str[] = "Hello World To U";
char i = 0;

char newSwitch;
char prevSwitch;

void delay_ms(int delay)
{
	int d=0;
	while(delay>0)
	{
		for(d=0;d<382;d++);
		delay--;
	}
}

void LCD_WriteString( char * str, unsigned char length)
{
    while(length>0)
    {
        LCD_DataWrite(*str);
        str++;
        length--;
    }
}

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
	TCON = TCON | 0x40;
}

void serial_init()
{
	// Setup Parameters:
	// 1. Enable the Interrupt
	// 2. Enable even parity
	// 3. Enable the recieving pin
	
	// Configuring the Serial Register(2. & 3.)
	// SM0 = 1 for sending the Data Parity Bit
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
		if(transmission == 1)
		{
			transmission = 0;
			LED0 = ~LED0;					// Cleared at the end to ensure that sufficient delay is there
			acc = character + 0;
			TB8 = ~PSW^0;
			delay_ms(100);
			SBUF = character;
		}
	}
	if(RI == 1)
	{
		LED1 = ~LED1;
		if(firstLineCounter == 16)
		{
			firstLineCounter = 0;
		}
		// Placing the cursor at appropriate position
		line1cursor = 0x80 + firstLineCounter;
		firstLineCounter++;
		LCD_CmdWrite(line1cursor);
		LCD_DataWrite(SBUF);
		RI = 0;
	}
}

/* Check Switch Function */
void check_switch()
{
	newSwitch = 0x0F & P1;
	if(newSwitch != prevSwitch)
	{
		i = 0;
		while(i<=15)
		{
			transmission = 1;
			character = str[i];
			TI = 1;
			while(!TI);		// Wait until transmission is completed
			i++;
		}
		transmission = 0;
		LCD_CmdWrite(0xC0);
		LCD_WriteString(str,16);
	}
}


void main()
{
	P2 = 0x00;											// Make Port 2 output 
	LCD_Init();
	P1 = 0x0F;			// Setting the port LEDs initially to zero
	timer1_init();
	serial_init();
	while(1)
	{
		prevSwitch = 0x0F & P1;
		delay_ms(500);
		check_switch();
	}
}

void LCD_Init()
{
  sdelay(100);
  LCD_CmdWrite(0x38);   	// LCD 2lines, 5*7 matrix
  LCD_CmdWrite(0x0E);			// Display ON cursor ON  Blinking off
  LCD_CmdWrite(0x01);			// Clear the LCD
  LCD_CmdWrite(0x80);			// Cursor to First line First Position
}

/**
 * FUNCTION_PURPOSE: Write Command to LCD
 * FUNCTION_INPUTS: cmd- command to be written
 * FUNCTION_OUTPUTS: none
 */
void LCD_CmdWrite(char cmd) reentrant 
{
	LCD_Ready();
	LCD_data=cmd;     			// Send the command to LCD
	LCD_rs=0;         	 		// Select the Command Register by pulling LCD_rs LOW
  LCD_rw=0;          			// Select the Write Operation  by pulling RW LOW
  LCD_en=1;          			// Send a High-to-Low Pusle at Enable Pin
  sdelay(5);
  LCD_en=0;
	sdelay(5);
}

/**
 * FUNCTION_PURPOSE: Write Command to LCD
 * FUNCTION_INPUTS: dat- data to be written
 * FUNCTION_OUTPUTS: none
 */
void LCD_DataWrite( char dat) reentrant 
{
	LCD_Ready();
  LCD_data=dat;	   				// Send the data to LCD
  LCD_rs=1;	   						// Select the Data Register by pulling LCD_rs HIGH
  LCD_rw=0;    	     			// Select the Write Operation by pulling RW LOW
  LCD_en=1;	   						// Send a High-to-Low Pusle at Enable Pin
  sdelay(5);
  LCD_en=0;
	sdelay(5);
}

/**
 * FUNCTION_PURPOSE: To check if the LCD is ready to communicate
 * FUNCTION_INPUTS: void
 * FUNCTION_OUTPUTS: none
 */
void LCD_Ready()
{
	LCD_data = 0xFF;
	LCD_rs = 0;
	LCD_rw = 1;
	LCD_en = 0;
	sdelay(5);
	LCD_en = 1;
	while(LCD_busy == 1)
	{
		LCD_en = 0;
		LCD_en = 1;
	}
	LCD_en = 0;
}

/**
 * FUNCTION_PURPOSE: A delay of 15us for a 24 MHz crystal
 * FUNCTION_INPUTS: void
 * FUNCTION_OUTPUTS: none
 */
void sdelay(int delay)
{
	char d=0;
	while(delay>0)
	{
		for(d=0;d<5;d++);
		delay--;
	}
}