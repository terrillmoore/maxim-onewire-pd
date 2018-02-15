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
//--------------------------------------------------------------------------
//
//  swt29.c - Does operations to the DS23E04
//  version 3.00
//

// Include Files
#include "ownet.h"
#include "swt1C.h"

//--------------------------------------------------------------------------
// SUBROUTINE - SetSwitch1C
//
// This routine turns the device on or off.
//
// 'portnum'  - number 0 to MAX_PORTNUM-1.  This number was provided to
//              OpenCOM to indicate the port number.
// 'SwNum'    - The serial number of the swith that is to be turned on or off
// 'channel'  - The channel the switch is suppose to be set on.
// 'seton'    - 'TRUE' then it is turned on 
//              'FALSE' turns it off 
//
// Returns: TRUE(1):    If set is successful
//          FALSE(0):   If set is not successful
//
SMALLINT setSwitch1C(int portnum, uchar *SNum, uchar *state)
{
   uchar buff[5];
   int i;

   owSerialNum(portnum,&SNum[0], FALSE);

   if(owAccess(portnum))
   {
      buff[0] = 0x5A;
      buff[1] = state[1];
      buff[2] = (uchar) ~state[1];
      for(i=0;i<2;i++)
         buff[i+3] = 0xFF;

      if(!owBlock(portnum,FALSE,&buff[0],5))
      {
         OWERROR(OWERROR_BLOCK_FAILED);
         return FALSE;
      }

      if((buff[3] != 0xAA))
      {
         OWERROR(OWERROR_LATCH_NOT_SET);
         return FALSE;
      }
	   else
      {
		   state[0] = buff[4];
      }

   }

   return TRUE;
}

//--------------------------------------------------------------------------
// SUBROUTINE - ReadSwitch1C
//
// This routine reads the state of the DS2408.
//
// 'portnum'    - number 0 to MAX_PORTNUM-1.  This number was provided to
//                OpenCOM to indicate the port number.
// 'SNum'       - The Serial number of the DS2405 that info is requested on
// 'state'      - This is the state information of the DS2408.
//
// Returns: TRUE(1):    If device is found and active
//          FALSE(0):   If device is not found and not active or not there
//
SMALLINT readSwitch1C(int portnum, uchar *SNum, uchar *state)
{
   uchar buff[6];
   int i;

   owSerialNum(portnum,&SNum[0], FALSE);

   if(owAccess(portnum))
   {
      buff[0] = 0xF0;

      // address 1
      buff[1] = (uchar) (544 & 0xFF);
      // address 2
      buff[2] = (uchar) (((544 & 0xFFFF) >> 8) & 0xFF);

      for(i=0;i<3;i++)
         buff[i+3] = 0xFF;

      if(!owBlock(portnum,FALSE,&buff[0],6))
      {
         OWERROR(OWERROR_BLOCK_FAILED);
         return FALSE;
      }

      for(i=0;i<3;i++)
         state[i] = buff[i+3];
   }

   return TRUE;
}

// SUBROUTINE - setResetMode1C
//
// Turns the Reset mode on/off.
//
// 'portnum'   - number 0 to MAX_PORTNUM-1.  This number was provided to
//               OpenCOM to indicate the port number.
// 'SNum'      - The Serial number of the DS2405 that info is requested on
// 'seton'     - 'TRUE' set reset mode, 'FALSE' turn off reset mode
//
// Returns: TRUE(1):    If operation worked
//          FALSE(0):   If an error occured
//
SMALLINT setResetMode1C(int portnum, uchar* SNum, SMALLINT seton)
{
   uchar reg[3];

   if(readRegister1C(portnum,SNum,&reg[0]))
   {
      if(seton && ((reg[2] & 0x08) == 0x08))
      {
         reg[2] = (uchar) (reg[2] & 0xF7);
      }
      else if((!seton) && ((reg[2] & 0x08) == 0x00))
      {
         reg[2] = (reg[2] | 0x08);
      }

      if(!setRegister1C(portnum,SNum,&reg[0]))
         return FALSE;
   }
   else
      return FALSE;

   return TRUE;
}

