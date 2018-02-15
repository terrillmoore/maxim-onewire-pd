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
// mbEE.h - Include memory bank EE functions.
//
// Version: 2.10
//

#include "ownet.h"


// general command defines 
#define READ_MEMORY_COMMAND 0xF0
#define WRITE_SCRATCHPAD_COMMAND 0x0F
#define READ_SCRATCHPAD_COMMAND 0xAA
#define COPY_SCRATCHPAD_COMMAND 0x55

// Local defines
#define SIZE_SHAEE 32
#define PAGE_LENGTH_EEWP 32

// Local function definitions
SMALLINT readEEWP(SMALLINT bank, int portnum, uchar *SNum, int str_add, 
                   SMALLINT rd_cont, uchar *buff,  int len);
SMALLINT writeEEWP(SMALLINT bank, int portnum, uchar *SNum, int str_add, 
                    uchar *buff, int len);
SMALLINT readPageEEWP(SMALLINT bank, int portnum, uchar *SNum, int page,
                       SMALLINT rd_cont, uchar *buff);
SMALLINT readPageExtraEEWP(SMALLINT bank, int portnum, uchar *SNum, int page,
                            SMALLINT rd_cont, uchar *buff, uchar *extra);
SMALLINT readPageExtraCRCEEWP(SMALLINT bank, int portnum, uchar *SNum, int page,
                               uchar *read_buff, uchar *extra);
SMALLINT readPageCRCEEWP(SMALLINT bank, int portnum, uchar *SNum, int page,
                          uchar *buff);
SMALLINT readPagePacketEEWP(SMALLINT bank, int portnum, uchar *SNum, int str_add,
                             SMALLINT rd_cont, uchar *buff, int *len);
SMALLINT readPagePacketExtraEEWP(SMALLINT bank, int portnum, uchar *SNum, 
                                  int str_add, SMALLINT rd_cont, uchar *buff, 
                                  int *len, uchar *extra);
SMALLINT writePagePacketEEWP(SMALLINT bank, int portnum, uchar *SNum, int str_add,
                              uchar *buff, int len);
SMALLINT getNumberPagesEEWP(SMALLINT bank, uchar *SNum);
int getSizeEEWP(SMALLINT bank, uchar *SNum);
SMALLINT getPageLengthEEWP(SMALLINT bank, uchar *SNum);
int getStartingAddressEEWP(SMALLINT bank, uchar *SNum);
char *getBankDescriptionEEWP(SMALLINT bank, uchar *SNum);
SMALLINT isGeneralPurposeMemoryEEWP(SMALLINT bank, uchar *SNum);
SMALLINT isReadWriteEEWP(SMALLINT bank, int portnum, uchar *SNum);
SMALLINT isWriteOnceEEWP(SMALLINT bank, int portnum, uchar *SNum);
SMALLINT isReadOnlyEEWP(SMALLINT bank, int portnum, uchar *SNum);
SMALLINT isNonVolatileEEWP(SMALLINT bank, uchar *SNum);
SMALLINT needsProgramPulseEEWP(SMALLINT bank, uchar *SNum);
SMALLINT needsPowerDeliveryEEWP(SMALLINT bank, uchar *SNum);
SMALLINT hasExtraInfoEEWP(SMALLINT bank, uchar *SNum);
SMALLINT getExtraInfoLengthEEWP(SMALLINT bank, uchar *SNum);
char    *getExtraInfoDescEEWP(SMALLINT bank, uchar *SNum);
SMALLINT getMaxPacketDataLengthEEWP(SMALLINT bank, uchar *SNum);
SMALLINT hasPageAutoCRCEEWP(SMALLINT bank, uchar *SNum);
SMALLINT canRedirectPageEEWP(SMALLINT bank, uchar *SNum);
SMALLINT canLockPageEEWP(SMALLINT bank, uchar *SNum);
SMALLINT canLockRedirectPageEEWP(SMALLINT bank, uchar *SNum);


// Local functions
SMALLINT writeSpadWP(int portnum, int addr, uchar *out_buf, int len);
SMALLINT readSpadWP(int portnum, ushort *addr, uchar *es, uchar *data);
SMALLINT copySpadWP(int portnum, int addr, uchar *SNum, uchar *extra_buf,
                  uchar *memory);

