//---------------------------------------------------------------------------
// Copyright (C) 2004 Dallas Semiconductor Corporation, All Rights Reserved.
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
//  libusblses.c - Acquire and release a Session on the 1-Wire Net using the
//                 USB interface DS2490.  (Requires libusb 
//                 http://libusb.sourceforge.net or 
//                 http://libusb-win32.sourceforge.net)
//
//
//  Version: 3.00
//
//  History:
//

#include "ownet.h"
#include "libusbds2490.h"
#include <string.h>
#include <ctype.h>

// local functions
static void DS2490Discover(void);

// handles to USB ports
usb_dev_handle *usb_dev_handle_list[MAX_PORTNUM];

//struct usb_dev_handle *usb_dev_handle_list[MAX_PORTNUM];
struct usb_device *usb_dev_list[MAX_PORTNUM];
int usb_num_devices = -1;

static SMALLINT usbhnd_init = 0;

//---------------------------------------------------------------------------
// Attempt to acquire a 1-Wire net using a USB port and a DS2490 based
// adapter.
//
// 'portnum'    - number 0 to MAX_PORTNUM-1.  This number is provided to
//                indicate the symbolic port number.
// 'port_zstr'  - zero terminated port name.  For this platform
//                use format 'DS2490-X' where X is the port number.
//
// Returns: TRUE - success, USB port opened
//
SMALLINT owAcquire(int portnum, char *port_zstr)
{
	int retportnum = 0;
	retportnum = owAcquireEx(port_zstr);
	if ((retportnum < 0) || (retportnum != portnum))
	{
		// Failure from owAcquireEx or from not matching port numbers.
		return FALSE;
	}
	return TRUE;
}

