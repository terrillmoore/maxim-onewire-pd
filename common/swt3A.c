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
//  swt3A.c - Does access read/write operations for the DS2413
//  version 3.10
//

// Include Files
#include "ownet.h"
#include "swt3A.h"


//--------------------------------------------------------------------------
// SUBROUTINE - owAccessRead
//
// This routine reads the PIO status bit assignment.
//
// 'portnum'  - number 0 to MAX_PORTNUM-1.  This number was provided to
//              OpenCOM to indicate the port number.
// 'SNum'     - The serial number of the swith that is to be turned on or off
// 'reset'    - If a reset occured and the PIO access read command needs to be
//              issued.
// 'state'    - The PIO Status Bit Assignment.
//
// Returns: TRUE(1):    If set is successful
//          FALSE(0):   If set is not successful
//
SMALLINT owAccessRead(int portnum, uchar *SNum, SMALLINT reset)
{
   uchar buff[2];
   uchar state;

   if(reset)
   {
      owSerialNum(portnum,&SNum[0],FALSE);

      if(owAccess(portnum))
      {
         buff[0] = 0xF5;  // PIO Access Read Command
         buff[1] = 0xFF;  // Used to read the PIO Status Bit Assignment

         if(!owBlock(portnum,FALSE,&buff[0],2))
         {
            OWERROR(OWERROR_BLOCK_FAILED);
            return FALSE;
         }

         state = buff[1];
      }
      else
      {
         OWERROR(OWERROR_DEVICE_SELECT_FAIL);
         return FALSE;
      }
   }
   else
   {
      state = (uchar)owReadByte(portnum);
   }

   return state;
}

//--------------------------------------------------------------------------
// SUBROUTINE - owAccessWrite
//
// This routine reads the PIO status bit assignment.
//
// 'portnum'  - number 0 to MAX_PORTNUM-1.  This number was provided to
//              OpenCOM to indicate the port number.
// 'SNum'     - The serial number of the swith that is to be turned on or off
// 'reset'    - If a reset occured and the PIO access read command needs to be
//              issued.
// 'state'    - The PIO Status Bit Assignment.
//
// Returns: TRUE(1):    If set is successful
//          FALSE(0):   If set is not successful
//
SMALLINT owAccessWrite(int portnum, uchar *SNum, SMALLINT reset, uchar state)
{
   uchar buff[5];
   uchar wrByte;

   wrByte = state | 0xFC;

   if(reset)
   {
      owSerialNum(portnum,&SNum[0],FALSE);

      if(owAccess(portnum))
      {
         buff[0] = 0x5A;    // PIO Access Write Command
         buff[1] = wrByte;  // Channel write information
         buff[2] = ~wrByte; // Inverted write byte
         buff[3] = 0xFF;    // Confirmation Byte
         buff[4] = 0xFF;    // PIO Pin Status

         if(!owBlock(portnum,FALSE,&buff[0],5))
         {
            OWERROR(OWERROR_BLOCK_FAILED);
            return FALSE;
         }

         if(buff[3] != 0xAA)  
         {
            return FALSE;
         }
      }
      else
      {
         OWERROR(OWERROR_DEVICE_SELECT_FAIL);
         return FALSE;
      }
   }
   else
   {
      buff[1] = wrByte;  // Channel write information
      buff[2] = ~wrByte; // Inverted write byte
      buff[3] = 0xFF;    // Confirmation Byte
      buff[4] = 0xFF;    // PIO Pin Status

      if(!owBlock(portnum,FALSE,&buff[0],5))
      {
         OWERROR(OWERROR_BLOCK_FAILED);
         return FALSE;
      }

      if(buff[3] != 0xAA)  
      {
         return FALSE;
      }
   }

   return TRUE;
}