// SUBROUTINE - getVCC1C
//
// Retrieves the state of the VCC pin.  If the pin is powered 'TRUE' is
// returned else 'FALSE' is returned if the pin is grounded.
//
// 'reg'   - register value that was read.
//
// Returns: TRUE(1):    If VCC is powered
//          FALSE(0):   If VCC is grounded
//
SMALLINT getVCC1C(uchar *reg)
{
   if((reg[2] & 0x80) == 0x80)
      return TRUE;

   return FALSE;
}

// SUBROUTINE - clearPowerOnReset1C
//
// Checks if the Power On Reset if on and if so clears it.
//
// 'portnum'   - number 0 to MAX_PORTNUM-1.  This number was provided to
//               OpenCOM to indicate the port number.
// 'SNum'      - The Serial number of the DS2405 that info is requested on
//
// Returns: TRUE(1):    If cleared
//          FALSE(0):   If an error occured
//
SMALLINT clearPowerOnReset1C(int portnum, uchar *SNum)
{
   uchar reg[3];

   if(!readRegister1C(portnum,SNum,&reg[0]))
      return FALSE;

   if((reg[2] & 0x08) == 0x08)
   {
      reg[2] = (reg[2] & 0xF7);
   }

   if(!setRegister1C(portnum,SNum,&reg[0]))
      return FALSE;

   return TRUE;
}

// SUBROUTINE - orConditionalSearch1C
//
// Checks if the 'or' Condition Search is set and if not sets it.
//
// 'portnum'   - number 0 to MAX_PORTNUM-1.  This number was provided to
//               OpenCOM to indicate the port number.
// 'SNum'      - The Serial number of the DS2405 that info is requested on
//
// Returns: TRUE(1):    If set
//          FALSE(0):   If an error occured
//
SMALLINT orConditionalSearch1C(int portnum, uchar *SNum)
{
   uchar reg[3];

   if(!readRegister1C(portnum,SNum,&reg[0]))
      return FALSE;

   if((reg[2] & 0x02) == 0x02)
   {
      reg[2] = (reg[2] & 0xFD);

      if(!setRegister1C(portnum,SNum,&reg[0]))
         return FALSE;
   }

   return TRUE;
}

// SUBROUTINE - andConditionalSearch1C
//
// Checks if the 'and' Conditional Search is set and if not sets it.
//
// 'portnum'   - number 0 to MAX_PORTNUM-1.  This number was provided to
//               OpenCOM to indicate the port number.
// 'SNum'      - The Serial number of the DS2405 that info is requested on
//
// Returns: TRUE(1):    If set
//          FALSE(0):   If an error occured
//
SMALLINT andConditionalSearch1C(int portnum, uchar *SNum)
{
   uchar reg[3];

   if(!readRegister1C(portnum,SNum,&reg[0]))
      return FALSE;

   if((reg[2] & 0x02) != 0x02)
   {
      reg[2] = (reg[2] | 0x02);

      if(!setRegister1C(portnum,SNum,&reg[0]))
         return FALSE;
   }

   return TRUE;
}


// SUBROUTINE - setChannelMask1C
//
// Sets the channel passed to the proper state depending on the set parameter for
//    responding to the Conditional Search.
//
// 'portnum'   - number 0 to MAX_PORTNUM-1.  This number was provided to
//               OpenCOM to indicate the port number.
// 'SNum'      - The Serial number of the DS2405 that info is requested on
// 'channel'   - channel to set
// 'seton'     - 'TRUE' turn channel on, 'FALSE' turn channel off
//
// Returns: TRUE(1):    If set
//          FALSE(0):   If an error occured
//
SMALLINT setChannelMask1C(int portnum, uchar *SNum, int channel, SMALLINT seton)
{
   uchar reg[3];
   uchar mask;

   mask = (uchar) (0x01 << channel);

   if(!readRegister1C(portnum,SNum,&reg[0]))
      return FALSE;

   if(seton)
      reg[0] = (uchar) (reg[0] | mask);
   else
      reg[0] = (uchar) (reg[0] & ~mask);

   if(!setRegister1C(portnum,SNum,&reg[0]))
      return FALSE;

   return TRUE;
}

