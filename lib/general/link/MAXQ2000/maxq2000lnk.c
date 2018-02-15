//---------------------------------------------------------------------------
// Copyright (C) 2000 Dallas Semiconductor Corporation, All Rights Reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY,  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL DALLAS SEMICONDUCTOR BE LIABLE FOR ANY CLAIM, DAMAGES
// OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// Except as contained in this notice, the name of Dallas Semiconductor
// shall not be used except as stated in the Dallas Semiconductor
// Branding Policy.
//---------------------------------------------------------------------------
//
//  maxq2000LNK.C - Link Layer functions required by general 1-Wire drive
//           implementation for MAXQ2000 microcontroller eval kit.
//
//  Version: 2.00
//
//  History: 1.00 -> 1.01  Added function msDelay.
//           1.02 -> 1.03  Added function msGettick.
//           1.03 -> 2.00  Changed 'MLan' to 'ow'. Added support for
//                         multiple ports.
//
//           2.10 -> 3.00  Added owReadBitPower and owWriteBytePower

#include "ownet.h"
#include <iomaxq200x.h>

// exportable link-level functions
SMALLINT owTouchReset(int);
SMALLINT owTouchBit(int,SMALLINT);
SMALLINT owTouchByte(int,SMALLINT);
SMALLINT owWriteByte(int,SMALLINT);
SMALLINT owReadByte(int);
SMALLINT owSpeed(int,SMALLINT);
SMALLINT owLevel(int,SMALLINT);
SMALLINT owProgramPulse(int);
void msDelay(int);
void usDelay(unsigned int);
long msGettick(void);
SMALLINT owWriteBytePower(int,SMALLINT);
SMALLINT owReadBitPower(int,SMALLINT);
SMALLINT hasPowerDelivery(int);
SMALLINT hasOverDrive(int);
SMALLINT hasProgramPulse(int);

static SMALLINT USpeed = 0; // current 1-Wire Net communication speed
static SMALLINT ULevel = 0; // current 1-Wire Net level

static SMALLINT counter = 0;

#define OW_COMMAND    0x00
#define OW_BUFFER     0x01
#define OW_INTERRUPT  0x02
#define OW_INT_ENABLE 0x03
#define OW_CLOCK      0x04
#define OW_CONTROL    0x05


//--------------------------------------------------------------------------
// Reset all of the devices on the 1-Wire Net and return the result.
//
// 'portnum'    - number 0 to MAX_PORTNUM-1.  This number is provided to
//                indicate the symbolic port number.
//
// Returns: TRUE(1):  presense pulse(s) detected, device(s) reset
//          FALSE(0): no presense pulses detected
//
SMALLINT owTouchReset(int portnum)
{
   int rslt;
   if(ULevel!=MODE_NORMAL)
      owLevel(portnum, MODE_NORMAL);

   OWA = OW_COMMAND;
   OWD = 0x01;
   
   usDelay(1200);
   
   OWA = OW_INTERRUPT;
   rslt = OWD;
   
   // assert that the line was not shorted
   // and that the reset is complete
   OWASSERT((rslt&0x041)==0x01,
      OWERROR_RESET_FAILED, FALSE);

   return (rslt&0x02)==0x00?TRUE:FALSE; // 0x02 - pdr bit - Presence Detect Result
}

//--------------------------------------------------------------------------
// Send 1 bit of communication to the 1-Wire Net and return the
// result 1 bit read from the 1-Wire Net.  The parameter 'sendbit'
// least significant bit is used and the least significant bit
// of the result is the return bit.
//
// 'portnum'    - number 0 to MAX_PORTNUM-1.  This number is provided to
//                indicate the symbolic port number.
// 'sendbit'    - the least significant bit is the bit to send
//
// Returns: 0:   0 bit read from sendbit
//          1:   1 bit read from sendbit
//
SMALLINT owTouchBit(int portnum, SMALLINT sendbit)
{
   if(ULevel!=MODE_NORMAL)
      owLevel(portnum, MODE_NORMAL);

   OWA = OW_CONTROL;
   OWD = (OWD|0x20); // enable bit_ctl
   
   OWA = OW_BUFFER;
   OWD = (sendbit&0x01);
   
   usDelay(80);
   
   return (OWD&0x01);
}

