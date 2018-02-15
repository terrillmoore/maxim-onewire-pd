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
//  swtduo.c - Goes through the testing of the DS2413 
//  Version 2.00


// Include files
#include <stdio.h>
#include <stdlib.h>
#include "ownet.h"
#include "swt3A.h"
#include "findtype.h"

// Constant definition
#define SWITCH_FAMILY      0x3A
#define MAXDEVICES         15

// local
int getNumber (int min, int max);


//--------------------------------------------------------------------------
// This is the begining of the program that tests the different Channels
int main(int argc, char **argv)
{
   int i,j,n;                      //loop counters
   short test=0;                   //info byte data
   short clear=0;                  //used to clear the button
   uchar SwitchSN[8];              //the serial numbers for the devices
   uchar send_block[10];
   short done = FALSE;             //used to indicate the end of the input loop from user
   int portnum=0;
   SMALLINT setA,setB;
   long errorcnt = 0;
   long cntA = 0;
   long cntB = 0;
   uchar state,read;
   int send_cnt;

   //----------------------------------------
   // Introduction header
   printf("\n/---------------------------------------------\n");
   printf("  swtest - V3.10\n"
          "  The following is a test to excersize the\n"
          "  different channels on the DS2406.\n");
   printf("  Press any CTRL-C to stop this program.\n\n");

   // check for required port name
   if (argc != 2)
   {
      printf("1-Wire Net name required on command line!\n"
             " (example: \"COM1\" (Win32 DS2480),\"/dev/cua0\" "
             "(Linux DS2480),\"{1,5}\" (Win32 TMEX)\n");
      exit(1);
   }

   // attempt to acquire the 1-Wire Net
   if((portnum = owAcquireEx(argv[1])) < 0)
   {
      OWERROR_DUMP(stdout);
      exit(1);
   }

   // success
   printf("Port opened: %s\n",argv[1]);

   owTouchReset(portnum);

   owWriteByte(portnum,0x33);
   SwitchSN[0] = (uchar)owReadByte(portnum);
   SwitchSN[1] = (uchar)owReadByte(portnum);
   SwitchSN[2] = (uchar)owReadByte(portnum);
   SwitchSN[3] = (uchar)owReadByte(portnum);
   SwitchSN[4] = (uchar)owReadByte(portnum);
   SwitchSN[5] = (uchar)owReadByte(portnum);
   SwitchSN[6] = (uchar)owReadByte(portnum);
   SwitchSN[7] = (uchar)owReadByte(portnum);

   for(j=0;j<8;j++)
      printf("%02X ",SwitchSN[j]);
   printf("\n");

   do
   {
      printf("PICK AN OPERATION:\n\n");
      printf("(1) PIO access read with confirmation\n");  // access read
      printf("(2) PIO access write with confirmation\n"); // access write
      printf("(3) PIO access read/write check.  Have A and B hooked up.\n");
      printf("(4) Read ROM command\n");                   // READ ROM
      printf("(5) Match ROM command\n");                  // Match ROM
      printf("(6) Skip ROM command\n");                   // Skip ROM
      printf("(7) Resume command\n");                     // Resume 
      printf("(8) OD Skip ROM and Match ROM command\n");  // OD skip/match ROM
      printf("(9) QUIT\n");

      scanf("%d",&n);
      if(n == 9)
      {
         n = 0;        //used to finish off the loop
         done = TRUE;
         break;
      }

      switch(n)
      {
         case 1:  // Access Read
            state = (uchar)owAccessRead(portnum,&SwitchSN[0],TRUE);
            printf("The PIO Status Pin is %02X\n",state);

            break;

         case 2:  // Write Access
            printf("\nEnter value for channel A (0,1):\n");
            scanf("%d",&setA);
            printf("Enter value for channel B (0,1):\n");
            scanf("%d",&setB);
            if(setA & setB)
            {
               state = 0xFF;
            }
            else if(setA & !setB)
            {
               state = 0xFE;
            }
            else if(!setA & setB)
            {
               state = 0xFD;
            }
            else
            {
               state = 0xFC;
            }

            if(owAccessWrite(portnum,&SwitchSN[0],TRUE,state))
            {
               printf("Value for channel A (%d) and B (%d) updated.\n",setA,setB);
            }
            else
               OWERROR_DUMP(stdout);

            break;

         case 3:
            errorcnt = 0;
            do
            {
               state = 0xFC;
               if(owAccessWrite(portnum,&SwitchSN[0],TRUE,state))
               {
                  read = (uchar)owAccessRead(portnum,&SwitchSN[0],TRUE);
                  if(read != 0xF0)
                  {
                     errorcnt++;
                     printf("The error count is %d\n",errorcnt);
                  }
               }

               state = 0xFD;
               if(owAccessWrite(portnum,&SwitchSN[0],TRUE,state))
               {
                  read = (uchar)owAccessRead(portnum,&SwitchSN[0],TRUE);
                  if(read != 0xD2)
                  {
                     errorcnt++;
                     printf("The error count is %d\n",errorcnt);
                  }
               }

               state = 0xFE;
               if(owAccessWrite(portnum,&SwitchSN[0],TRUE,state))
               {
                  read = (uchar)owAccessRead(portnum,&SwitchSN[0],TRUE);
                  if(read != 0x78)
                  {
                     errorcnt++;
                     printf("The error count is %d\n",errorcnt);
                  }
               }

               state = 0xFF;
               if(owAccessWrite(portnum,&SwitchSN[0],TRUE,state))
               {
                  read = (uchar)owAccessRead(portnum,&SwitchSN[0],TRUE);
                  if(read != 0x0F)
                  {
                     errorcnt++;
                     printf("The error count is %d\n",errorcnt);
                  }
               }
            }while(errorcnt < 100000000);

            break;

         case 4:  // Read ROM
            owTouchReset(portnum);

            owWriteByte(portnum,0x33);
            printf("%02X ",owReadByte(portnum));
            printf("%02X ",owReadByte(portnum));
            printf("%02X ",owReadByte(portnum));
            printf("%02X ",owReadByte(portnum));
            printf("%02X ",owReadByte(portnum));
            printf("%02X ",owReadByte(portnum));
            printf("%02X ",owReadByte(portnum));
            printf("%02X ",owReadByte(portnum));
            printf("\n");
               
            break;

         case 5:  // Match ROM
            owTouchReset(portnum);

            owWriteByte(portnum,0x55);
            owWriteByte(portnum,0x00);
            owWriteByte(portnum,0x00);
            owWriteByte(portnum,0x00);
            owWriteByte(portnum,0x00);
            owWriteByte(portnum,0x00);
            owWriteByte(portnum,0x00);
            owWriteByte(portnum,0x00);
            owWriteByte(portnum,0x00);
               
            owWriteByte(portnum,0xF5);
            printf("PIO status is %02X\n",owReadByte(portnum));

            break;

         case 6:  // Skip ROM
            owTouchReset(portnum);

            owWriteByte(portnum,0xCC);

            owWriteByte(portnum,0xF5);
            printf("PIO status is %02X\n",owReadByte(portnum));

            break;

         case 7:  // Resume command
            if (!owTouchReset(portnum))
               OWERROR_DUMP(stdout);

            send_cnt = 0;
            // match rom command
            send_block[send_cnt++] = 0x55;
            for(i=0;i<8;i++)
               send_block[send_cnt++] = 0x00;

            if(!owBlock(portnum,FALSE,send_block,send_cnt))
            {
               printf("Match failed.\n");
            }

            if (!owTouchReset(portnum))
               OWERROR_DUMP(stdout);

            // resume command
            owTouchByte(portnum,0xA5);

            owWriteByte(portnum,0xF5);
            printf("PIO status is %02X\n",owReadByte(portnum));

            break;

         case 8:  // OD Skip and Match ROM
            if (!owTouchReset(portnum))
               OWERROR_DUMP(stdout);

            if(!owWriteByte(portnum,0x3C))
               printf("OD skip rom error.\n");

            send_cnt = 0;
            if (owTouchReset(portnum))
            {
               // send the match command 0x69
               if (owWriteByte(portnum,0x69))
               {
                  // switch to overdrive communication speed
                  owSpeed(portnum,MODE_OVERDRIVE);

                  // create a buffer to use with block function
                  // Serial Number
                  for (i = 0; i < 8; i++)
                     send_block[send_cnt++] = 0x00;

                  // send/recieve the transfer buffer
                  if (!owBlock(portnum,FALSE,send_block,8))  // added ! needs to be removed
                  {
                     // verify that the echo of the writes was correct
                     for (i = 0; i < 8; i++)
                        if (send_block[i] != 0x00)
                        {
                           printf("bad echo for overdrive\n");
                        }
                  }
               }
            }

            owSpeed(portnum,MODE_NORMAL);

            if (!owTouchReset(portnum))
               OWERROR_DUMP(stdout);
                
            // overdrive skip rom
            if(!owWriteByte(portnum,0x3C))
               printf("OD skip rom error.\n");

            // switch to overdrive communication speed
            owSpeed(portnum,MODE_OVERDRIVE);

            owWriteByte(portnum,0xF5);
            printf("PIO status is %02X\n",owReadByte(portnum));

            // switch to normal communication speed
            owSpeed(portnum,MODE_NORMAL);

            break;

            default:
               break;
      }

   }while(!done);

   owRelease(portnum);
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