// SUBROUTINE - setChannelPolarity1C
//
// Sets the channel passed to the proper state depending on the set parameter for
//    responding to the Conditional Search.
//
// 'portnum'   - number 0 to MAX_PORTNUM-1.  This number was provided to
//               OpenCOM to indicate the port number.
// 'SNum'      - The Serial number of the DS2405 that info is requested on
// 'channel'   - channel to set
// 'seton'     - 'TRUE' turn channel on for polarity condition search, 
//               'FALSE' turn channel off
//
// Returns: TRUE(1):    If set
//          FALSE(0):   If an error occured
//
SMALLINT setChannelPolarity1C(int portnum, uchar *SNum, int channel, SMALLINT seton)
{
   uchar reg[3];
   uchar polarity;

   polarity = (uchar) (0x01 << channel);;

   if(!readRegister1C(portnum,SNum,&reg[0]))
      return FALSE;

   if(seton)
      reg[1] = (uchar) (reg[1] | polarity);
   else
      reg[1] = (uchar) (reg[1] & ~polarity);

   if(!setRegister1C(portnum,SNum,&reg[0]))
      return FALSE;

   return TRUE;
}

// SUBROUTINE - getChannelMask1C
//
// Retrieves the information if the channel is masked for the Conditional Search.
//
// 'portnum'   - number 0 to MAX_PORTNUM-1.  This number was provided to
//               OpenCOM to indicate the port number.
// 'SNum'      - The Serial number of the DS2405 that info is requested on
// 'channel'   - Channel to look for mask
//
// Returns: TRUE(1):    If channel is masked
//          FALSE(0):   otherwise
//
SMALLINT getChannelMask1C(int portnum, uchar *SNum, int channel)
{
   uchar reg[3];
   uchar mask;

   if(!readRegister1C(portnum,SNum,&reg[0]))
      return FALSE;

   mask = (uchar) (0x01 << channel);

   return ((reg[0] & mask) == mask);
}

// SUBROUTINE - getChannelPolarity1C
//
// Retrieves the polarity of the channel for the Conditional Search.
//
// 'portnum'   - number 0 to MAX_PORTNUM-1.  This number was provided to
//               OpenCOM to indicate the port number.
// 'SNum'      - The Serial number of the DS2405 that info is requested on
// 'channel'   - Channel to look for polarity
//
// Returns: TRUE(1):    If channel polarity is set
//          FALSE(0):   otherwise
//
SMALLINT getChannelPolarity1C(int portnum, uchar *SNum, int channel)
{
   uchar reg[3];
   uchar polarity;

   if(!readRegister1C(portnum,SNum,&reg[0]))
      return FALSE;

   polarity = (uchar) (0x01 << channel);

   return ((reg[1] & polarity) == polarity);
}



//--------------------------------------------------------------------------
// SUBROUTINE - setRegister1C
//
// This routine set the register values
//
// 'portnum'  - number 0 to MAX_PORTNUM-1.  This number was provided to
//              OpenCOM to indicate the port number.
// 'SNum'     - The serial number of the swith that is to be turned on or off
// 'reg'      - register values to be set.
//
// Returns: TRUE(1):    If set is successful
//          FALSE(0):   If set is not successful
//
SMALLINT setRegister1C(int portnum, uchar *SNum, uchar *reg)
{
   uchar buff[6];
   int i;

   owSerialNum(portnum,&SNum[0], FALSE);

   if(owAccess(portnum))
   {
      buff[0] = 0xCC;
      // address 1
      buff[1] = (uchar) (547 & 0xFF);
      // address 2
      buff[2] = (uchar) (((547 & 0xFFFF) >> 8) & 0xFF);

      for(i=0;i<3;i++)
         buff[i+3] = reg[i];

      if(!owBlock(portnum,FALSE,&buff[0],6))
      {
         OWERROR(OWERROR_BLOCK_FAILED);
         return FALSE;
      }
   }

   return TRUE;
}

