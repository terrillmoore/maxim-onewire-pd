//---------------------------------------------------------------------------
// Copyright (C) 1999 Dallas Semiconductor Corporation, All Rights Reserved.
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
//  DS550SES.C - Acquire and release a Session for general 1-Wire Net
//              library
//
//  Version: 2.00
//           1.03 -> 2.00  Changed 'MLan' to 'ow'. Added support for
//                         multiple ports.
//

#include "ownet.h"
#include <iomaxq200x.h>

#define OW_COMMAND    0x00
#define OW_BUFFER     0x01
#define OW_INTERRUPT  0x02
#define OW_INT_ENABLE 0x03
#define OW_CLOCK      0x04
#define OW_CONTROL    0x05

// local function prototypes
SMALLINT owAcquire(int,char *);
int      owAcquireEx(char *);
void     owRelease(int);

//---------------------------------------------------------------------------
// Attempt to acquire a 1-Wire net
//
// 'portnum'    - number 0 to MAX_PORTNUM-1.  This number is provided to
//                indicate the symbolic port number.
// 'port_zstr'  - zero terminated port name.
//
// Returns: TRUE - success, port opened
//
SMALLINT owAcquire(int portnum, char *port_zstr)
{
   OWA = OW_CLOCK;
   OWD = 0x89; // set the clock divider
   
   OWA = OW_CONTROL;
   OWD = 0x00; // clear the control register

   // checks to make sure the line is idling high.
   return (TRUE);
}

//---------------------------------------------------------------------------
// Attempt to acquire a 1-Wire net using a com port and a DS2480 based
// adapter.
//
// 'port_zstr'  - zero terminated port name.  For this platform
//                use format COMX where X is the port number.
//
// Returns: The portnum or -1 if the port wasn't acquired.
//
int owAcquireEx(char *port_zstr)
{
   OWA = OW_CLOCK;
   OWD = 0x89; // set the clock divider
   
   OWA = OW_CONTROL;
   OWD = 0x00; // clear the control register

   return 0;
}
//---------------------------------------------------------------------------
// Release the previously acquired a 1-Wire net.
//
// 'portnum'    - number 0 to MAX_PORTNUM-1.  This number is provided to
//                indicate the symbolic port number.
//
void owRelease(int portnum)
{
   OWA = OW_CLOCK;
   OWD = 0x00;
}