//---------------------------------------------------------------------------
// Attempt to acquire the specified 1-Wire net.
//
// 'port_zstr'  - zero terminated port name.  For this platform
//                use format 'DS2490-X' where X is the port number.
//
// Returns: port number or -1 if not successful in setting up the port.
//
int owAcquireEx(char *port_zstr)
{
   int portnum = 0, portstringlength = 0, i = 0, ret = 0;
	int portnumfromstring; //character escape sequence
	//char portSubStr[] = "\\\\.\\DS2490-";
	char portSubStr[] = "DS2490-";
   char *retStr;
	char tempStr[4];

	// initialize tempStr
	memset(&tempStr[0],0x00,4);

	// convert supplied string to uppercase
	for (i = 0;i < (int)strlen(port_zstr); i++)
	{
		port_zstr[i] = (char)toupper((char)port_zstr[i]); 
	}

	// strip off portnumber from supplied string and put it in tempStr
   portstringlength = strlen(port_zstr) - strlen(&portSubStr[0]); // get port string length
	if ((portstringlength < 0) || (portstringlength > 3))
	{
		// error portstring length can only be 3 characters (i.e., "0" to "999")
		OWERROR(OWERROR_PORTNUM_ERROR);
		return -1;
	}
	for (i=0;i<portstringlength;i++) // tempStr holds the "stripped" port number (as a string)
	{
		//tempStr[i] = port_zstr[i+11];
		tempStr[i] = port_zstr[i + 7];
	}

	// check the port string and grab the port number from it.
    retStr = strstr (port_zstr, &portSubStr[0]); // check for "DS2490-"
    if (retStr == NULL)
    {
	    // error - owAcquire called with invalid port string
		OWERROR(OWERROR_PORTNUM_ERROR); 
	    return -1;
    }
	portnumfromstring = atoi(&tempStr[0]);
	
	if (portnumfromstring == 0) // port could be zero, so make an extra check befor failing
    {
		ret = memcmp(&tempStr[0],"0",1);
		if (ret != 0) 
		{
			// error - portnum supplied is not numeric
			OWERROR(OWERROR_PORTNUM_ERROR);
			return -1;
		}
	}

	// set portnum to the one provided
	portnum = portnumfromstring;

    if(!usbhnd_init)  // check to see the USB bus has been setup properly
    {
	    // discover all DS2490s on USB bus...
	    DS2490Discover();
        usbhnd_init = 1;
    }


    // check to see if the portnumber is valid
    if ((usb_num_devices < portnum) || (portnum == 0)) 
    {
		// error - Attempted to select invalid port number
        OWERROR(OWERROR_LIBUSB_NO_ADAPTER_FOUND);
	    return -1;
    }

    // check to see if opening the device is valid
    if (usb_dev_handle_list[portnum] != NULL) 
	{
		// error - USB Device already open
		OWERROR(OWERROR_LIBUSB_DEVICE_ALREADY_OPENED);
        return -1;
    }

    // open the device
    usb_dev_handle_list[portnum] = usb_open(usb_dev_list[portnum]);
    if (usb_dev_handle_list[portnum] == NULL) 
    {
        // Failed to open usb device
		OWERROR(OWERROR_LIBUSB_OPEN_FAILED);
		return -1;
    }

    // set the configuration
    if (usb_set_configuration(usb_dev_handle_list[portnum], 1)) 
    {
        // Failed to set configuration
		OWERROR(OWERROR_LIBUSB_SET_CONFIGURATION_ERROR);
        usb_close(usb_dev_handle_list[portnum]); // close handle
	    return -1;
    }

    // claim the interface
    if (usb_claim_interface(usb_dev_handle_list[portnum], 0)) 
    {
        // Failed to claim interface
	    //printf("%s\n", usb_strerror());
		OWERROR(OWERROR_LIBUSB_CLAIM_INTERFACE_ERROR);
	    usb_close(usb_dev_handle_list[portnum]); // close handle
		return -1;
	}

	// set the alt interface
	if (usb_set_altinterface(usb_dev_handle_list[portnum], 3)) 
	{
		// Failed to set altinterface
		//printf("%s\n", usb_strerror());
		OWERROR(OWERROR_LIBUSB_SET_ALTINTERFACE_ERROR);
		usb_release_interface(usb_dev_handle_list[portnum], 0); // release interface
		usb_close(usb_dev_handle_list[portnum]);   // close handle
		return -1;
	}

   
    // clear USB endpoints before doing anything with them.
	usb_clear_halt(usb_dev_handle_list[portnum], DS2490_EP3);
	usb_clear_halt(usb_dev_handle_list[portnum], DS2490_EP2);
	usb_clear_halt(usb_dev_handle_list[portnum], DS2490_EP1);

    // verify adapter is working
    if (!AdapterRecover(portnum))
    {
		usb_release_interface(usb_dev_handle_list[portnum], 0); // release interface
		usb_close(usb_dev_handle_list[portnum]);   // close handle
        return -1;
    }

    // reset the 1-Wire
    owTouchReset(portnum);

    return portnum;
}

//---------------------------------------------------------------------------
// Release the port previously acquired a 1-Wire net.
//
// 'portnum'    - number 0 to MAX_PORTNUM-1.  This number is provided to
//                indicate the symbolic port number.
//
void owRelease(int portnum)
{
	usb_release_interface(usb_dev_handle_list[portnum], 0);
	usb_close(usb_dev_handle_list[portnum]);
	usb_dev_handle_list[portnum] = NULL;
}

//---------------------------------------------------------------------------
// Discover all DS2490s on bus
//
void DS2490Discover(void)
{
	struct usb_bus *bus;
	struct usb_device *dev;

	// initialize USB subsystem
#ifdef LIBUSB_DEBUG_DISABLE  // Use this define at compile time to turn off LIBUSB debug info
   usb_set_debug(0);
#endif
   usb_init();
   usb_find_busses();
   usb_find_devices();

   usb_num_devices = 0; // avoid port zero to make it look like other builds

	for (bus = usb_get_busses(); bus; bus = bus->next) 
	{
		for (dev = bus->devices; dev; dev = dev->next) 
		{
			if (dev->descriptor.idVendor == 0x04FA &&
				 dev->descriptor.idProduct == 0x2490) 
			{
				usb_dev_list[++usb_num_devices] = dev;
			}
		}
	}
}