//--------------------------------------------------------------------------
// Send 8 bits of communication to the 1-Wire Net and return the
// result 8 bits read from the 1-Wire Net.  The parameter 'sendbyte'
// least significant 8 bits are used and the least significant 8 bits
// of the result is the return byte.
//
// 'portnum'    - number 0 to MAX_PORTNUM-1.  This number is provided to
//                indicate the symbolic port number.
// 'sendbyte'   - 8 bits to send (least significant byte)
//
// Returns:  8 bits read from sendbyte
//
SMALLINT owTouchByte(int portnum, SMALLINT sendbyte)
{
   if(ULevel!=MODE_NORMAL)
      owLevel(portnum, MODE_NORMAL);

   OWA = OW_CONTROL;
   OWD = (OWD&(~0x20)); // disable bit_ctl

   OWA = OW_BUFFER;
   OWD = sendbyte;
   
   usDelay(640);
   
   return OWD;
}

//--------------------------------------------------------------------------
// Send 8 bits of communication to the 1-Wire Net and verify that the
// 8 bits read from the 1-Wire Net is the same (write operation).
// The parameter 'sendbyte' least significant 8 bits are used.
//
// 'portnum'    - number 0 to MAX_PORTNUM-1.  This number is provided to
//                indicate the symbolic port number.
// 'sendbyte'   - 8 bits to send (least significant byte)
//
// Returns:  TRUE: bytes written and echo was the same
//           FALSE: echo was not the same
//
SMALLINT owWriteByte(int portnum, SMALLINT sendbyte)
{
   return (owTouchByte(portnum,sendbyte) == sendbyte) ? TRUE : FALSE;
}

//--------------------------------------------------------------------------
// Send 8 bits of read communication to the 1-Wire Net and and return the
// result 8 bits read from the 1-Wire Net.
//
// 'portnum'    - number 0 to MAX_PORTNUM-1.  This number is provided to
//                indicate the symbolic port number.
//
// Returns:  8 bytes read from 1-Wire Net
//
SMALLINT owReadByte(int portnum)
{
   return owTouchByte(portnum,0xFF);
}

//--------------------------------------------------------------------------
// Set the 1-Wire Net communucation speed.
//
// 'portnum'    - number 0 to MAX_PORTNUM-1.  This number is provided to
//                indicate the symbolic port number.
// 'new_speed'  - new speed defined as
//                SPEED_NORMAL     0x00
//                SPEED_OVERDRIVE  0x01
//
// Returns:  current 1-Wire Net speed
//
SMALLINT owSpeed(int portnum, SMALLINT new_speed)
{
   // not supported, only normal speed
   return USpeed;
}

//--------------------------------------------------------------------------
// Set the 1-Wire Net line level.  The values for NewLevel are
// as follows:
//
// 'portnum'    - number 0 to MAX_PORTNUM-1.  This number is provided to
//                indicate the symbolic port number.
// 'new_level'  - new level defined as
//                MODE_NORMAL     0x00
//                MODE_STRONG5    0x02
//                MODE_PROGRAM    0x04
//                MODE_BREAK      0x08
//
// Returns:  current 1-Wire Net level
//
// Note: Strong and Program not supported on 550 target.
SMALLINT owLevel(int portnum, SMALLINT new_level)
{
   switch(new_level)
   {
      case MODE_BREAK:
         OWA = OW_CONTROL;
         OWD = (OWD | 0x04); // set EN_FOW
         OWA = OW_COMMAND;
         OWD = (OWD | 0x04); // set FOW, to force line low
         break;
      default:
         new_level = MODE_NORMAL;
      case MODE_NORMAL:
         OWA = OW_CONTROL;
         OWD = (OWD & (~0x04)); // clear EN_FOW
         break;
   }
   ULevel = new_level;
   return new_level;
}

//--------------------------------------------------------------------------
// This procedure creates a fixed 480 microseconds 12 volt pulse
// on the 1-Wire Net for programming EPROM iButtons.
//
// 'portnum'    - number 0 to MAX_PORTNUM-1.  This number is provided to
//                indicate the symbolic port number.
//
// Returns:  TRUE  successful
//           FALSE program voltage not available
//
SMALLINT owProgramPulse(int portnum)
{
   // Not supported
   return 0;
}

//--------------------------------------------------------------------------
//  Description:
//     Delay for at least 'len' ms.  Works only at 13.5MHz.
//
void msDelay(int delay)
{
   while(delay--)
   {
      usDelay(1000);
   }
}

