/* flmDB.c */
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


/*******************************************************************************
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 */

#include <string.h>
#include <stdint.h>
#include "config.h"
#include "types.h"
#include "flmDB.h"
#include "24CBlockDev.h"
#include "ticketDB.h"
#include "rtc.h"
#include "uart.h"

#define FLMDB_MAXID (FLMDB_SIZE/sizeof(flmdb_entry_t)-1)
/*
#define READ_BLOCK(a, d, s)  E24C_blockdev_readBlock((a),(d),(s))
#define WRITE_BLOCK(a, d, s) E24C_blockdev_writeBlock((a),(d),(s))
#define SET_BLOCK(a,v,s)     E24C_blockdev_setBlock((a),(v),(s))
*/
#include "keys.h"
#include "enc2E24C.h"

#define READ_BLOCK(a, d, s)  crypto_read_block((d),(a),(s), eeprom_key)
#define WRITE_BLOCK(a, d, s) crypto_write_block((d),(a),(s), eeprom_key)
#define SET_BLOCK(a,v,s)     crypto_set_block((v),(a),(s), eeprom_key)

void flmdb_loadentry(flmdb_entry_t * entry, entryid_t id){
	if(id>FLMDB_MAXID)
		return;
	READ_BLOCK(FLMDB_OFFSET + id*sizeof(flmdb_entry_t),
	                        entry, sizeof(flmdb_entry_t));
	return;
}

void flmdb_setentry(flmdb_entry_t * entry, entryid_t id){
	if(id>FLMDB_MAXID)
		return;
	WRITE_BLOCK(FLMDB_OFFSET + id*sizeof(flmdb_entry_t), 
	                         entry, sizeof(flmdb_entry_t));
	return;
}

void flmdb_process(uint8_t * searchmac, userid_t uid, userflags_t * flags){
	entryid_t i;
	flmdb_entry_t entry;
/*	
	uart_putstr_P(PSTR("\r\n searchmac: "));
	uart_hexdump(searchmac, 32);
*/	
	for(i=0; i<= FLMDB_MAXID; ++i){
		flmdb_loadentry(&entry, i);
//		uart_putc('~');
		if(entry.active && !memcmp(&(entry.hnick), searchmac, sizeof(hnick_t))){
//			uart_putc('-');
			ticketdb_getUserFlags(uid, flags);
			/* apply flag modifiers */
			flags->admin            |= entry.setflags.admin;
			flags->exist            |= entry.setflags.exist;
			flags->locked           |= entry.setflags.locked;
			flags->notify_lostadmin |= entry.setflags.notify_lostadmin;
			flags->force_admin_pin  |= entry.setflags.force_admin_pin;
			flags->force_normal_pin |= entry.setflags.force_normal_pin;
			flags->lock_nick        |= entry.setflags.lock_nick;
			flags->admin            &= ~entry.clearflags.admin;
			flags->exist            &= ~entry.clearflags.exist;
			flags->locked           &= ~entry.clearflags.locked;
			flags->notify_lostadmin &= ~entry.clearflags.notify_lostadmin;
			flags->force_admin_pin  &= ~entry.clearflags.force_admin_pin;
			flags->force_normal_pin &= ~entry.clearflags.force_normal_pin;
			flags->lock_nick        &= ~entry.clearflags.lock_nick;
			
			/* sync with db */
			ticketdb_setUserFlags(uid, flags);
			// entry.active = 0;
			if(!entry.permanent){
				uint8_t t;
				t = entry.last;
				memset(&entry, 0, sizeof(flmdb_entry_t));
				entry.last = t;
				flmdb_setentry(&entry, i);
			}
		}
		if(entry.last){
			return;
		}
	}
}

void flmdb_makeentry(uint8_t * mac, userflags_t setflags, userflags_t clearflags, uint8_t permanent){
	entryid_t i;
	flmdb_entry_t entry;
	flmdb_entry_t t;
	
	memcpy(entry.hnick, mac, sizeof(hnick_t));
	entry.active = 1;
	entry.last=0;
	entry.permanent = permanent?1:0;
	entry.timestamp = gettimestamp();
	entry.setflags = setflags;
	entry.clearflags = clearflags;
	entry.reserved[0]=entry.reserved[1]=entry.reserved[2]=0;
	
	for(i=0; i<=FLMDB_MAXID; ++i){
		flmdb_loadentry(&t, i);
		//READ_BLOCK(FLMDB_OFFSET + i*sizeof(flmdb_entry_t) + FLMDB_ACTIVE_OFFSET, &t, 1);
		if(t.last){
			if(t.active){
				if(i==FLMDB_MAXID){
					// ERROR!!!
//					uart_putstr_P(PSTR("\r\n ERROR: FLMDB full!!!"));
					return;
				}
				t.last=0;
				flmdb_setentry(&t, i);
				entry.last=1;
				flmdb_setentry(&entry, i+1);
				return;
			}else{
				entry.last=1;
				flmdb_setentry(&entry, i);
				return;
			}
		} else {
			if(!t.active){
				flmdb_setentry(&entry, i);
				return;
			}
		}
	}
}

void flmdb_format(void){
	flmdb_entry_t entry;
	SET_BLOCK(FLMDB_OFFSET, 0, FLMDB_SIZE);
	memset(&entry, 0, sizeof(flmdb_entry_t));
	entry.last = 1;
	flmdb_setentry(&entry, 0);
}


