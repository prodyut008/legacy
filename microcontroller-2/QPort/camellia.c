/**
 * 
 * 
 * 
 * 
 */
 
#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "camellia.h"
#include "uart.h"
#include "debug.h"
#include <util/delay.h>
 
/*****************************************************************************/
uint64_t camellia_f(uint64_t x, uint64_t k);
/*****************************************************************************/
uint64_t camellia_fl(uint64_t x, uint64_t k);
/*****************************************************************************/
uint64_t camellia_fl_inv(uint64_t y, uint64_t k);
/*****************************************************************************/
void change_endian(void* data, uint8_t length);

uint64_t PROGMEM camellia_sigma[6]={ /* 64 byte table */
	0xA09E667F3BCC908BLL,
	0xB67AE8584CAA73B2LL,
	0xC6EF372FE94F82BELL,
	0x54FF53A5F1D36F1CLL,
	0x10E527FADE682D1DLL,
	0xB05688C2B3E6C1FDLL
};	

/* an ugly macro to load an entry form the table above */

#define SIGMA(p) (( ((uint64_t)(pgm_read_dword((prog_uint32_t*)camellia_sigma+2*(p)+1)))<<32) + \
				    ((uint64_t)(pgm_read_dword((prog_uint32_t*)camellia_sigma+2*(p)+0) )) )
/*
#define SIGMA(p) camellia_sigma[(p)]
*/
/*****************************************************************************/
/*
void camellia128_ctx_dump(camellia128_ctx_t *s){
	uart_putstr_P(PSTR("\r\n==State Dump=="));
	uart_putstr_P(PSTR("\n\rKAl: ")); uart_hexdump(&(s->kal), 8);
	uart_putstr_P(PSTR("\n\rKAr: ")); uart_hexdump(&(s->kar), 8);
	uart_putstr_P(PSTR("\n\rKLl: ")); uart_hexdump(&(s->kll), 8);
	uart_putstr_P(PSTR("\n\rKLr: ")); uart_hexdump(&(s->klr), 8);	
	return;
}
*/
/*****************************************************************************/
//*
//extern prog_uint64_t camellia_sigma[6];

void camellia128_init(camellia128_ctx_t* s, uint8_t* key){
	uint8_t i;
	s->kll = 0; //((uint64_t*)key)[0];
	
//	/ * load the key, endian-adjusted, to kll,klr * /
	for(i=0; i<8; ++i){
		s->kll <<= 8;
		s->kll |= *key++;
	}
	for(i=0; i<8; ++i){
		s->klr <<= 8;
		s->klr |= *key++;
	}
	/*
	{
		uint64_t x;
		uart_putstr("\r\n sigma_0: "); x=SIGMA(0); uart_hexdump(&x, 8);
		uart_putstr("\r\n sigma_1: "); x=SIGMA(1); uart_hexdump(&x, 8);
		uart_putstr("\r\n sigma_2: "); x=SIGMA(2); uart_hexdump(&x, 8);
		uart_putstr("\r\n sigma_3: "); x=SIGMA(3); uart_hexdump(&x, 8);
		uart_putstr("\r\n sigma_4: "); x=SIGMA(4); uart_hexdump(&x, 8);
		uart_putstr("\r\n sigma_5: "); x=SIGMA(5); uart_hexdump(&x, 8);
	} */
	
	s->kal = s->kll;
	s->kar = s->klr;
	
	s->kar ^= camellia_f(s->kal, SIGMA(0));
	s->kal ^= camellia_f(s->kar, SIGMA(1));
	
	s->kal ^= s->kll;
	s->kar ^= s->klr;
	
	s->kar ^= camellia_f(s->kal, SIGMA(2));
	s->kal ^= camellia_f(s->kar, SIGMA(3));
//	/ ** /
//	uart_putstr("\n\r----------------init finished--------------------");
}
//*/
/*****************************************************************************/
void camellia128_keyop(camellia128_ctx_t* s, int8_t q);
/*****************************************************************************/
void camellia128_keyop_inv(camellia128_ctx_t* s, int8_t q);
/*****************************************************************************/

#define SEL_KA 1
#define SEL_KL 0

#define KEY_POSTC1		0x00
#define KEY_POSTC2		0x01
#define KEY_INC2		0x02

#define KEY_DIR			0x04
#define KEY_DIR_NORM	0x00
#define KEY_DIR_INV		0x04

#define KEY_AMMOUNT		0x08 
#define KEY_ROL17		0x08
#define KEY_ROL15		0x00

