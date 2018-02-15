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
//  swtmain1c.c - This utility uses the different functions of the DS28E04
//
//  Version:   3.10
//  History:
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "ownet.h"
#include "findtype.h"
#include "swt1C.h"

// Defines for the switch statements for data entry
#define MODE_TEXT          0
#define MODE_HEX           1
// Max number for data entry
#define MAX_LEN            256

// globals
long time_stamp;
// verbose output mode
int VERBOSE=0;

int getNumber (int min, int max);
int menuSelect(uchar *SNum);

//----------------------------------------------------------------------
//  This is the Main routine for swtmain1c
//
int main(short argc, char **argv)
{
   char msg[200];
   uchar data[256];
   int portnum = 0;
   int n=0;
   int addr = 0;
   int len;
   uchar es = 0x00;
   uchar address[2];
   ushort i;
   uchar sn[8],state[3], reg[3], send_block[37];
   uchar family[2][8];
   int done=FALSE; 
   int channel=0,send_cnt=0;
   SMALLINT alternate=TRUE, alt=FALSE;
   ushort lastcrc16;
   uchar latch, set;
   uchar check;

   // check for required port name
   if (argc != 2)
   {
      sprintf(msg,"1-Wire Net name required on command line!\n"
                  " (example: \"COM1\" (Win32 DS2480),\"/dev/cua0\" "
                  "(Linux DS2480),\"1\" (Win32 TMEX)\n");
      printf("%s",msg);
      return 0;
   }

   if((portnum = owAcquireEx(argv[1])) < 0)
   {
      printf("Did not Acquire port.\n",1);
      exit(1);
   }
   else
   {
      if(FindDevices(portnum,&family[0],0x1C,1))
      {
         for(i=0;i<8;i++)
            sn[i] = family[0][i];

         printf("device found: ");
         for(i=0;i<8;i++)
            printf("%02X ",sn[i]);
         printf("\n");

         do
         {
            printf("PICK AN OPERATION:\n\n");
            printf("(1)  Read Channel state\n");             // Gives channel information

            printf("(2)  Set Channel On/Off\n");             // Sets channel

            printf("(3)  Read Channel Mask\n");              // Read Selection Mask

            printf("(4)  Set Channel mask\n");               // Sets channel mask

            printf("(5)  Read Channel Polarity\n");          // Read Polarity Selection

            printf("(6)  Set Channel polarity\n");           // sets channel polarity

            printf("(7)  Read Control/Status Register\n");   // Read Control/Status Reg

            printf("(8)  Set Reset Mode On/Off\n");          // Set Reset Mode

            printf("(9)  Clear Power on Reset\n");           // Clear Power on reset

            printf("(10) Get VCC state\n");                  // Get VCC state

            printf("(11) Set OR conditional search\n");      // or condition search

            printf("(12) Set AND conditional search\n");     // and condition search

            printf("(13) Write Scratchpad\n");               // write scratchpad command

            printf("(14) Read Scratchpad\n");                // read scratchpad command

            printf("(15) Copy Scratchpad\n");                // copy scratchpad command

            printf("(16) Read Memory\n");                    // read memory

            printf("(17) PIO access read with CRC confirmation\n");  // access read

            printf("(18) LED test\n");                       // LED test

            printf("(19) QUIT\n");

            scanf("%d",&n);
            if(n == 19)
            {
               n = 0;        //used to finish off the loop
               done = TRUE;
               break;
            }

            switch(n)
            {
               case 1:  // Channel Info
                  printf("\nEnter the channel\n");
                  scanf("%d",&channel);

                  if(readSwitch1C(portnum,&sn[0],&state[0]))
                  {
                     printf("The channel is ");
                     if(getLatchState1C(channel,&state[0]))
                        printf("on.\n");
                     else
                        printf("off\n");
                     printf("The Level is ");
                     if(getLevel1C(channel,&state[0]))
                        printf("high.\n");
                     else
                        printf("low.\n");
                     if(getSensedActivity1C(channel,&state[0]))
                        printf("Activity was detected on the channel.\n\n");
                     else
                        printf("No activity was detected on the channel.\n\n");
                  }
                  else
                     OWERROR_DUMP(stdout);

                  break;

               case 2:  // Sets channel
                  printf("\nEnter the channel\n");
                  scanf("%d",&channel);
                  printf("Turn channel off enter 0, on 1.\n");
                  scanf("%d",&set);

                  if(setLatchState1C(portnum,&sn[0],channel,set))
                  {
                     printf("Latch was set ");
                     if(set)
                        printf("on.\n");
                     else
                        printf("off.\n");
                  }
                  else
                     OWERROR_DUMP(stdout);

                  break;

               case 3:  // Read Selection Mask
                  printf("\nEnter the channel\n");
                  scanf("%d",&channel);

                  if(readRegister1C(portnum,&sn[0],&reg[0]))
                  {
                     printf("register is %02X %02X %02X\n",reg[0],reg[1],reg[2]);
                     printf("The Selection Mask for channel %d is ",channel);

                     latch = (uchar) (0x01 << channel);
                    
                     if((reg[0] & latch) == latch)
                        printf("set.\n\n");
                     else
                        printf("not set.\n\n");
                  }
                  else
                     OWERROR_DUMP(stdout);

                  break;

               case 4:  // Sets channel mask
                  printf("\nEnter the channel\n");
                  scanf("%d",&channel);
                  printf("Turn channel mask off enter 0, on 1.\n");
                  scanf("%d",&set);

                  if(setChannelMask1C(portnum,&sn[0],channel,set))
                  {
                     printf("The mask for channel %d was set ",channel);
                     if(set)
                        printf("on.\n\n");
                     else
                        printf("off.\n\n");
                  }
                  else
                     OWERROR_DUMP(stdout);

                  break;

               case 5:  // Read Polarity Selection 
                  printf("\nEnter the channel\n");
                  scanf("%d",&channel);

                  printf("The Polarity for channel %d is ",channel);

                  if(getChannelPolarity1C(portnum,&sn[0],channel))
                     printf("set.\n\n");
                  else
                     printf("not set.\n\n");

                  break;

               case 6:  // sets channel polarity
                  printf("\nEnter the channel\n");
                  scanf("%d",&channel);
                  printf("Turn channel polarity off enter 0, on 1.\n");
                  scanf("%d",&set);

                  if(setChannelPolarity1C(portnum,&sn[0],channel,set))
                  {
                     printf("The polarity for channel %d was set ",channel);
                     if(set)
                        printf("on.\n\n");
                     else
                        printf("off.\n\n");
                  }
                  else
                     OWERROR_DUMP(stdout);

                  break;

               case 7:  // Read Control/Status Reg
                  if(readRegister1C(portnum,&sn[0],&reg[0]))
                     printf("The Constrol/Status register is as following in hex %02X\n\n",
                             reg[2]);
                  else
                     OWERROR_DUMP(stdout);

                  break;

               case 8:  // Set Reset Mode
                  printf("Turn reset mode off enter 0, on 1.\n");
                  scanf("%d",&set);

                  if(setResetMode1C(portnum,&sn[0],set))
                  {
                     printf("Reset Mode was turned ");
                     if(set)
                        printf("on.\n\n");
                     else
                        printf("off.\n\n");
                  }
                  else
                     OWERROR_DUMP(stdout);

                  break;

               case 9:  // Clear Power on reset
                  if(clearPowerOnReset1C(portnum,&sn[0]))
                     printf("Power on reset was cleared.\n\n");
                  else
                     OWERROR_DUMP(stdout);

                  break;

               case 10:  // Get VCC state
                  if(readRegister1C(portnum,&sn[0],&reg[0]))
                  {
                     printf("VCC state register is %02X\n",reg[2]);
                     if(getVCC1C(&reg[0]))
                        printf("VCC is powered.\n\n");
                     else
                        printf("VCC is grounded.\n\n");
                  }
                  else
                     OWERROR_DUMP(stdout);

                  break;

               case 11:  // or condition search
                  if(orConditionalSearch1C(portnum,&sn[0]))
                     printf("OR condition search was set.\n\n");
                  else
                     OWERROR_DUMP(stdout);

                  break;

               case 12:  // and condition search
                  if(andConditionalSearch1C(portnum,&sn[0]))
                     printf("AND condition search was set.\n\n");
                  else
                     OWERROR_DUMP(stdout);

                  break;

               case 13:  // write scratchpad
                  printf("Enter the address to start writing: ");

                  addr = getNumber(0, 550);

            		if(menuSelect(&sn[0]) == MODE_TEXT)
       					len = getData(data,MAX_LEN,MODE_TEXT);
        				else
        					len = getData(data,MAX_LEN,MODE_HEX);

                  if(!writeScratch1C(portnum,&sn[0],addr,len,&data[0]))
                     OWERROR_DUMP(stdout);
                  break;

               case 14:  // read scratchpad
                  if(!readScratch1C(portnum,&sn[0],&len,&es,&address[0],&data[0]))
                  {
                     printf("error\n");
                     OWERROR_DUMP(stdout);
                  }
                  else
                  {
                     printf("Address bytes:   %02X %02X\n",address[0],address[1]);
                     printf("ES byte:         %02X\n",es);
                     printf("Length:          %d\n",len);
                     printf("Scratchpad data: ");
                     for(i=0;i<len;i++)
                        printf("%02X ",data[i]);
                     printf("\n");
                  }

                  break;

               case 15:  // copy scratchpad
                  if(!copyScratch1C(portnum,&sn[0]))
                  {
                     OWERROR_DUMP(stdout);
                  }
                  else
                  {
                     printf("Copy Scratchpad Complete.\n");
                  }

                  break;

               case 16:  // read memory
                  printf("Enter the address to start reading: ");
                  addr = getNumber(0, 550);

                  printf("Enter the length you want to read: ");
                  len = getNumber(0,256);

                  if(!read1C(portnum,&sn[0],addr,len,&data[0]))
                  {
                     OWERROR_DUMP(stdout);
                  }
                  else
                  {
                     for(i=0;i<len;i++)
                        printf("%02X ",data[i]);
                     printf("\n");
                  }

                  break;

               case 17:  // and condition search
                  if (!owTouchReset(portnum))
                     OWERROR_DUMP(stdout);

                  if(!owWriteByte(portnum,0xCC))
                     printf("skip rom error.\n");

                  owWriteByte(portnum,0xF5);
                  for(i=0;i<34;i++)
                     send_block[send_cnt++] = 0xFF;
                  
                  if(!owBlock(portnum,FALSE,&send_block[0],send_cnt))
                  {
                     OWERROR(OWERROR_BLOCK_FAILED);
                     return FALSE;
                  }

                  setcrc16(portnum,0);
                  lastcrc16 = docrc16(portnum,0xF5);
                  for(i=0;i<34;i++)
                     lastcrc16 = docrc16(portnum,send_block[i]);
                  if(lastcrc16 != 0xB001)
                     printf("CRC didn't match.\n");


                  printf("read data: ");
                  for(i=0;i<34;i++)
                     printf("%02X ",send_block[i]);
                  printf("\n");

                  send_cnt = 0;
                  for(i=0;i<34;i++)
                     send_block[send_cnt++] = 0xFF;
                  
                  if(!owBlock(portnum,FALSE,&send_block[0],send_cnt))
                  {
                     OWERROR(OWERROR_BLOCK_FAILED);
                     return FALSE;
                  }

                  setcrc16(portnum,0);
                  for(i=0;i<34;i++)
                     lastcrc16 = docrc16(portnum,send_block[i]);
                  if(lastcrc16 != 0xB001)
                     printf("CRC2 didn't match.\n");


                  printf("read data2: ");
                  for(i=0;i<34;i++)
                     printf("%02X ",send_block[i]);
                  printf("\n");

                  send_cnt = 0;
                  for(i=0;i<34;i++)
                     send_block[send_cnt++] = 0xFF;
                  
                  if(!owBlock(portnum,FALSE,&send_block[0],send_cnt))
                  {
                     OWERROR(OWERROR_BLOCK_FAILED);
                     return FALSE;
                  }

                  setcrc16(portnum,0);
                  for(i=0;i<34;i++)
                     lastcrc16 = docrc16(portnum,send_block[i]);
                  if(lastcrc16 != 0xB001)
                     printf("CRC3 didn't match.\n");


                  printf("read data3: ");
                  for(i=0;i<34;i++)
                     printf("%02X ",send_block[i]);
                  printf("\n");

                  break;

               case 18:  // LED test
                  printf("\nEnter the channel to turn the LED on.\n");
                  scanf("%d",&channel);
                  printf("Turn reset mode off enter 1, on 0.\n");
                  scanf("%d",&set);
                  printf("Alternate on and off 0=No, 1=Yes.\n");
                  scanf("%d",&alternate);

                  if (!owTouchReset(portnum))
                     OWERROR_DUMP(stdout);

                  if(!owWriteByte(portnum,0xCC))
                     printf("skip rom error.\n");

                  owWriteByte(portnum,0x5A);

                  for(i=0;i<256;i++)
                  {
                     if(channel == 0)
                     {
                        if(set == 0)
                        {
                           if(!alternate)
                           {
                              if(!owWriteByte(portnum,0xFE))
                                 printf("write byte error.\n");
                              if(!owWriteByte(portnum,0x01))
                                 printf("write byte error.\n");
                              check = 0xFE;
                           }
                           else
                           {
                              if(alt)
                              {
                                 if(!owWriteByte(portnum,0xFE))
                                    printf("write byte error.\n");
                                 if(!owWriteByte(portnum,0x01))
                                    printf("write byte error.\n");
                                 check = 0xFE;
                                 alt = FALSE;
                              }
                              else
                              {
                                 if(!owWriteByte(portnum,0xFF))
                                    printf("write byte error.\n");
                                 if(!owWriteByte(portnum,0x00))
                                    printf("write byte error.\n");
                                 check = 0xFF;
                                 alt = TRUE;
                              }
                           }
                        }
                        else
                        {
                           if(!alternate)
                           {
                              if(!owWriteByte(portnum,0xFF))
                                 printf("write byte error.\n");
                              if(!owWriteByte(portnum,0x00))
                                 printf("write byte error.\n");
                              check = 0xFF;
                           }
                           else
                           {
                              if(alt)
                              {
                                 if(!owWriteByte(portnum,0xFE))
                                    printf("write byte error.\n");
                                 if(!owWriteByte(portnum,0x01))
                                    printf("write byte error.\n");
                                 check = 0xFE;
                                 alt = FALSE;
                              }
                              else
                              {
                                 if(!owWriteByte(portnum,0xFF))
                                    printf("write byte error.\n");
                                 if(!owWriteByte(portnum,0x00))
                                    printf("write byte error.\n");
                                 check = 0xFF;
                                 alt = TRUE;
                              }
                           }
                        }
                     }
                     else
                     {
                        if(set == 0)
                        {
                           if(!alternate)
                           {
                              if(!owWriteByte(portnum,0xFD))
                                 printf("write byte error.\n");
                              if(!owWriteByte(portnum,0x02))
                                 printf("write byte error.\n");
                              check = 0xFD;
                           }
                           else
                           {
                              if(alt)
                              {
                                 if(!owWriteByte(portnum,0xFD))
                                    printf("write byte error.\n");
                                 if(!owWriteByte(portnum,0x02))
                                    printf("write byte error.\n");
                                 check = 0xFD;
                                 alt = FALSE;
                              }
                              else
                              {
                                 if(!owWriteByte(portnum,0xFF))
                                    printf("write byte error.\n");
                                 if(!owWriteByte(portnum,0x00))
                                    printf("write byte error.\n");
                                 check = 0xFF;
                                 alt = TRUE;
                              }
                           }
                        }
                        else
                        {
                           if(!alternate)
                           {
                              if(!owWriteByte(portnum,0xFF))
                                 printf("write byte error.\n");
                              if(!owWriteByte(portnum,0x00))
                                 printf("write byte error.\n");
                              check = 0xFF;
                           }
                           else
                           {
                              if(alt)
                              {
                                 if(!owWriteByte(portnum,0xFD))
                                    printf("write byte error.\n");
                                 if(!owWriteByte(portnum,0x02))
                                    printf("write byte error.\n");
                                 check = 0xFD;
                                 alt = FALSE;
                              }
                              else
                              {
                                 if(!owWriteByte(portnum,0xFF))
                                    printf("write byte error.\n");
                                 if(!owWriteByte(portnum,0x00))
                                    printf("write byte error.\n");
                                 check = 0xFF;
                                 alt = TRUE;
                              }
                           }
                        }
                     }

                     send_block[0] = (uchar)owReadByte(portnum);

                     send_block[1] = (uchar)owReadByte(portnum);

                     if((send_block[0] != 0xAA) && (send_block[1] != check))
                        printf("confirmation byte was %02X and read back was %02X\n",
                                 send_block[0],send_block[1]);
                  }

               default:
                  break;
            }

         }while(!done);

      }
      else
         printf("DS28E04 not found on One Wire Network\n");

      owRelease(portnum);
   }

   return 1;
}

/**
 * Retrieve user input from the console.
 *
 * min  minimum number to accept
 * max  maximum number to accept
 *
 * @return numeric value entered from the console.
 */
int getNumber (int min, int max)
{
   int value = min,cnt;
   int done = FALSE;
   do
   {
      cnt = scanf("%d",&value);
      if(cnt>0 && (value>max || value<min))
      {
         printf("Value (%d) is outside of the limits (%d,%d)\n",value,min,max);
         printf("Try again:\n");
      }
      else
         done = TRUE;

   }
   while(!done);

   return value;

}

/**
 * Display menu and ask for a selection.
 *
 * menu  the menu selections.
 *
 * @return numberic value entered from the console.
 */
int menuSelect(uchar *SNum)
{
   int length;

   printf("\n");
   printf("Data Entry Mode\n");
   printf("(0) Text (single line)\n");
   printf("(1) Hex (XX XX XX XX ...)\n");
   length = 1;

   printf("\nPlease enter value: ");

   return getNumber(0, length);
}