//--------------------------------------------------------------------------
//  Description:
//     Delay for at least 'len' us.  Works only at 13.5MHz.
//
void usDelay(unsigned int delay)
{
   // bug: with fastcall=auto, compiler doesn't move argument
   // into loop counter variable.  This does it manually.
   asm("MOVE LC[1], A[0]");
   
   do
   {
      asm("nop");
      asm("nop");
      asm("nop");
      asm("nop");
      asm("nop");
      asm("nop");
      asm("nop");
      asm("nop");
      asm("nop");
      asm("nop");
      asm("nop");
      asm("nop");
      asm("nop");
   }
   while(--delay);
}

//--------------------------------------------------------------------------
// Get the current millisecond tick count.  Does not have to represent
// an actual time, it just needs to be an incrementing timer.
//
long msGettick(void)
{
   // not fully supported yet
   // returns timer 1, which is in 8-bit auto-reload (presumably) for the
   // serial port.  may not be good enough.
   return ++counter;
}

//--------------------------------------------------------------------------
// Send 8 bits of communication to the 1-Wire Net and verify that the
// 8 bits read from the 1-Wire Net is the same (write operation).
// The parameter 'sendbyte' least significant 8 bits are used.  After the
// 8 bits are sent change the level of the 1-Wire net.
//
// 'portnum'  - number 0 to MAX_PORTNUM-1.  This number was provided to
//              OpenCOM to indicate the port number.
// 'sendbyte' - 8 bits to send (least significant byte)
//
// Returns:  TRUE: bytes written and echo was the same
//           FALSE: echo was not the same
//
SMALLINT owWriteBytePower(int portnum, SMALLINT sendbyte)
{
   // not supported on the by the MAXQ20
   return FALSE;
}

//--------------------------------------------------------------------------
// Send 1 bit of communication to the 1-Wire Net and verify that the
// response matches the 'applyPowerResponse' bit and apply power delivery
// to the 1-Wire net.  Note that some implementations may apply the power
// first and then turn it off if the response is incorrect.
//
// 'portnum'  - number 0 to MAX_PORTNUM-1.  This number was provided to
//              OpenCOM to indicate the port number.
// 'applyPowerResponse' - 1 bit response to check, if correct then start
//                        power delivery
//
// Returns:  TRUE: bit written and response correct, strong pullup now on
//           FALSE: response incorrect
//
SMALLINT owReadBitPower(int portnum, SMALLINT applyPowerResponse)
{
   // not supported on the by the MAXQ20
   return FALSE;
}

//--------------------------------------------------------------------------
// This procedure indicates wether the adapter can deliver power.
//
// 'portnum'  - number 0 to MAX_PORTNUM-1.  This number was provided to
//              OpenCOM to indicate the port number.
//
// Returns:  TRUE  if adapter is capable of delivering power.
//
SMALLINT owHasPowerDelivery(int portnum)
{
   // add adapter specific code here
   return FALSE;
}

//--------------------------------------------------------------------------
// This procedure indicates wether the adapter can deliver power.
//
// 'portnum'  - number 0 to MAX_PORTNUM-1.  This number was provided to
//              OpenCOM to indicate the port number.
//
// Returns:  TRUE  if adapter is capable of over drive.
//
SMALLINT owHasOverDrive(int portnum)
{
   // add adapter specific code here
   return TRUE;
}

//--------------------------------------------------------------------------
// This procedure creates a fixed 480 microseconds 12 volt pulse
// on the 1-Wire Net for programming EPROM iButtons.
//
// 'portnum'  - number 0 to MAX_PORTNUM-1.  This number was provided to
//              OpenCOM to indicate the port number.
//
// Returns:  TRUE  program volatage available
//           FALSE program voltage not available
SMALLINT owHasProgramPulse(int portnum)
{
   // add adapter specific code here
   return FALSE;
}



//--------------------------------------------------------------------------
// Micro-specific main function.  Sinse demo apps all have main with
// signature (int,char**), but micros don't typically run with cmd-line
// options, this main function fakes it and calls the demo app main.
//--------------------------------------------------------------------------

extern int micro_main(int, char*[]);
#ifdef main
#undef main
#endif
void main(void)
{
   char* args[2] = {"micro", "micro"};
   micro_main(2, args);
}

