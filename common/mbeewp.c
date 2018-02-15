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
//  mbSHAEE.c - Reads and writes to memory locations for the SHAEE memory bank.
//  version 1.00
//

// Include Files
#include "ownet.h"
#include "mbeewp.h"

// General command defines
#define READ_MEMORY_EEWP       0xF0
#define WRITE_SCRATCHPAD_EEWP  0x0F
#define READ_SCRATCHPAD_EEWP   0xAA
#define COPY_SCRATCHPAD_EEWP   0x55

// Local defines
#define PAGE_LENGTH_EEWP 32

// Global variables
char    *bankDescriptionEEWP     = "";
SMALLINT writeVerificationEEWP    = TRUE;
SMALLINT generalPurposeMemoryEEWP = TRUE;
SMALLINT readWriteEEWP            = TRUE;   // check memory status
SMALLINT writeOnceEEWP            = FALSE;  // check memory status
SMALLINT readOnlyEEWP             = FALSE;  // check memory status
SMALLINT nonVolatileEEWP          = TRUE;
SMALLINT needProgramPulseEEWP     = FALSE;
SMALLINT needPowerDeliveryEEWP    = FALSE;
SMALLINT ExtraInfoEEWP            = FALSE;   // memory status page doesn't, so check bank
SMALLINT pageAutoCRCEEWP          = FALSE;   // memory status page doesn't, so check bank


/**
 * Read  memory in the current bank with no CRC checking (device or
 * data). The resulting data from this API may or may not be what is on
 * the 1-Wire device.  It is recommends that the data contain some kind
 * of checking (CRC) like in the readPagePacketSHAEE() method or have
 * the 1-Wire device provide the CRC as in readPageCRCSHAEE().  readPageCRCSHAEE()
 * however is not supported on all memory types, see 'hasPageAutoCRCSHAEE()'.
 * If neither is an option then this method could be called more
 * then once to at least verify that the same thing is read consistantly.
 *
 * bank     to tell what memory bank of the ibutton to use.
 * portnum  the port number of the port being used for the
 *          1-Wire Network.
 * SNum     the serial number for the part that the read is
 *          to be done on.
 * str_add  starting physical address
 * rd_cont  if 'true' then device read is continued without
 *          re-selecting.  This can only be used if the new
 *          read() continious where the last one led off
 *          and it is inside a 'beginExclusive/endExclusive'
 *          block.
 * buff     byte array to place read data into
 * len      length in bytes to read
 *
 * @return 'true' if the read was complete
 */
SMALLINT readEEWP(SMALLINT bank, int portnum, uchar *SNum, int str_add, 
                   SMALLINT rd_cont, uchar *buff, int len)
{
   int i;
   int addr;
   uchar raw_buf[3];

   // check if read exceeds memory
   if ((str_add + len) > (PAGE_LENGTH_EEWP * getNumberPagesEEWP(bank,SNum)))
   {
      OWERROR(OWERROR_READ_OUT_OF_RANGE);
      return FALSE;
   }

   // see if need to access the device
   if (!rd_cont)
   {
      owSerialNum(portnum,SNum,FALSE);

      // select the device
      if (!owAccess(portnum))
      {
         OWERROR(OWERROR_DEVICE_SELECT_FAIL);
         return FALSE;
      }

      // build start reading memory block
      addr    = str_add + getStartingAddressEEWP(bank,SNum);

      raw_buf[0] = READ_MEMORY_EEWP;
      raw_buf[1] = addr & 0xFF;
      raw_buf[2] = ((addr & 0xFFFF) >> 8) & 0xFF;
   
      // do the first block for command, address
      if(!owBlock(portnum,FALSE,raw_buf,3))
      {
         OWERROR(OWERROR_BLOCK_FAILED);
         return FALSE;
      }
   }

   // pre-fill readBuf with 0xFF 
   for(i=0;i<len;i++)
      buff[i] = 0xFF;

   if(!owBlock(portnum,FALSE,buff,len))
   {
      OWERROR(OWERROR_BLOCK_FAILED);
      return FALSE;
   }
   return TRUE;
}

/**
 * Write  memory in the current bank.  It is recommended that
 * when writing  data that some structure in the data is created
 * to provide error free reading back with readSHAEE().  Or the
 * method 'writePagePacketSHAEE()' could be used which automatically
 * wraps the data in a length and CRC.
 *
 * When using on Write-Once devices care must be taken to write into
 * into empty space.  If write() is used to write over an unlocked
 * page on a Write-Once device it will fail.
 *
 * bank     to tell what memory bank of the ibutton to use.
 * portnum  the port number of the port being used for the
 *          1-Wire Network.
 * SNum     the serial number for the part that the write is
 *          to be done on.
 * str_add  starting address
 * buff     byte array containing data to write
 * len      length in bytes to write
 *
 * @return 'true' if the write was complete.
 */
