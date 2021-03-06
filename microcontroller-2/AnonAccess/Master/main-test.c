/* main-test.c */
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


// main.c
/*
*
*/


/****************************************************
 ****************************************************
 *  INCLUDES
 ****************************************************
 ****************************************************/

#include "config.h"
#include "debug.h"
#include "uart.h"
#include "i2c.h"
#include "24CBlockDev.h"
#include "ticketDB.h"
#include "flmDB.h"
#include "entropium.h"
#include "enc2E24C.h"
#include "keys.h"
#include "main_test_tools.h"
#include "hwrnd.h"


/****************************************************
 ****************************************************
 *  MACROS
 ****************************************************
 ****************************************************/


#define TERM_LN_ERASE "\x1B[2K"
#define TERM_CUR_UP1 "\x1B[A"
#define TERM_CUR_UP (s) "\x1B["#(S)"A"

#define HELP_STR	 \
	"Press\r\n"\
	"a - to detect I2C devices\r\n"\
	"d - to dump an eeprom\r\n"\
	"f - to test blockDev driver\r\n"\
	"z - to set all bytes to 0xff in blockDev\r\n"\
	"c - to learn about your terminal\r\n"\
	"s - to test shabea implementation\r\n"  \
	"t - to get a timestamp\r\n" \
	"i - to init the database\r\n" \
	"n - to add a user to datatabase (hard)\r\n" \
	"m - to set admin flag\r\n" \
	"j - to clear admin flag\r\n" \
	"k - to initiate keymigration\r\n" \
	"+ - to add a user\r\n" \
	"# - to do action\r\n" \
	"b - to enter LA-Sim mode" \
	"\r\n"


/****************************************************
 * main()
 * **************************************************/
int main (void)
{
	i2c_detect_t dev_table;
	uart_init();
    // DEBUG_INIT();
    uart_putstr_P(PSTR("\r\nAnonAccess test system - MAIN\r\n"));
	main_tools_init();
    /*******************/
    i2c_set_speed(I2C_SPEED_100);
	/******************/

	load_eeprom_crypt_key(eeprom_key);

    uart_putstr_P(PSTR(HELP_STR));
    while(1){
    	uint8_t tc=uart_getc();
    	busy |=1;
    		switch (tc){
    //			case 't': thermo_dump(0x90); break;
    			case 'a': i2c_detect(dev_table); break;
    			case 'd': eeprom_dump_page(0xA0, 0, 512); break;
    			case 'l': eeprom_dump_page(0xA0, 0,2048); break;
    			case 'r': crypto_eeprom_dump(0, 512); break;
    			case 'R': crypto_eeprom_dump(0, 4096); break;
    			case 'z': E24C_blockdev_setBlock(0,0x00, 1024); break;
    			// {int i; for (i=0; i<1024; ++i){ E24C_blockdev_writeByte(i, 0xFF);}}; break;
    			case 'F': crypto_eeprom_dump(FLMDB_OFFSET, 4096); break;
    			case 'c': console_dbg(); break;
    			case 's': test_shabea256(); break;
    			case 't': console_dumptimestamp(); break;
    			case 'f': ticketdb_format(TICKETDB_SIZE); flmdb_format();/* break;*/
    			case 'i': ticketdb_init();dump_dbstats(); break;
    			case 'n': console_adduser_db(); break;
    			case 'm': console_setadmin(); break;
    			case 'j': console_clearadmin(); break;
    			
    			case 'k': do_keymigrate(); break;
    			
    			case '+': console_adduser(); break;
    			case '#': console_verifyuser(); break;
    			case 'b': console_lasim(); break;
    		/*	
    			case '1': E24C_blockdev_setBlock(0,'1',42); break;
    			case '2': E24C_blockdev_setBlock(0,'2',128); break;
    			case '3': E24C_blockdev_setBlock(1,'3',130); break;
    			case '4': E24C_blockdev_setBlock(14,'4',114); break;
    			case '5': E24C_blockdev_setBlock(0,'#',128LL*1024); break;
    		*/	
    			case '1': crypto_set_block(0, 0, 2342, eeprom_key); break; 
    			case '2': crypto_set_block(0, 0, 100, eeprom_key); break; 
    			case '3': { uint8_t nb[128];
    						memset(nb, 0, 128); 
    						encrypt_E24Cblock(nb, 0, eeprom_key);} break;
    			case '4': { uint8_t nb[128];
    						uint8_t db[128];
    						memset(nb, 0, 128); 
    						decrypt_E24Cblock(db, 0, eeprom_key);
    						uart_hexdump(db, 128);
    						if(memcmp(db, nb, 128)){
    							uart_putstr_P(PSTR("\r\ndecrypt failed"));
    						}else{
    							uart_putstr_P(PSTR("\r\ndecrypt successfull"));
    						}
    					} break;
    			case '5': { uint8_t nb[128];
    						uint8_t db[128];
    						memset(nb, 0, 128); 
    						crypto_read_block(db, 0, 128, eeprom_key);
    						uart_hexdump(db, 128);
    						if(memcmp(db, nb, 128)){
    							uart_putstr_P(PSTR("\r\ndecrypt failed"));
    						}else{
    							uart_putstr_P(PSTR("\r\ndecrypt successfull"));
    						}
    					} break;					 
    			
    			case '6': { uint8_t nb[128];
    						uint8_t i;
    						for(i=0; i<128; ++i){
    							nb[i]=i;
    						} 
    						crypto_write_block(nb, 0, 128, eeprom_key);
    					} break;
    			case '7': { uint8_t i;
    						for(i=0; i<120; ++i){
    							crypto_write_block(&i, i, 1, eeprom_key);
    						} 
    						
    					} break;
    			case '8': { uint8_t i=120;
    					//	for(i=0; i<120; ++i){
    							crypto_set_block(i, i, 1, eeprom_key);
    					//	} 
    						
    					} break;
    					
    			default: uart_putstr_P(PSTR(HELP_STR)); break;
    		}	
    		busy&= ~1;
    		uart_putstr_P(PSTR("\r\ndone\r\n"));
    }

    return (0);
}


