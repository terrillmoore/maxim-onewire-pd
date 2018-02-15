Test application to perform various operations 
on the DS23E04 Dual Addressable Switch. 

Required on the command line is the 1-Wire port name:

example:  "COM1"                (Win32 DS2480B)
          "/dev/cua0"           (Linux DS2480B)
          "1"                   (Win32 TMEX)
          "\\.\DS2490-1"        (Win32 USB DS2490)
          "{1,5}"               (Win32 DS2480B multi build)
          "{1,6}"               (Win32 USB DS2490 multi build)
          "{1,2}"               (Win32 DS1410E multi build)

This application uses the 1-Wire Public Domain API. 
Implementations of this API can be found in the '\lib' folder.
The libraries are divided into three categories: 'general', 
'userial' and 'other'. Under each category are sample platform
link files. The 'general' and 'userial' libraries have 'todo' 
templates for creating new platform specific implementations.  

This application also uses utility and header files found
in the '\common' folder. 

Application File(s):		'\apps\swtlpop'
swtmain1c.c   - loops through different sequences to
		different DS28E04 operations

Common Module File(s):		'\common'
swt1C.h     -  	file to be included for the DS28E04
swt1C.c     - 	functions to operate the DS28E04.
crcutil.c   -   keeps track of the CRC for 16 and 8 bit operations
findtype.c  - 	finds all of the devices of the same type 
		(family code) on a 1-Wire Net
ownet.h    -   	include file for 1-Wire Net library
ioutil.c   - 	I/O utility functions