SMALLINT writeEEWP(SMALLINT bank, int portnum, uchar *SNum, int str_add,
                    uchar *buff, int len)
{
   int i, room_left;
   int startx, nextx, abs_addr, pl;
   uchar raw_buf[8];
   uchar extra[20];
   uchar memory[64];

   // return if nothing to do
   if (len == 0)
      return TRUE;

   // check if write exceeds memory
   if ((str_add + len) > getSizeEEWP(bank,SNum))
   {
      OWERROR(OWERROR_WRITE_OUT_OF_RANGE);
      return FALSE;
   }

   // check if trying to write read only bank
   if(isReadOnlyEEWP(bank,portnum,SNum))
   {
      OWERROR(OWERROR_READ_ONLY);
      return FALSE;
   }

   // loop while still have pages to write
   startx   = 0;
   nextx    = 0;   // (start and next index into writeBuf)
   pl       = 8;
   abs_addr = getStartingAddressEEWP(bank,SNum) + str_add;

   if(!readEEWP(bank,portnum,SNum,(str_add/PAGE_LENGTH_EEWP)*PAGE_LENGTH_EEWP,
                 FALSE,memory,PAGE_LENGTH_EEWP))
      return FALSE;

   do
   {
      // calculate room left in current page
      room_left = pl - ((abs_addr + startx) % pl);

      // check if block left will cross end of page
      if ((len - startx) > room_left)
         nextx = startx + room_left;
      else
         nextx = len;

      if((str_add+startx) >= PAGE_LENGTH_EEWP)
      {
         if(!readEEWP(bank,portnum,SNum,((str_add+startx)/PAGE_LENGTH_EEWP)*PAGE_LENGTH_EEWP,
                       FALSE,memory,PAGE_LENGTH_EEWP))
            return FALSE;
      }

      if((str_add+startx) >= PAGE_LENGTH_EEWP)
         for(i=0;i<8;i++)
            raw_buf[i] = memory[((((startx+str_add)/8)*8)-32)+i];
      else
         for(i=0;i<8;i++)
            raw_buf[i] = memory[(((startx+str_add)/8)*8)+i];

      if((nextx-startx) == 8)
         for(i=0;i<8;i++)
            raw_buf[i] = buff[startx+i];
      else
         if(((str_add+nextx)%8) == 0)
            for(i=0;i<(8-((str_add+startx)%8));i++)
               raw_buf[((str_add+startx)%8)+i] = buff[startx+i];
         else
            for(i=0;i<(((str_add+nextx)%8)-((str_add+startx)%8));i++)
               raw_buf[((str_add+startx)%8)+i] = buff[startx+i];

      // write the page of data to scratchpad
      if(!writeSpadWP(portnum,abs_addr+startx+room_left-8,raw_buf,8))
         return FALSE;

      if(!copySpadWP(portnum,abs_addr+startx+room_left-8,SNum,extra,memory))
         return FALSE;

      if(str_add >= PAGE_LENGTH_EEWP)
         for(i=0;i<8;i++)
            memory[((((startx+str_add)/8)*8)-32)+i] = raw_buf[i];
      else
         for(i=0;i<8;i++)
            memory[(((startx+str_add)/8)*8)+i] = raw_buf[i];

      // point to next index
      startx = nextx;
   }
   while (nextx < len);

   return TRUE;
}

/**
 * Read  page in the current bank with no
 * CRC checking (device or data). The resulting data from this API
 * may or may not be what is on the 1-Wire device.  It is recommends
 * that the data contain some kind of checking (CRC) like in the
 * readPagePacketSHAEE() method or have the 1-Wire device provide the
 * CRC as in readPageCRCSHAEE().  readPageCRCSHAEE() however is not
 * supported on all memory types, see 'hasPageAutoCRCSHAEE()'.
 * If neither is an option then this method could be called more
 * then once to at least verify that the same thing is read consistantly.
 *
 * bank     to tell what memory bank of the ibutton to use.
 * portnum  the port number of the port being used for the
 *          1-Wire Network.
 * SNum     the serial number for the part.
 * page     the page to read
 * rd_cont  if 'true' then device read is continued without
 *          re-selecting.  This can only be used if the new
 *          read() continious where the last one led off
 *          and it is inside a 'beginExclusive/endExclusive'
 *          block.
 * buff     byte array containing data that was read.
 *
 * @return - returns '0' if the read page wasn't completed.
 *                   '1' if the operation is complete.
 */
SMALLINT readPageEEWP(SMALLINT bank, int portnum, uchar *SNum, int page,
                       SMALLINT rd_cont, uchar *buff)
{
   return readEEWP(bank,portnum,SNum,page*PAGE_LENGTH_EEWP,FALSE,buff,PAGE_LENGTH_EEWP);
}

