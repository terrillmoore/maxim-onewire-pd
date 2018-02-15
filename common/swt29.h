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

SMALLINT setSwitch29(int portnum, uchar *SNum, uchar *state);

SMALLINT readSwitch29(int portnum, uchar *SNum, uchar *state);

SMALLINT setResetMode(int portnum, uchar* SNum, SMALLINT seton);

SMALLINT getVCC(uchar *reg);

SMALLINT clearPowerOnReset(int portnum, uchar *SNum);

SMALLINT orConditionalSearch(int portnum, uchar *SNum);

SMALLINT andConditionalSearch(int portnum, uchar *SNum);

SMALLINT pioConditionalSearch(int portnum, uchar *SNum);

SMALLINT activityConditionalSearch(int portnum, uchar *SNum);

SMALLINT setChannelMask(int portnum, uchar *SNum, int channel, SMALLINT seton);

SMALLINT setChannelPolarity(int portnum, uchar *SNum, int channel, SMALLINT seton);

SMALLINT getChannelMask(int portnum, uchar *SNum, int channel);

SMALLINT getChannelPolarity(int portnum, uchar *SNum, int channel);

SMALLINT setRegister29(int portnum, uchar *SNum, uchar *reg);

SMALLINT readRegister29(int portnum, uchar *SNum, uchar *reg);

SMALLINT getLatchState(int channel, uchar *state);

SMALLINT getLevel(int channel, uchar *state);

SMALLINT getSensedActivity (int channel, uchar *state);

SMALLINT setLatchState(int portnum, uchar *SNum, int channel, uchar set);
