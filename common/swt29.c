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
//  swt29.c - Does operations to the DS2408
//  version 3.00
//

// Include Files
#include "ownet.h"
#include "swt29.h"

//--------------------------------------------------------------------------
// SUBROUTINE - SetSwitch05
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
SMALLINT setSwitch29(int portnum, uchar *SNum, uchar *state)
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
   }

   return TRUE;
}

//--------------------------------------------------------------------------
// SUBROUTINE - ReadSwitch05
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
SMALLINT readSwitch29(int portnum, uchar *SNum, uchar *state)
{
   uchar buff[32];
   int i;

   owSerialNum(portnum,&SNum[0], FALSE);

   if(owAccess(portnum))
   {
      buff[0] = 0xF0;

      // address 1
      buff[1] = (uchar) (136 & 0xFF);
      // address 2
      buff[2] = (uchar) (((136 & 0xFFFF) >> 8) & 0xFF);

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

// SUBROUTINE - setResetMode
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
SMALLINT setResetMode(int portnum, uchar* SNum, SMALLINT seton)
{
   uchar reg[3];

   if(readRegister29(portnum,SNum,&reg[0]))
   {
      if(seton && ((reg[2] & 0x04) == 0x04))
      {
         reg[2] = (uchar) (reg[2] & 0xFB);
      }
      else if((!seton) && ((reg[2] & 0x04) == 0x00))
      {
         reg[2] = (reg[2] | 0x04);
      }

      if(!setRegister29(portnum,SNum,&reg[0]))
         return FALSE;
   }
   else
      return FALSE;

   return TRUE;
}

// SUBROUTINE - getVCC
//
// Retrieves the state of the VCC pin.  If the pin is powered 'TRUE' is
// returned else 'FALSE' is returned if the pin is grounded.
//
// 'reg'   - register value that was read.
//
// Returns: TRUE(1):    If VCC is powered
//          FALSE(0):   If VCC is grounded
//
SMALLINT getVCC(uchar *reg)
{
   if((reg[2] & 0x80) == 0x80)
      return TRUE;

   return FALSE;
}

// SUBROUTINE - clearPowerOnReset
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
SMALLINT clearPowerOnReset(int portnum, uchar *SNum)
{
   uchar reg[3];

   if(!readRegister29(portnum,SNum,&reg[0]))
      return FALSE;

   if((reg[2] & 0x08) == 0x08)
   {
      reg[2] = (reg[2] & 0xF7);
   }

   if(!setRegister29(portnum,SNum,&reg[0]))
      return FALSE;

   return TRUE;
}

// SUBROUTINE - orConditionalSearch
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
SMALLINT orConditionalSearch(int portnum, uchar *SNum)
{
   uchar reg[3];

   if(!readRegister29(portnum,SNum,&reg[0]))
      return FALSE;

   if((reg[2] & 0x02) == 0x02)
   {
      reg[2] = (reg[2] & 0xFD);

      if(!setRegister29(portnum,SNum,&reg[0]))
         return FALSE;
   }

   return TRUE;
}

// SUBROUTINE - andConditionalSearch
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
SMALLINT andConditionalSearch(int portnum, uchar *SNum)
{
   uchar reg[3];

   if(!readRegister29(portnum,SNum,&reg[0]))
      return FALSE;

   if((reg[2] & 0x02) != 0x02)
   {
      reg[2] = (reg[2] | 0x02);

      if(!setRegister29(portnum,SNum,&reg[0]))
         return FALSE;
   }

   return TRUE;
}

// SUBROUTINE - pioConditionalSearch
//
// Checks if the 'PIO' Conditional Search is set for input and if not sets it.
//
// 'portnum'   - number 0 to MAX_PORTNUM-1.  This number was provided to
//               OpenCOM to indicate the port number.
// 'SNum'      - The Serial number of the DS2405 that info is requested on
//
// Returns: TRUE(1):    If set
//          FALSE(0):   If an error occured
//
SMALLINT pioConditionalSearch(int portnum, uchar *SNum)
{
   uchar reg[3];

   if(!readRegister29(portnum,SNum,&reg[0]))
      return FALSE;

   if((reg[2] & 0x01) == 0x01)
   {
      reg[2] = (reg[2] & 0xFE);

      if(!setRegister29(portnum,SNum,&reg[0]))
         return FALSE;
   }

   if(!setRegister29(portnum,SNum,&reg[0]))
      return FALSE;

   return TRUE;
}

// SUBROUTINE - activityConditionalSearch
//
// Checks if the activity latches are set for Conditional Search and if not sets it.
//
// 'portnum'   - number 0 to MAX_PORTNUM-1.  This number was provided to
//               OpenCOM to indicate the port number.
// 'SNum'      - The Serial number of the DS2405 that info is requested on
//
// Returns: TRUE(1):    If set
//          FALSE(0):   If an error occured
//
SMALLINT activityConditionalSearch(int portnum, uchar *SNum)
{
   uchar reg[3];

   if(!readRegister29(portnum,SNum,&reg[0]))
      return FALSE;

   if((reg[2] & 0x01) != 0x01)
   {
      reg[2] = (reg[2] | 0x01);

      if(!setRegister29(portnum,SNum,&reg[0]))
         return FALSE;
   }

   return TRUE;
}

// SUBROUTINE - setChannelMask
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
SMALLINT setChannelMask(int portnum, uchar *SNum, int channel, SMALLINT seton)
{
   uchar reg[3];
   uchar mask;

   mask = (uchar) (0x01 << channel);

   if(!readRegister29(portnum,SNum,&reg[0]))
      return FALSE;

   if(seton)
      reg[0] = (uchar) (reg[0] | mask);
   else
      reg[0] = (uchar) (reg[0] & ~mask);

   if(!setRegister29(portnum,SNum,&reg[0]))
      return FALSE;

   return TRUE;
}

// SUBROUTINE - setChannelPolarity
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
SMALLINT setChannelPolarity(int portnum, uchar *SNum, int channel, SMALLINT seton)
{
   uchar reg[3];
   uchar polarity;

   polarity = (uchar) (0x01 << channel);;

   if(!readRegister29(portnum,SNum,&reg[0]))
      return FALSE;

   if(seton)
      reg[1] = (uchar) (reg[1] | polarity);
   else
      reg[1] = (uchar) (reg[1] & ~polarity);

   if(!setRegister29(portnum,SNum,&reg[0]))
      return FALSE;

   return TRUE;
}

// SUBROUTINE - getChannelMask
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
SMALLINT getChannelMask(int portnum, uchar *SNum, int channel)
{
   uchar reg[3];
   uchar mask;

   if(!readRegister29(portnum,SNum,&reg[0]))
      return FALSE;

   mask = (uchar) (0x01 << channel);

   return ((reg[0] & mask) == mask);
}

// SUBROUTINE - getChannelPolarity
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
SMALLINT getChannelPolarity(int portnum, uchar *SNum, int channel)
{
   uchar reg[3];
   uchar polarity;

   if(!readRegister29(portnum,SNum,&reg[0]))
      return FALSE;

   polarity = (uchar) (0x01 << channel);

   return ((reg[1] & polarity) == polarity);
}



//--------------------------------------------------------------------------
// SUBROUTINE - setRegister29
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
SMALLINT setRegister29(int portnum, uchar *SNum, uchar *reg)
{
   uchar buff[6];
   int i;

   owSerialNum(portnum,&SNum[0], FALSE);

   if(owAccess(portnum))
   {
      buff[0] = 0xCC;
      buff[1] = (uchar) ((139) & 0xFF);
      buff[2] = (uchar) ((((139) & 0xFFFF) >> 8) & 0xFF);

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
// SUBROUTINE - readRegister29
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
SMALLINT readRegister29(int portnum, uchar *SNum, uchar *reg)
{
   uchar buff[6];
   int i;

   owSerialNum(portnum,&SNum[0], FALSE);

   if(owAccess(portnum))
   {
      buff[0] = 0xF0;

      // address 1
      buff[1] = (uchar) (139 & 0xFF);
      // address 2
      buff[2] = (uchar) (((139 & 0xFFFF) >> 8) & 0xFF);

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
SMALLINT getLatchState(int channel, uchar *state)
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
SMALLINT getLevel(int channel, uchar *state)
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
SMALLINT getSensedActivity (int channel, uchar *state)
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
SMALLINT setLatchState(int portnum, uchar *SNum, int channel, uchar set)
{
   uchar state[3];
   uchar latch = (uchar) (0x01 << channel);

   if(readSwitch29(portnum,SNum,&state[0]))
   {
      if(set)
         state[1] = (uchar) (state[1] | latch);
      else
         state[1] = (uchar) (state[1] & ~latch);

      if(setSwitch29(portnum,SNum,&state[0]))
         return TRUE;
   }

   return FALSE;
}