/**
 * Read  page with extra information in the current bank with no
 * CRC checking (device or data). The resulting data from this API
 * may or may not be what is on the 1-Wire device.  It is recommends
 * that the data contain some kind of checking (CRC) like in the
 * readPagePacketSHAEE() method or have the 1-Wire device provide the
 * CRC as in readPageCRCSHAEE().  readPageCRCSHAEE() however is not
 * supported on all memory types, see 'hasPageAutoCRCSHAEE()'.
 * If neither is an option then this method could be called more
 * then once to at least verify that the same thing is read consistantly.
 * See the method 'hasExtraInfoSHAEE()' for a description of the optional
 * extra information some devices have.
 *
 * bank     to tell what memory bank of the ibutton to use.
 * portnum  the port number of the port being used for the
 *          1-Wire Network.
 * SNum     the serial number for the part.
 * page     the page to read
 * rd_cont  if 'true' then device read is continued without
 *          re-selecting.  This can only be used if the new
 *          read() continious where the last one led off
 *          and it is inside a 'beginExclusive/endExclusive'
 *          block.
 * buff     byte array containing data that was read
 * extra    the extra information
 *
 * @return - returns '0' if the read page wasn't completed with extra info.
 *                   '1' if the operation is complete.
 */
SMALLINT readPageExtraEEWP(SMALLINT bank, int portnum, uchar *SNum, int page,
                            SMALLINT rd_cont, uchar *buff, uchar *extra)
{
   int addr;

   if(!hasExtraInfoEEWP(bank,SNum))
   {
      OWERROR(OWERROR_EXTRA_INFO_NOT_SUPPORTED);
      return FALSE;
   }

   // build start reading memory block
   addr = page*PAGE_LENGTH_EEWP;
   if(!readEEWP(bank,portnum,SNum,addr,FALSE,buff,PAGE_LENGTH_EEWP))
      return FALSE;

   return TRUE;
}

/**
 * Read a complete memory page with CRC verification provided by the
 * device with extra information.  Not supported by all devices.
 * See the method 'hasPageAutoCRC()'.
 * See the method 'haveExtraInfo()' for a description of the optional
 * extra information.
 *
 * bank      to tell what memory bank of the ibutton to use.
 * portnum   the port number of the port being used for the
 *           1-Wire Network.
 * SNum      the serial number for the part.
 * page      the page to read
 * read_buff byte array containing data that was read.
 * extra     the extra information
 *
 * @return - returns '0' if the read page wasn't completed with extra info.
 *                   '1' if the operation is complete.
 */
SMALLINT readPageExtraCRCEEWP(SMALLINT bank, int portnum, uchar *SNum, int page,
                               uchar *read_buff, uchar *extra)
{
   int addr;

   if(!hasPageAutoCRCEEWP(bank,SNum))
   {
      OWERROR(OWERROR_CRC_EXTRA_INFO_NOT_SUPPORTED);
      return FALSE;
   }

   // build start reading memory block
   addr = page * PAGE_LENGTH_EEWP;
   if(!readEEWP(bank,portnum,SNum,addr,FALSE,read_buff,PAGE_LENGTH_EEWP))
      return FALSE;

   return TRUE;
}

/**
 * Read a complete memory page with CRC verification provided by the
 * device.  Not supported by all devices.  See the method
 * 'hasPageAutoCRCSHAEE()'.
 *
 * bank     to tell what memory bank of the ibutton to use.
 * portnum  the port number of the port being used for the
 *          1-Wire Network.
 * SNum     the serial number for the part.
 * page     the page to read
 * buff     byte array containing data that was read
 *
 * @return - returns '0' if the read page wasn't completed.
 *                   '1' if the operation is complete.
 */
SMALLINT readPageCRCEEWP(SMALLINT bank, int portnum, uchar *SNum, int page, uchar *buff)
{
   return readEEWP(bank,portnum,SNum,page*PAGE_LENGTH_EEWP,FALSE,buff,PAGE_LENGTH_EEWP);
}

/**
 * Read a Universal Data Packet.
 *
 * The Universal Data Packet always starts on page boundaries but
 * can end anywhere in the page.  The structure specifies the length of
 * data bytes not including the length byte and the CRC16 bytes.
 * There is one length byte. The CRC16 is first initialized to
 * the page number.  This provides a check to verify the page that
 * was intended is being read.  The CRC16 is then calculated over
 * the length and data bytes.  The CRC16 is then iNVerted and stored
 * low byte first followed by the high byte.  This is structure is
 * used by this method to verify the data but is not returned, only
 * the data payload is returned.
 *
 * bank     to tell what memory bank of the ibutton to use.
 * portnum  the port number of the port being used for the
 *          1-Wire Network.
 * SNum     the serial number for the part.
 * page     the page to read
 * rd_cont  if 'true' then device read is continued without
 *          re-selecting.  This can only be used if the new
 *          read() continious where the last one led off
 *          and it is inside a 'beginExclusive/endExclusive'
 *          block.
 * buff     byte array containing data that was read.
 * len      length of the packet
 *
 * @return - returns '0' if the read page packet wasn't completed
 *                   '1' if the operation is complete.
 */
