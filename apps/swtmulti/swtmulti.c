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
//  swtmulti.c - This utility uses the DS2408 functions in the swt29.c file
//               to manipulate the device
//
//  Version:   3.10
//  History:
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "ownet.h"
#include "findtype.h"
#include "swt29.h"

// globals
long time_stamp;
// verbose output mode
int VERBOSE=0;

//----------------------------------------------------------------------
//  This is the Main routine for swtmulti
//
int main(short argc, char **argv)
{
   char msg[200];
   int portnum = 0;
   int n=0;
   ushort i;
   uchar sn[8],state[3], reg[3];
   uchar family[2][8];
   int done=FALSE; 
   int channel=0;
   uchar latch, set;

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
      if(FindDevices(portnum,&family[0],0x29,1))
      {
         for(i=0;i<8;i++)
            sn[i] = family[0][i];

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

            printf("(13) Set PIO conditional search\n");     // pio condition search

            printf("(14) Set activity conditional search\n");// activity condition search

            printf("(15) QUIT\n");

            scanf("%d",&n);
            if(n == 15)
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

                  if(readSwitch29(portnum,&sn[0],&state[0]))
                  {
                     printf("The channel is ");
                     if(getLatchState(channel,&state[0]))
                        printf("on.\n");
                     else
                        printf("off\n");
                     printf("The Level is ");
                     if(getLevel(channel,&state[0]))
                        printf("high.\n");
                     else
                        printf("low.\n");
                     if(getSensedActivity(channel,&state[0]))
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

                  if(setLatchState(portnum,&sn[0],channel,set))
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

                  if(readRegister29(portnum,&sn[0],&reg[0]))
                  {
                     printf("The Selection Mask for channel %d is ",channel);

                     latch = (uchar) (0x01 << channel);
                     if((reg[0] & latch) == latch)
                        printf("set.\n\n");
                     else
                        printf("is not set.\n\n");
                  }
                  else
                     OWERROR_DUMP(stdout);

                  break;

               case 4:  // Sets channel mask
                  printf("\nEnter the channel\n");
                  scanf("%d",&channel);
                  printf("Turn channel mask off enter 0, on 1.\n");
                  scanf("%d",&set);

                  if(setChannelMask(portnum,&sn[0],channel,set))
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

                  if(getChannelPolarity(portnum,&sn[0],channel))
                     printf("set.\n\n");
                  else
                     printf("not set.\n\n");

                  break;

               case 6:  // sets channel polarity
                  printf("\nEnter the channel\n");
                  scanf("%d",&channel);
                  printf("Turn channel polarity off enter 0, on 1.\n");
                  scanf("%d",&set);

                  if(setChannelPolarity(portnum,&sn[0],channel,set))
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
                  if(readRegister29(portnum,&sn[0],&reg[0]))
                     printf("The Constrol/Status register is as following in hex %02X\n\n",
                             reg[2]);
                  else
                     OWERROR_DUMP(stdout);

                  break;

               case 8:  // Set Reset Mode
                  printf("Turn reset mode off enter 0, on 1.\n");
                  scanf("%d",&set);

                  if(setResetMode(portnum,&sn[0],set))
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
                  if(clearPowerOnReset(portnum,&sn[0]))
                     printf("Power on reset was cleared.\n\n");
                  else
                     OWERROR_DUMP(stdout);

                  break;

               case 10:  // Get VCC state
                  if(readRegister29(portnum,&sn[0],&reg[0]))
                  {
                     if(getVCC(&reg[0]))
                        printf("VCC is powered.\n\n");
                     else
                        printf("VCC is grounded.\n\n");
                  }
                  else
                     OWERROR_DUMP(stdout);

                  break;

               case 11:  // or condition search
                  if(orConditionalSearch(portnum,&sn[0]))
                     printf("OR condition search was set.\n\n");
                  else
                     OWERROR_DUMP(stdout);

                  break;

               case 12:  // and condition search
                  if(andConditionalSearch(portnum,&sn[0]))
                     printf("AND condition search was set.\n\n");
                  else
                     OWERROR_DUMP(stdout);

                  break;

               case 13:  // pio condition search
                  if(pioConditionalSearch(portnum,&sn[0]))
                  {
                     printf("PIO condition search was set.\n\n");
                  }
                  else
                     OWERROR_DUMP(stdout);

                  break;

               case 14:  // activity condition search
                  if(activityConditionalSearch(portnum,&sn[0]))
                     printf("Activity condition search was set.\n\n");
                  else
                     OWERROR_DUMP(stdout);

                  break;

               default:
                  break;
            }

         }while(!done);

      }
      else
         printf("DS2408 not found on One Wire Network\n");

      owRelease(portnum);
   }

   return 1;
}