//--------------------------------------------------------------------------
// SUBROUTINE - readRegister1C
//
// This routine reads the register values.
//
// 'portnum' - number 0 to MAX_PORTNUM-1.  This number was provided to
//             OpenCOM to indicate the port number.
// 'SNum'    - The Serial number of the DS2405 that info is requested on
// 'reg'     - The register values that were read.
//
// Returns: TRUE(1):    If read was successful.
//          FALSE(0):   Otherwise
//
SMALLINT readRegister1C(int portnum, uchar *SNum, uchar *reg)
{
   uchar buff[6];
   int i;

   owSerialNum(portnum,&SNum[0], FALSE);

   if(owAccess(portnum))
   {
      buff[0] = 0xF0;

      // address 1
      buff[1] = (uchar) (547 & 0xFF);
      // address 2
      buff[2] = (uchar) (((547 & 0xFFFF) >> 8) & 0xFF);

      for(i=0;i<3;i++)
         buff[i+3] = 0xFF;

      if(!owBlock(portnum,FALSE,&buff[0],6))
      {
         OWERROR(OWERROR_BLOCK_FAILED);
         return FALSE;
      }

      for(i=0;i<3;i++)
         reg[i] = buff[i+3];
   }

   return TRUE;
}

//--------------------------------------------------------------------------
// SUBROUTINE - getLatchState
//
// Checks the latch state of the indicated channel.
//
// 'channel' - the channel to check the latch state
// 'state'   - array of state variables that has already been read.
//
// Returns: TRUE(1):    If latch is set
//          FALSE(0):   If latch is not set
//
SMALLINT getLatchState1C(int channel, uchar *state)
{
   uchar latch = (uchar) (0x01 << channel);
   return ((state [1] & latch) == latch);
}

//--------------------------------------------------------------------------
// SUBROUTINE - getLevel
//
// Checks the sensed level on the indicated channel.
//
// 'channel' - the channel to check the latch state
// 'state'   - array of state variables that has already been read.
//
// Returns: TRUE(1):    If level is high
//          FALSE(0):   If level is low
//
SMALLINT getLevel1C(int channel, uchar *state)
{
   uchar  level = (uchar) (0x01 << channel);
   return ((state[0] & level) == level);
}

//--------------------------------------------------------------------------
// SUBROUTINE - getSensedActivity
//
// Checks if the indicated channel has experienced activity.
//
// 'channel' - the channel to check the latch state
// 'state'   - array of state variables that has already been read.
//
// Returns: TRUE(1):    If activity was detected
//          FALSE(0):   If no activity was detected
//
SMALLINT getSensedActivity1C(int channel, uchar *state)
{
   uchar activity = (uchar) (0x01 << channel);
   return ((state[2] & activity) == activity);
}

//--------------------------------------------------------------------------
// SUBROUTINE - setLatchState
//
// Sets the channel to the state provided.
//
// 'portnum' - number 0 to MAX_PORTNUM-1.  This number was provided to
//             OpenCOM to indicate the port number.
// 'SNum'    - The Serial number of the DS2405 that info is requested on
// 'channel' - the channel to check the latch state
// 'set'     - to set the channel on(1) or off(0)
//
// Returns: TRUE(1):    If latch was set
//          FALSE(0):   If an error occured
//
SMALLINT setLatchState1C(int portnum, uchar *SNum, int channel, uchar set)
{
   uchar state[3];
   uchar latch = (uchar) (0x01 << channel);

   if(readSwitch1C(portnum,SNum,&state[0]))
   {
      if(set)
         state[1] = (uchar) (state[1] | latch);
      else
         state[1] = (uchar) (state[1] & ~latch);

      if(setSwitch1C(portnum,SNum,&state[0]))
         return TRUE;
   }

   return FALSE;
}