SMALLINT readPagePacketEEWP(SMALLINT bank, int portnum, uchar *SNum, int page,
                             SMALLINT rd_cont, uchar *buff, int *len)
{
   uchar raw_buf[PAGE_LENGTH_EEWP];
   ushort lastcrc16;
   int i;

   // read the  page
   if(!readPageEEWP(bank,portnum,SNum,page,rd_cont,&raw_buf[0]))
      return FALSE;
      
   // check if length is realistic
   if((raw_buf[0] & 0x00FF) > getMaxPacketDataLengthEEWP(bank,SNum))
   {
      OWERROR(OWERROR_INVALID_PACKET_LENGTH);
      return FALSE;
   }
   else if(raw_buf[0] == 0)
   {
      *len = raw_buf[0];
      return TRUE;
   }

   // verify the CRC is correct
   setcrc16(portnum,(ushort)((getStartingAddressEEWP(bank,SNum)/PAGE_LENGTH_EEWP) + page));
   for(i=0;i<(raw_buf[0]+3);i++)
      lastcrc16 = docrc16(portnum,raw_buf[i]);

   if(lastcrc16 == 0x0000B001)
   {
      // extract the data out of the packet
      for(i=0;i<raw_buf[0];i++)
         buff[i] = raw_buf[i+1];

      // extract the length
      *len = (int) raw_buf[0];
   }
   else
   {
      OWERROR(OWERROR_CRC_FAILED);
      return FALSE;
   }

   return TRUE;
}

/**
 * Read a Universal Data Packet and extra information.  See the
 * method 'readPagePacketSHAEE()' for a description of the packet structure.
 * See the method 'hasExtraInfoSHAEE()' for a description of the optional
 * extra information some devices have.
 *
 * bank     to tell what memory bank of the ibutton to use.
 * portnum  the port number of the port being used for the
 *          1-Wire Network.
 * SNum     the serial number for the part.
 * page     the page to read
 * rd_cont  if 'true' then device read is continued without
 *          re-selecting.  This can only be used if the new
 *          read() continious where the last one led off
 *          and it is inside a 'beginExclusive/endExclusive'
 *          block.
 * buff     byte array containing data that was read.
 * len      length of the packet
 * extra    extra information
 *
 * @return - returns '0' if the read page packet wasn't completed
 *                   '1' if the operation is complete.
 */
SMALLINT readPagePacketExtraEEWP(SMALLINT bank, int portnum, uchar *SNum,
                                  int page, SMALLINT rd_cont, uchar *buff,
                                  int *len, uchar *extra)
{
   uchar raw_buf[PAGE_LENGTH_EEWP];
   int addr;
   int i;
   ushort lastcrc16;

   if(!hasExtraInfoEEWP(bank,SNum))
   {
      OWERROR(OWERROR_EXTRA_INFO_NOT_SUPPORTED);
      return FALSE;
   }

   addr = page * PAGE_LENGTH_EEWP;

   // read the  page
   if(!readEEWP(bank,portnum,SNum,addr,FALSE,&raw_buf[0],PAGE_LENGTH_EEWP))
      return FALSE;
      
   // check if length is realistic
   if ((raw_buf[0] & 0x00FF) > getMaxPacketDataLengthEEWP(bank,SNum))
   {
      OWERROR(OWERROR_INVALID_PACKET_LENGTH);
      return FALSE;
   }
   else if(raw_buf[0] == 0)
   {
      *len = raw_buf[0];
      return TRUE;
   }

   // verify the CRC is correct
   setcrc16(portnum,(ushort)((getStartingAddressEEWP(bank,SNum)/PAGE_LENGTH_EEWP) + page));
   for(i=0;i<(raw_buf[0]+3);i++)
      lastcrc16 = docrc16(portnum,raw_buf[i]);

   if(lastcrc16 == 0x0000B001)
   {
      // extract the data out of the packet
      for(i=0;i<raw_buf[0];i++)
         buff[i] = raw_buf[i+1];

      // return the length
      *len = (int) raw_buf[0];
   }
   else
   {
      OWERROR(OWERROR_CRC_FAILED);
      return FALSE;
   }

   return TRUE;

}

