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
//  swt1C.c - Does operations to the DS23E04
//  version 3.00
//

// Include Files
#include "ownet.h"

SMALLINT setSwitch1C(int portnum, uchar *SNum, uchar *state);

SMALLINT readSwitch1C(int portnum, uchar *SNum, uchar *state);

SMALLINT setResetMode1C(int portnum, uchar* SNum, SMALLINT seton);

SMALLINT getVCC1C(uchar *reg);

SMALLINT clearPowerOnReset1C(int portnum, uchar *SNum);

SMALLINT orConditionalSearch1C(int portnum, uchar *SNum);

SMALLINT andConditionalSearch1C(int portnum, uchar *SNum);

SMALLINT setChannelMask1C(int portnum, uchar *SNum, int channel, SMALLINT seton);

SMALLINT setChannelPolarity1C(int portnum, uchar *SNum, int channel, SMALLINT seton);

SMALLINT getChannelMask1C(int portnum, uchar *SNum, int channel);

SMALLINT getChannelPolarity1C(int portnum, uchar *SNum, int channel);

SMALLINT setRegister1C(int portnum, uchar *SNum, uchar *reg);

SMALLINT readRegister1C(int portnum, uchar *SNum, uchar *reg);

SMALLINT getLatchState1C(int channel, uchar *state);

SMALLINT getLevel1C(int channel, uchar *state);

SMALLINT getSensedActivity1C(int channel, uchar *state);

SMALLINT setLatchState1C(int portnum, uchar *SNum, int channel, uchar set);

SMALLINT read1C(int portnum, uchar *SNum, int address, int len, uchar *data);

SMALLINT writeScratch1C(int portnum, uchar *SNum, int address, int len, uchar *data);

SMALLINT readScratch1C(int portnum, uchar *SNum, int *len, uchar *es, uchar *addr, uchar *data);

SMALLINT copyScratch1C(int portnum, uchar *SNum);