//--------------------------------------------------------------------------
// SUBROUTINE - read1C
//
// Reads data from memory
//
// 'portnum' - number 0 to MAX_PORTNUM-1.  This number was provided to
//             OpenCOM to indicate the port number.
// 'SNum'    - The Serial number of the DS2405 that info is requested on
// 'address' - address to start reading
// 'data'    - storage array for the data read.
//
// Returns: TRUE(1):    If read was completed
//          FALSE(0):   If an error occured
//
SMALLINT read1C(int portnum, uchar *SNum, int address, int len, uchar *data)
{
   uchar buff[259];
   int i;

   if((address+len) > 550)
   {
      OWERROR(OWERROR_READ_OUT_OF_RANGE);
      return FALSE;
   }

   owSerialNum(portnum,&SNum[0], FALSE);

   if(owAccess(portnum))
   {
      buff[0] = 0xF0;
      buff[1] = (uchar) (address & 0xFF);
      buff[2] = (uchar) (((address & 0xFFFF) >> 8) & 0xFF);
      for(i=0;i<len;i++)
         buff[i+3] = 0xFF;

      if(!owBlock(portnum,FALSE,&buff[0],len+3))
      {
         OWERROR(OWERROR_BLOCK_FAILED);
         return FALSE;
      }
      for(i=0;i<len;i++)
         data[i] = buff[i+3];
   }

   return TRUE;
}

//--------------------------------------------------------------------------
// SUBROUTINE - writeScratch1C
//
// Writes data to the scratchpad
//
// 'portnum' - number 0 to MAX_PORTNUM-1.  This number was provided to
//             OpenCOM to indicate the port number.
// 'SNum'    - The Serial number of the DS2405 that info is requested on
// 'address' - address to write data
// 'len'     - length of data to be written to scratchpad
// 'data'    - storage array for the data to be written
//
// Returns: TRUE(1):    If write scratchpad was completed
//          FALSE(0):   If an error occured
//
SMALLINT writeScratch1C(int portnum, uchar *SNum, int address, int len, uchar *data)
{
   uchar buff[37];
   ushort lastcrc16;
   int i;

   owSerialNum(portnum,&SNum[0], FALSE);

   if(owAccess(portnum))
   {
      buff[0] = 0x0F;
      buff[1] = (uchar) (address & 0xFF);
      buff[2] = (uchar) (((address & 0xFFFF) >> 8) & 0xFF);
      for(i=0;i<len;i++)
         buff[i+3] = data[i];
      if(len == 32)
      {
         buff[len+3] = 0xFF;
         buff[len+4] = 0xFF;
      }

      if(len == 32)
      {
         if(!owBlock(portnum,FALSE,&buff[0],len+5))
         {
            OWERROR(OWERROR_BLOCK_FAILED);
            return FALSE;
         }
      }
      else
      {
         if(!owBlock(portnum,FALSE,&buff[0],len+3))
         {
            OWERROR(OWERROR_BLOCK_FAILED);
            return FALSE;
         }
      }

      if(len == 32)
      {
         setcrc16(portnum,0);

         for(i=0;i<len+5;i++)
         {
            lastcrc16 = docrc16(portnum,buff[i]);
         }
         if(lastcrc16 != 0xB001)
         {
            OWERROR(OWERROR_CRC_FAILED);
            return FALSE;
         }
      }

   }

   return TRUE;
}