/**
 * Write a Universal Data Packet.  See the method 'readPagePacketSHAEE()'
 * for a description of the packet structure.
 *
 * bank     to tell what memory bank of the ibutton to use.
 * portnum  the port number of the port being used for the
 *          1-Wire Network.
 * SNum     the serial number for the part.
 * page     the page the packet is being written to.
 * buff     byte array containing data that to write.
 * len      length of the packet
 *
 * @return - returns '0' if the write page packet wasn't completed
 *                   '1' if the operation is complete.
 */
SMALLINT writePagePacketEEWP(SMALLINT bank, int portnum, uchar *SNum, int page,
                              uchar *buff, int len)
{
   uchar raw_buf[PAGE_LENGTH_EEWP];
   ushort crc;
   int i;

   // make sure length does not exceed max
   if(len > getMaxPacketDataLengthEEWP(bank,SNum))
   {
      OWERROR(OWERROR_PACKET_LENGTH_EXCEEDS_PAGE);
      return FALSE;
   }

   // see if this bank is general read/write
   if (!isGeneralPurposeMemoryEEWP(bank,SNum))
   {
      OWERROR(OWERROR_NOT_GENERAL_PURPOSE);
      return FALSE;
   }

   // construct the packet to write
   raw_buf[0] = (uchar) len;

   for(i=0;i<len;i++)
      raw_buf[i+1] = buff[i];

   // calculate crc
   setcrc16(portnum,(ushort)((getStartingAddressEEWP(bank,SNum)/PAGE_LENGTH_EEWP) + page));
   for(i=0;i<(len+1);i++)
      crc = docrc16(portnum,raw_buf[i]);

   raw_buf[len + 1] = (uchar) ~crc & 0xFF;
   raw_buf[len + 2] = (uchar) ((~crc & 0xFFFF) >> 8) & 0xFF;

   // write the packet, return result
   if(!writeEEWP(bank,portnum,SNum,page*PAGE_LENGTH_EEWP,raw_buf,len+3))
      return FALSE;

   return TRUE;
}

/**
 * Query to get the number of pages in current memory bank.
 *
 * bank     to tell what memory bank of the ibutton to use.
 * SNum     the serial number for the part.
 *
 * @return  number of pages in current memory bank
 */
SMALLINT getNumberPagesEEWP(SMALLINT bank, uchar *SNum)
{
   SMALLINT pages;

   pages = 1;

   return pages;
}

/**
 * Query to get the memory bank size in bytes.
 *
 * bank     to tell what memory bank of the ibutton to use.
 * SNum     the serial number for the part.
 *
 * @return  memory bank size in bytes.
 */
int getSizeEEWP(SMALLINT bank, uchar *SNum)
{
   int size;

   size = 32;

   return size;
}

/**
 * Query to get the starting physical address of this bank.  Physical
 * banks are sometimes sub-divided into logical banks due to changes
 * in attributes.
 *
 * bank     to tell what memory bank of the ibutton to use.
 * SNum     the serial number for the part.
 *
 * @return  physical starting address of this logical bank.
 */
int getStartingAddressEEWP(SMALLINT bank, uchar *SNum)
{
   int start = 0;

   switch(bank)
   {
      case 0:
         start = 0;
         break;

      case 1:
         start = 32;
         break;

      case 2:
         start = 64;
         break;

      case 3:
         start = 96;
         break;

      case 4:
         start = 128;
         break;

      default:
         start = 0;
         break;
   }

   return start;
}

/**
 * Query to get page length in bytes in current memory bank.
 *
 * bank     to tell what memory bank of the ibutton to use.
 * SNum     the serial number for the part.
 *
 * @return   page length in bytes in current memory bank
 */
SMALLINT getPageLengthEEWP(SMALLINT bank, uchar *SNum)
{
   return PAGE_LENGTH_EEWP;
}

/**
 * Query to see get a string description of the current memory bank.
 *
 * bank     to tell what memory bank of the ibutton to use.
 * SNum     the serial number for the part.
 *
 * @return  String containing the memory bank description
 */
char *getBankDescriptionEEWP(SMALLINT bank, uchar *SNum)
{
   switch(bank)
   {
      case 0:
         return "Page Zero with EPROM mode.";
         break;
   
      case 1:
         return "Page One with EPROM mode";
         break;

      case 2:
         return "Page Two with EPROM mode.";
         break;

      case 3:
         return "Page Three with EPROM mode.";
         break;

      case 4:
         return "Status Page that contains the secret and the status.";
         break;

      default:
         return bankDescriptionEEWP;
         break;
   }
   
   return bankDescriptionEEWP;
}

/**
 * Query to see if the current memory bank is general purpose
 * user memory.  If it is NOT then it is Memory-Mapped and writing
 * values to this memory will affect the behavior of the 1-Wire
 * device.
 *
 * bank     to tell what memory bank of the ibutton to use.
 * SNum     the serial number for the part.
 *
 * @return  'true' if current memory bank is general purpose
 */
