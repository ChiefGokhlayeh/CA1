/*  LCD module

    Computerarchitektur / Computer Architecture
    (C) 2020 J. Friedrich, W. Zimmermann 
    Hochschule Esslingen

    Author:   W.Zimmermann, May 06, 2020
    Modified: 
*/

#include <mc9s12dp256.h>                   
#include "lcd.h"

#ifndef _HCS12_SERIALMON 
  #ifndef SIMULATOR
    #define SIMULATOR
  #endif
#endif

//***** LCD Display ***********************************************************
/* Magic delay constants, based on 24MHz CPU clock */
#define ENBIT       (0x02)
#define DELAY40US   (200L)
#define DELAY100US  (5000L)
#define DELAY4_1MS  (22000L)
#define DELAY4_10MS (220000L)
#define LCDWIDTH    (16)

#define LCDDATA PORTK

////////////////////////////////////////////////////////////////////////////////
//! Delay routine (uses busy wait)
void Delay(unsigned long constant)
{   volatile unsigned long counter;
    for (counter = constant; counter > 0; counter--);
}

//! Fixed 10ms delay
void delay_10ms(void)
{   Delay(DELAY4_10MS);
}


////////////////////////////////////////////////////////////////////////////////
#ifdef SIMULATOR
// LCD functions for the Codewarrior HCS12 simulator

#define LCDCTRL PORTA

void SLcdWriteCmd(unsigned char cmd)
{   LCDCTRL = 0b00000100;
    LCDDATA = cmd;
    LCDCTRL = 0b00000000;
}

void SLcdWriteDat(unsigned char data)
{   LCDCTRL = 0b00000101;
    LCDDATA = data;
    LCDCTRL = 0b00000001;
}

void initLCD(void)
{   DDRA = 0xFF;

    SLcdWriteCmd(0x30);
    SLcdWriteCmd(0x30);
    SLcdWriteCmd(0x30);
    
    SLcdWriteCmd(0x38);
    SLcdWriteCmd(0x0C);	// 0x0E ???
    SLcdWriteCmd(0x01);
    SLcdWriteCmd(0x06);
}
////////////////////////////////////////////////////////////////////////////////
#else	// #ifdef _HCS12_SERIALMON  
// LCD function for the Dragon12 board

#define SLcdWriteCmd(cmd)  LcdWrite4(cmd, 0)
#define SLcdWriteDat(data) LcdWrite4(data,1)

/*
 * Write LCD module in 8-bit mode
 * Inputs:
 *  data: to be written, lower 4 bits are ignored
 *  rs: register select, only bit 0 is significant
 * Handles the shifting into place and the EN pulsing
 * This is only used at the start of the init sequence
 *
*/
static void LcdWrite8(unsigned char data)
{   unsigned char temp;

    temp = (data >> 2);                 //Prepare data, rs is always 0
    LCDDATA = temp;                     //Do write with EN=0 */
    LCDDATA = temp | ENBIT;             //         with EN=1 pulse write enable
    LCDDATA = temp;                     //         with EN=0
    Delay(DELAY40US);                   //Pause for display to complete processing
}

/*
 * Write LCD module in 4-bit mode
 * Inputs:
 *  data: to be written, 8 bits are significant
 *  rs: register select, only bit 0 is significant
 * Does two consecutive writes, high nibble, then low
 * Handles the shifting into place and the EN pulsing
 * This is can be used at any time (init and display)
 *
*/
static void LcdWrite4(unsigned char data, unsigned char rs)
{   unsigned char hi, lo;

    hi = ((data & 0xf0) >> 2) | (rs & 0x01);    //Split byte into 2 nibbles
    lo = ((data & 0x0f) << 2) | (rs & 0x01);

    LCDDATA = hi;                        //Do write upper nibble with EN=0
    LCDDATA = hi | ENBIT;                //                      with EN=1 pulse write enable
    LCDDATA = hi;                        //                      with EN=0
    LCDDATA = lo;                        //Do write lower nibble with EN=0
    LCDDATA = lo | ENBIT;                //                      with EN=1 pulse write enable
    LCDDATA = lo;                        //                      with EN=0
    Delay(DELAY40US);                   //Pause for display to complete processing
}

//! Initialize LCD module, must be called before using LCD display
void initLCD(void)
{   DDRK = 0xFF;                        //Set port K as output

    LcdWrite8(0x30);                    //Tell LCD once
    Delay(DELAY4_1MS);
    LcdWrite8(0x30);                    //Tell LCD twice
    Delay(DELAY100US);
    LcdWrite8(0x30);                    //Tell LCD thrice
    LcdWrite8(0x20);                    //Last write in 8-bit mode sets bus to 4 bit mode
                                        //Now we are in 4 bit mode, write upper/lower nibble
                                        
    SLcdWriteCmd(0x28); 		//last function set: 4-bit mode, 2 lines, 5x7 matrix
    SLcdWriteCmd(0x0C); 		//display on, cursor off, blink off									*/
    SLcdWriteCmd(0x01); 		//display clear																			*/
    SLcdWriteCmd(0x06); 		//cursor auto-increment, disable display shift
}
#endif
////////////////////////////////////////////////////////////////////////////////

//! Write a line of max. 16 ASCII characters to the LCD display
/* Write a line to the LCD.
 * Inputs:
 *   string: is a pointer to a null terminated array of char to be sent.
 *   line: determines which line to display (0=top line, 1=bottom line).
 * If the string is less than 16 characters long the rest of the line
 * is filled with blanks.
 *
 * Strings longer than 16 characters (excluding the terminating \0)
 * get truncated.
 *
*/
void writeLine(char* string, unsigned char line)
{   char currentChar;
    char endOfLine;

    if (line == 1)                      //Set address in LCD module
        SLcdWriteCmd(0xC0);		//-- bottom line
//	LcdWrite4(0xC0, 0);		//-- bottom line
    else
  	SLcdWriteCmd(0x80);  		//-- top line
//	LcdWrite4(0x80, 0);		//-- top line

     endOfLine = 0;			//Send 16 characters to LDC display
     for (currentChar = 0; currentChar < LCDWIDTH; ++currentChar)
     {  if (string[currentChar] == 0)
        {   endOfLine = 1;
	}
	if (endOfLine == 0)
	{   SLcdWriteDat(string[currentChar]); // rs=1 means data
//	    LcdWrite4(string[currentChar], 1); // rs=1 means data
	} else
	{   SLcdWriteDat(' ');
//	    LcdWrite4(' ', 1);
	}
     }
}

