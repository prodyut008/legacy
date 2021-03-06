/* base64_enc.c */
/*
 *   This file is part of AnonAccess, an access system which can be used
 *    to open door or doing other things with an anonymity featured
 *    account managment.
 *   Copyright (C) 2006, 2007, 2008  Daniel Otte (daniel.otte@rub.de)
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


/**
 * base64 encoder (RFC3548)
 * Author: Daniel Otte
 * License: GPLv3
 * 
 * 
 */

#include <stdint.h>
#include "base64_enc.h"


static 
char bit6toAscii(uint8_t a){
	a &= (uint8_t)0x3F;
	
	if(a<=25){
		return a+'A';
	} else {
		if(a<=51){
			return a-26+'a';
		} else {
			if(a<=61){
				return a-52+'0';
			} else {
				if(a==62){
					return '+';
				} else {
					return '/'; /* a == 63 */
				}
			}
		}
	}
}

void base64enc(char* dest, void* src, uint16_t length){
	uint16_t i,j;
	uint8_t a[4];
	for(i=0; i<length/3; ++i){
		a[0]= (((uint8_t*)src)[i*3+0])>>2;
		a[1]= (((((uint8_t*)src)[i*3+0])<<4) | ((((uint8_t*)src)[i*3+1])>>4)) & 0x3F;
		a[2]= (((((uint8_t*)src)[i*3+1])<<2) | ((((uint8_t*)src)[i*3+2])>>6)) & 0x3F;
		a[3]= (((uint8_t*)src)[i*3+2]) & 0x3F;
		for(j=0; j<4; ++j){
			*dest++=bit6toAscii(a[j]);
		}
	}
	/* now we do the rest */
	switch(length%3){
		case 0: 
			break;
		case 1:
			a[0]=(((uint8_t*)src)[i*3+0])>>2;
			a[1]=((((uint8_t*)src)[i*3+0])<<4)&0x3F;
			*dest++=bit6toAscii(a[0]);
			*dest++=bit6toAscii(a[1]);
			*dest++='=';
			*dest++='=';
			goto finalise;
		case 2:		
			a[0]= (((uint8_t*)src)[i*3+0])>>2;
			a[1]= (((((uint8_t*)src)[i*3+0])<<4) | ((((uint8_t*)src)[i*3+1])>>4)) & 0x3F;
			a[2]= ((((uint8_t*)src)[i*3+1])<<2) & 0x3F;
			*dest++=bit6toAscii(a[0]);
			*dest++=bit6toAscii(a[1]);
			*dest++=bit6toAscii(a[2]);
			*dest++='=';
			goto finalise;
		default: /* this will not happen! */
			break;
	}
  finalise:
  	*dest='\0';
}