SMALLINT isGeneralPurposeMemoryEEWP(SMALLINT bank, uchar *SNum)
{
   SMALLINT gp = generalPurposeMemoryEEWP;

   if(bank == 4)
      gp = FALSE;

   return gp;
}

/**
 * Query to see if current memory bank is read/write.
 *
 * bank     to tell what memory bank of the ibutton to use.
 * SNum     the serial number for the part.
 *
 * @return  'true' if current memory bank is read/write
 */
SMALLINT isReadWriteEEWP(SMALLINT bank, int portnum, uchar *SNum)
{
   SMALLINT rw = readWriteEEWP;
   uchar status[8];

   if(!readEEWP(4,portnum,SNum,8,FALSE,&status[0],8))
         return FALSE;

   switch(bank)
   {
      case 0:
         if( ((status[0] == 0xAA) || (status[0] == 0x55)) )
             rw = FALSE;
         break;

      case 1:
         if((status[1] == 0xAA) || (status[1] == 0x55))
            rw = FALSE;
         break;

      case 2:
         if((status[2] == 0xAA) || (status[2] == 0x55))
            rw = FALSE;
         break;

      case 3:
         if((status[3] == 0xAA) || (status[3] == 0x55))
            rw = FALSE;
         break;

      case 4:
         rw = FALSE;
         break;

      default:
         rw = readWriteEEWP;
         break;
   }

   return rw;
}

/**
 * Query to see if current memory bank is write write once such
 * as with EPROM technology.
 *
 * bank     to tell what memory bank of the ibutton to use.
 * SNum     the serial number for the part.
 *
 * @return  'true' if current memory bank can only be written once
 */
SMALLINT isWriteOnceEEWP(SMALLINT bank, int portnum, uchar *SNum)
{
   SMALLINT once = writeOnceEEWP;
   SMALLINT same = FALSE;
   uchar status[8];

   if(!readEEWP(4,portnum,SNum,0,FALSE,&status[0],8))
      return FALSE;

   switch(bank)
   {
      case 0:
         if(((status[0] == 0xAA) || (status[0] == 0x55)) )
            once = FALSE;
         break;

      case 1:
         if((status[1] == 0xAA) || (status[1] == 0x55))
            once = FALSE;
         break;

      case 2:
         if((status[2] == 0xAA) || (status[2] == 0x55))
            once = FALSE;
         break;

      case 3:
         if((status[2] == 0xAA) || (status[2] == 0x55))
            once = FALSE;
         break;

      case 4:
         once = FALSE;
         break;

      default:
         once = readWriteEEWP;
         break;
   }

   return once;
}

/**
 * Query to see if current memory bank is read only.
 *
 * bank     to tell what memory bank of the ibutton to use.
 * SNum     the serial number for the part.
 *
 * @return  'true' if current memory bank can only be read
 */
SMALLINT isReadOnlyEEWP(SMALLINT bank, int portnum, uchar *SNum)
{
   return readOnlyEEWP;
}

/**
 * Query to see if current memory bank non-volatile.  Memory is
 * non-volatile if it retains its contents even when removed from
 * the 1-Wire network.
 *
 * bank     to tell what memory bank of the ibutton to use.
 * SNum     the serial number for the part.
 *
 * @return  'true' if current memory bank non volatile.
 */
SMALLINT isNonVolatileEEWP(SMALLINT bank, uchar *SNum)
{
   return nonVolatileEEWP;
}

/**
 * Query to see if current memory bank pages need the adapter to
 * have a 'ProgramPulse' in order to write to the memory.
 *
 * bank     to tell what memory bank of the ibutton to use.
 * SNum     the serial number for the part.
 *
 * @return  'true' if writing to the current memory bank pages
 *                 requires a 'ProgramPulse'.
 */
SMALLINT needsProgramPulseEEWP(SMALLINT bank, uchar *SNum)
{
   return needProgramPulseEEWP;
}

/**
 * Query to see if current memory bank pages need the adapter to
 * have a 'PowerDelivery' feature in order to write to the memory.
 *
 * bank     to tell what memory bank of the ibutton to use.
 * SNum     the serial number for the part.
 *
 * @return  'true' if writing to the current memory bank pages
 *                 requires 'PowerDelivery'.
 */
SMALLINT needsPowerDeliveryEEWP(SMALLINT bank, uchar *SNum)
{
   return needPowerDeliveryEEWP;
}

/**
 * Checks to see if this memory bank's pages deliver extra
 * information outside of the normal data space,  when read.  Examples
 * of this may be a redirection byte, counter, tamper protection
 * bytes, or SHA-1 result.  If this method returns true then the
 * methods with an 'extraInfo' parameter can be used.
 *
 * bank     to tell what memory bank of the ibutton to use.
 * SNum     the serial number for the part.
 *
 * @return  true if reading the this memory bank's
 *               pages provides extra information
 */
