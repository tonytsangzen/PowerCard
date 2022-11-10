/*	
 * Copyright 2001-2010 Georges Menie (www.menie.org)
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of California, Berkeley nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* this code needs standard functions memcpy() and memset()
   and input/output functions _inbyte() and _outbyte().
   the prototypes of the input/output functions are:
     int _inbyte(unsigned short timeout); // msec timeout
     void _outbyte(int c);
 */

#include "crc16.h"
#include "iap.h"

#define SOH  0x01
#define STX  0x02
#define EOT  0x04
#define ACK  0x06
#define NAK  0x15
#define CAN  0x18
#define CTRLZ 0x1A

#define DLY_1S 1000
#define MAXRETRANS 25
unsigned char xbuff[128 + 4]; /* 1024 for XModem 1k + 3 head chars + 2 crc + nul */
unsigned char rxState = 0;
unsigned char rxCount;
unsigned char rxPkgNum;
unsigned char rxCrc[2];
extern void myDevEP2_IN_Deal(uint8_t s);
#define _outbyte myDevEP2_IN_Deal
__attribute__((section(".highcode")))
static int check(const unsigned char *buf, int sz)
{
		unsigned short crc = crc16_ccitt(buf, sz);
		unsigned short tcrc = (buf[sz]<<8)+buf[sz+1];
		return (crc == tcrc);
}

__attribute__((section(".highcode")))
void xmodemReceive(unsigned char ch)
{
	switch(rxState){
		case 0:
			switch(ch){
				case SOH:			/*start of transfer*/
					rxState++;
					rxCount = 2;
					xbuff[1] = ch;
					break;
				case EOT:			/*end of transfer*/
					if(program_end() == 0)
						_outbyte(ACK);
					else
						_outbyte(NAK);
					break;
				case CAN:			/*cancel*/
					_outbyte(ACK);
					break;
				default:
					break;
			}
			break;
		case 1:
			xbuff[rxCount] = ch;
			rxCount++;
			if(rxCount == 134){
				rxState = 0;
				if(xbuff[2] == (uint8_t)(~xbuff[3])){
					if(check((const unsigned char*)&xbuff[4], 128)){
						if(program_process(&xbuff[4], 128) == 0){
							_outbyte(ACK);
							break;
						}
					}
				}
				_outbyte(NAK);
			}
			break;
		default:
			rxState = 0;
			break;
	}
}

