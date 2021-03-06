/* bcal-performance.c */
/*
    This file is part of the ARM-Crypto-Lib.
    Copyright (C) 2006-2010  Daniel Otte (daniel.otte@rub.de)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * \file    bcal-performance.c
 * \author  Daniel Otte
 * \email   daniel.otte@rub.de
 * \date    2010-02-16
 * \license GPLv3 or later
 *
 */

#include "bcal-performance.h"
#include "keysize_descriptor.h"
#include "blockcipher_descriptor.h"
#include "performance_test.h"
#include "cli.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


static
void printvalue(unsigned long v){
	char str[20];
	int i;
	ultoa(v, str, 10);
	for(i=0; i<10-strlen(str); ++i){
		cli_putc(' ');
	}
	cli_putstr(str);
}

void bcal_performance(const bcdesc_t* bcd){
	bcdesc_t bc;
	memcpy(&bc, bcd, sizeof(bcdesc_t));
	uint8_t ctx[bc.ctxsize_B];
	uint8_t data[(bc.blocksize_b+7)/8];
	uint16_t keysize = get_keysize(bc.valid_keysize_desc);
	uint8_t key[(keysize+7)/8];
	uint64_t t;
	uint8_t i;

	if(bc.type!=BCDESC_TYPE_BLOCKCIPHER)
		return;
	calibrateTimer();
	print_overhead();
	cli_putstr("\r\n\r\n === ");
	cli_putstr(bc.name);
	cli_putstr(" performance === "
	           "\r\n    type:             blockcipher"
	           "\r\n    keysize (bits):     ");
	printvalue(keysize);

	cli_putstr("\r\n    ctxsize (bytes):    ");
	printvalue(bc.ctxsize_B);

	cli_putstr("\r\n    blocksize (bits):   ");
	printvalue(bc.blocksize_b);

	t=0;
	if(bc.init.init1){
		if((bc.flags&BC_INIT_TYPE)==BC_INIT_TYPE_1){
			for(i=0; i<32; ++i){
				startTimer(0);
				START_TIMER;
				(bc.init.init1)(key, &ctx);
				STOP_TIMER;
				t += stopTimer();
				if(i!=31 && bc.free){
					bc.free(&ctx);
				}
			}
		} else {
			for(i=0; i<32; ++i){
				startTimer(0);
				START_TIMER;
				(bc.init.init2)(key, keysize, &ctx);
				STOP_TIMER;
				t += stopTimer();
				if(i!=31 && bc.free){
					bc.free(&ctx);
				}
			}
		}
		t>>=5;
		cli_putstr("\r\n    init (cycles):      ");
		printvalue(t);
	}

	t=0;
	for(i=0; i<32; ++i){
		startTimer(0);
		START_TIMER;
		bc.enc.enc1(data, &ctx);
		STOP_TIMER;
		t += stopTimer();
	}
	t>>=5;
	cli_putstr("\r\n    encrypt (cycles):   ");
	printvalue(t);

	t=0;
	for(i=0; i<32; ++i){
		startTimer(0);
		START_TIMER;
		bc.dec.dec1(data, &ctx);
		STOP_TIMER;
		t += stopTimer();
	}
	t>>=5;
	cli_putstr("\r\n    decrypt (cycles):   ");
	printvalue(t);

	if(bc.free){
		bc.free(&ctx);
	}

}


void bcal_performance_multiple(const bcdesc_t** bcd_list){
	const bcdesc_t* bcd;
	for(;;){
		bcd = *bcd_list;
		if(!bcd){
			cli_putstr("\r\n\r\n End of performance figures\r\n");
			return;
		}
		bcal_performance(bcd++);
		bcd_list++;
	}
}