SMALLINT hasExtraInfoEEWP(SMALLINT bank, uchar *SNum)
{
   return ExtraInfoEEWP;
}

/**
 * Query to get the length in bytes of extra information that
 * is read when read a page in the current memory bank.  See
 * 'hasExtraInfoSHAEE()'.
 *
 * bank     to tell what memory bank of the ibutton to use.
 * SNum     the serial number for the part.
 *
 * @return  number of bytes in Extra Information read when reading
 *          pages in the current memory bank.
 */
SMALLINT getExtraInfoLengthEEWP(SMALLINT bank, uchar *SNum)
{
   return 0;
}

/**
 * Query to get a string description of what is contained in
 * the Extra Informationed return when reading pages in the current
 * memory bank.  See 'hasExtraInfoSHAEE()'.
 *
 * bank     to tell what memory bank of the ibutton to use.
 * SNum     the serial number for the part.
 *
 * @return string describing extra information.
 */
char *getExtraInfoDescEEWP(SMALLINT bank, uchar *SNum)
{
   return "";
}

/**
 * Query to see if current memory bank pages can be read with
 * the contents being verified by a device generated CRC.
 * This is used to see if the 'ReadPageCRCSHAEE()' can be used.
 *
 * bank     to tell what memory bank of the ibutton to use.
 * SNum     the serial number for the part.
 *
 * @return  'true' if current memory bank can be read with self
 *          generated CRC.
 */
SMALLINT hasPageAutoCRCEEWP(SMALLINT bank, uchar *SNum)
{
   return pageAutoCRCEEWP;
}

/**
 * Query to get Maximum data page length in bytes for a packet
 * read or written in the current memory bank.  See the 'ReadPagePacket()'
 * and 'WritePagePacket()' methods.  This method is only usefull
 * if the current memory bank is general purpose memory.
 *
 * bank     to tell what memory bank of the ibutton to use.
 * SNum     the serial number for the part.
 *
 * @return  max packet page length in bytes in current memory bank
 */
SMALLINT getMaxPacketDataLengthEEWP(SMALLINT bank, uchar *SNum)
{
   return PAGE_LENGTH_EEWP - 3;
}

/**
 * Query to see if current memory bank pages can be redirected
 * to another pages.  This is mostly used in Write-Once memory
 * to provide a means to update.
 *
 * bank     to tell what memory bank of the ibutton to use.
 * SNum     the serial number for the part.
 *
 * @return  'true' if current memory bank pages can be redirected
 *          to a new page.
 */
SMALLINT canRedirectPageEEWP(SMALLINT bank, uchar *SNum)
{
   return FALSE;
}

/**
 * Query to see if current memory bank pages can be locked.  A
 * locked page would prevent any changes to the memory.
 *
 * bank     to tell what memory bank of the ibutton to use.
 * SNum     the serial number for the part.
 *
 * @return  'true' if current memory bank pages can be redirected
 *          to a new page.
 */
SMALLINT canLockPageEEWP(SMALLINT bank, uchar *SNum)
{
   return FALSE;
}

/**
 * Query to see if current memory bank pages can be locked from
 * being redirected.  This would prevent a Write-Once memory from
 * being updated.
 *
 * bank     to tell what memory bank of the ibutton to use.
 * SNum     the serial number for the part.
 *
 * @return  'true' if current memory bank pages can be locked from
 *          being redirected to a new page.
 */
SMALLINT canLockRedirectPageEEWP(SMALLINT bank, uchar *SNum)
{
   return FALSE;
}


// ------------------------
// Extras
// ------------------------

/**
 * Write to the Scratch Pad, which is a max of 8 bytes.
 *
 * portnum  the port number of the port being used for the
 *          1-Wire Network.
 * addr     the address to write the data to
 * out_buf  byte array to write into scratch pad
 * len      length of the write data
 *
 * @return - returns 'TRUE' if scratch pad was written correctly
 *                   and 'FALSE' if not.
 */