//--------------------------------------------------------------------------
// SUBROUTINE - read1C
//
// Reads data from the scratchpad
//
// 'portnum' - number 0 to MAX_PORTNUM-1.  This number was provided to
//             OpenCOM to indicate the port number.
// 'SNum'    - The Serial number of the DS2405 that info is requested on
// 'len'     - length of data in the scratchpad
// 'es'      - es byte read from scratchpad
// 'addr'    - address that was read from the scratchpad
// 'data'    - storage array for the data read from the scratchpad
//
// Returns: TRUE(1):    If read was completed
//          FALSE(0):   If an error occured
//
SMALLINT readScratch1C(int portnum, uchar *SNum, int *len, uchar *es, uchar *addr, uchar *data)
{
   uchar buff[38];
   ushort lastcrc16;
   int i,j;

   owSerialNum(portnum,&SNum[0], FALSE);

   if(owAccess(portnum))
   {

      buff[0] = 0xAA;
      for(i=0;i<37;i++)
         buff[i+1] = 0xFF;

      if(!owBlock(portnum,FALSE,&buff[0],38))
      {
         OWERROR(OWERROR_BLOCK_FAILED);
         return FALSE;
      }

      setcrc16(portnum,0);
      for(i=0;i<38;i++)
      {
         lastcrc16 = docrc16(portnum,buff[i]);

         if(lastcrc16 == 0xB001)
         {
            addr[0] = buff[1];
            addr[1] = buff[2];
            *es = buff[3];
            *len = i-5;
            for(j=0;j<i-5;j++)
            {
               data[j] = buff[j+4];
            }

            return TRUE;
         }
      }
   }

   return FALSE;
}

//--------------------------------------------------------------------------
// SUBROUTINE - copyScratch1C
//
// Copies the data from the scratchpad to memory
//
// 'portnum' - number 0 to MAX_PORTNUM-1.  This number was provided to
//             OpenCOM to indicate the port number.
// 'SNum'    - The Serial number of the DS2405 that info is requested on
//
// Returns: TRUE(1):    If copy was completed
//          FALSE(0):   If an error occured
//
SMALLINT copyScratch1C(int portnum, uchar *SNum)
{
   uchar buffer[4];
   uchar data[32];
   uchar es=0x00;
   uchar addr[2];
   uchar reg[3];
   SMALLINT pullup;
   int len;

   if(!readScratch1C(portnum,SNum,&len,&es,&addr[0],&data[0]))
      return FALSE;

   if(!readRegister1C(portnum,SNum,&reg[0]))
      return FALSE;

   pullup = getVCC1C(&reg[0]);

   owSerialNum(portnum,SNum, FALSE);

   if(owAccess(portnum))
   {
      buffer[0] = 0x55;
      buffer[1] = addr[0];
      buffer[2] = addr[1];
      buffer[3] = es;

      if(!pullup)
      {
         if(!owBlock(portnum,FALSE,&buffer[0],3))
         {
            OWERROR(OWERROR_BLOCK_FAILED);
            return FALSE;
         }

         // send last byte and enable strong pullup
         if (!owWriteBytePower(portnum, buffer[3]))
         {
            OWERROR(OWERROR_WRITE_BYTE_FAILED);
            return FALSE;
         }

         // delay for copy to complete
         msDelay(10);

         // turn off strong pullup
         owLevel(portnum, MODE_NORMAL);

      }
      else
      {
         if(!owBlock(portnum,FALSE,&buffer[0],4))
         {
            OWERROR(OWERROR_BLOCK_FAILED);
            return FALSE;
         }

      }

      if(owReadByte(portnum) != 0xAA)
      {
         OWERROR(OWERROR_COPY_SCRATCHPAD_NOT_FOUND);
         return FALSE;
      }

   }

   printf("ES byte before copy command: %02X\n",es);
   if(!readScratch1C(portnum,SNum,&len,&es,&addr[0],&data[0]))
      return FALSE;
   printf("ES byte after copy command:  %02X\n",es);

   return TRUE;
}