void camellia_6rounds(camellia128_ctx_t* s, uint64_t* bl, uint64_t* br, uint8_t roundop, uint8_t keychoice);
/*****************************************************************************/


void camellia128_enc(camellia128_ctx_t* s, void* block){

	#define BL (((uint64_t*)block)[0])
	#define BR (((uint64_t*)block)[1])
	/* endian adjustment */
	 /*BL*/
	 /* 	1 2 3 4 5 6 7 8
	  *		8 7 6 5 4 3 2 1
	  */
/*
	uart_putstr_P(PSTR("\n\renc-entry"));
	camellia128_ctx_dump(s); 
*/	 
	uint64_t temp64;
	
	change_endian(&BL, 64/8);	
	change_endian(&BR, 64/8);
	
	/* Prewhitening */
	BL ^= s->kll;
	BR ^= s->klr;
	
	/* the first 6 */
	camellia_6rounds(s, &BL, &BR, KEY_ROL15 | KEY_DIR_NORM | KEY_POSTC1 , 0x33);
	/* FL injection  */
   camellia128_keyop(s, -1);
	BL = camellia_fl(BL, s->kal);
	BR = camellia_fl_inv(BR, s->kar);
   camellia128_keyop(s, -1);
	/* middle 6 */
	camellia_6rounds(s, &BL, &BR, KEY_ROL15 | KEY_DIR_NORM | KEY_INC2 , 0x34);
	/* FL injection  */
   camellia128_keyop(s, 1);
   	BL = camellia_fl(BL, s->kll);
	BR = camellia_fl_inv(BR, s->klr);
   camellia128_keyop(s, 1);
   /* last 6 */
	camellia_6rounds(s, &BL, &BR, KEY_ROL17 | KEY_DIR_NORM | KEY_POSTC2 , 0x0C);
	/* Postwhitening */
	BR ^= s->kal;
	BL ^= s->kar;
	
	temp64 = BR;
	BR = BL;
	BL = temp64;
/* fix? */
	camellia128_keyop(s,1);
	
	change_endian(&BL, 64/8);	
	change_endian(&BR, 64/8);
		
/*	
	uart_putstr_P(PSTR("\n\renc-exit"));
	camellia128_ctx_dump(s);	
*/
	#undef BL
	#undef BR	
}

/*****************************************************************************/

void camellia128_dec(camellia128_ctx_t* s, void* block){

	#define BL (((uint64_t*)block)[1])
	#define BR (((uint64_t*)block)[0])
	/* endian adjustment */
	 /*BL*/
	 /* 	1 2 3 4 5 6 7 8
	  *		8 7 6 5 4 3 2 1
	  */
/*	
	uart_putstr_P(PSTR("\n\rdec-entry"));
	camellia128_ctx_dump(s);
*/	
	uint64_t temp64;
		
	change_endian(&BL, 64/8);	
	change_endian(&BR, 64/8);
		
	camellia128_keyop_inv(s, 1);
	/* Prewhitening */
	BR ^= s->kal; /* kw3 */
	BL ^= s->kar; /* kw4 */
	/* the first 6 */
	camellia_6rounds(s, &BR, &BL, KEY_ROL17 | KEY_DIR_INV | KEY_POSTC1 , 0x0C);
	/* FL injection  */
   camellia128_keyop_inv(s, 1);
	BR = camellia_fl(BR, s->klr);
	BL = camellia_fl_inv(BL, s->kll);
   camellia128_keyop_inv(s, 1);
	/* middle 6 */	
	camellia_6rounds(s, &BR, &BL, KEY_ROL15 | KEY_DIR_INV | KEY_INC2 , 0x0B);
	/* FL injection  */
   camellia128_keyop_inv(s, -1);
   	BR = camellia_fl(BR, s->kar);
	BL = camellia_fl_inv(BL, s->kal);
   camellia128_keyop_inv(s, -1);
   /* last 6 */
	camellia_6rounds(s, &BR, &BL, KEY_ROL15 | KEY_DIR_INV | KEY_POSTC2 , 0x33);
	
	/* Postwhitening */
	BL ^= s->kll; /* kw1 */ 
	BR ^= s->klr; /* kw2 */
	
	temp64 = BR;
	BR = BL;
	BL = temp64;
	
	change_endian(&BL, 64/8);	
	change_endian(&BR, 64/8);
	
/*	
	uart_putstr_P(PSTR("\n\rdec-exit"));
	camellia128_ctx_dump(s);	
*/
}

/*****************************************************************************/
/*****************************************************************************/



/* EOF */