SMALLINT writeSpadWP(int portnum, int addr, uchar *out_buf, int len)
{
   uchar send_block[50];
   short send_cnt=0,i;
   ushort lastcrc16;

   // access the device
   if(owAccess(portnum))
   {
      setcrc16(portnum,0);
      // write scratchpad command
      send_block[send_cnt] = 0x0F;
      lastcrc16 = docrc16(portnum,send_block[send_cnt++]);
      // address 1
      send_block[send_cnt] = (uchar)(addr & 0xFF);
      lastcrc16 = docrc16(portnum,send_block[send_cnt++]);
      // address 2
      send_block[send_cnt] = (uchar)((addr >> 8) & 0xFF);
      lastcrc16 = docrc16(portnum,send_block[send_cnt++]);
      // data
      for (i = 0; i < len; i++)
      {
         send_block[send_cnt] = out_buf[i];
         lastcrc16 = docrc16(portnum,send_block[send_cnt++]);
      }

      // CRC16
      send_block[send_cnt++] = 0xFF;
      send_block[send_cnt++] = 0xFF;

      // now send the block
      if (owBlock(portnum,FALSE,send_block,send_cnt))
      {

         // perform CRC16 of last 2 byte in packet
         for (i = send_cnt - 2; i < send_cnt; i++)
            lastcrc16 = docrc16(portnum,send_block[i]);

         // verify CRC16 is correct
         if (lastcrc16 == 0xB001)
            return TRUE;
         else
         {
            OWERROR(OWERROR_CRC_FAILED);
            return FALSE;
         }
      }
      else
      {
         OWERROR(OWERROR_BLOCK_FAILED);
         return FALSE;
      }
   }
   else
      OWERROR(OWERROR_DEVICE_SELECT_FAIL);

   return FALSE;
}

/**
 * Copy what is in the Sratch Pad, which is a max of 8 bytes to a certain
 * address in memory.
 *
 * portnum       the port number of the port being used for the
 *                  1-Wire Network.
 * addr          the address to copy the data to
 * SNum          the serial number for the part.
 * extra_buf     byte array to write the calculated MAC
 * memory        byte array for memory of the page to be copied to
 *
 * @return - returns 'TRUE' if the scratch pad was copied correctly 
 *                   and 'FALSE' if not.
 */
SMALLINT copySpadWP(int portnum, int addr, uchar *SNum, uchar *extra_buf, uchar *memory)
{
   short send_cnt=0;
   ushort tmpadd;
   uchar send_block[80];
   uchar scratch[8],es,test;

   if(!readSpadWP(portnum,&tmpadd,&es,&scratch[0]))
   {
      return FALSE;
   }

   // access the device
   if(owAccess(portnum))
   {
      // Copy command
      send_block[send_cnt++] = 0x55;

      // address 1
      send_block[send_cnt++] = (uchar)(tmpadd & 0xFF);
      // address 2
      send_block[send_cnt++] = (uchar)((tmpadd >> 8) & 0xFF);

      // ending address with data status
      send_block[send_cnt++] = es;

      if(owBlock(portnum,FALSE,send_block,send_cnt))
      {
         msDelay(13);

         test = owReadByte(portnum);

         if((test == 0xAA) || (test == 0x55))
            return TRUE;
         else
         {
            OWERROR(OWERROR_COPY_SCRATCHPAD_FAILED);
            return FALSE;
         }
      }
      else
      {
         OWERROR(OWERROR_BLOCK_FAILED);
         return FALSE;
      }
   }
   else
   {
      OWERROR(OWERROR_DEVICE_SELECT_FAIL);
      return FALSE;
   }


   return FALSE;
}

/**
 * Read from the Scratch Pad, which is a max of 8 bytes.
 *
 * portnum       number 0 to MAX_PORTNUM-1.  This number is provided to
 *                  indicate the symbolic port number.
 * addr          the address to read the data
 * es            offset byte read from scratchpad
 * data          data buffer read from scratchpad
 *
 * @return - returns 'TRUE' if the scratch pad was read correctly
 *                   and 'FALSE' if not
 */
SMALLINT readSpadWP(int portnum, ushort *addr, uchar *es, uchar *data)
{
   short send_cnt=0;
   uchar send_block[50];
   int i;
   ushort lastcrc16;

   // access the device
   if (owAccess(portnum))
   {
      // read scratchpad command
      send_block[send_cnt++] = 0xAA;
      // now add the read bytes for data bytes and crc16
      for(i=0; i<13; i++)
         send_block[send_cnt++] = 0xFF;

      // now send the block
      if (owBlock(portnum,FALSE,send_block,send_cnt))
      {
         // copy data to return buffers
         *addr = (send_block[2] << 8) | send_block[1];
         *es = send_block[3];

         // calculate CRC16 of result
         setcrc16(portnum,0);
         for (i = 0; i < send_cnt ; i++)
            lastcrc16 = docrc16(portnum,send_block[i]);

         // verify CRC16 is correct
         if (lastcrc16 == 0xB001)
         {
            for (i = 0; i < 8; i++)
               data[i] = send_block[4 + i];
            // success
            return TRUE;
         }
         else
         {
            OWERROR(OWERROR_CRC_FAILED);
            return FALSE;
         }
      }
      else
      {
         OWERROR(OWERROR_BLOCK_FAILED);
         return FALSE;
      }
   }
   else
   {
      OWERROR(OWERROR_DEVICE_SELECT_FAIL);
      return FALSE;
   }

   return FALSE;
}

