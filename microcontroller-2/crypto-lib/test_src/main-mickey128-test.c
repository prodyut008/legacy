/*
 * Mickey128 test-suit
 * 
*/

#include "config.h"
#include "serial-tools.h"
#include "uart.h"
#include "debug.h"
#include "cli.h"

#include "mickey128.h"
#include "nessie_stream_test.h"

#include <stdint.h>
#include <string.h>

char* algo_name = "Mickey128";

/*****************************************************************************
 *  additional validation-functions											 *
 *****************************************************************************/
void mickey128_genctx_dummy(uint8_t* key, uint16_t keysize_b, void* ctx){
	mickey128_init(key, keysize_b, NULL, 0, ctx);
}

void testrun_nessie_mickey128(void){
	nessie_stream_ctx.outsize_b =   8; /* actually unused */
	nessie_stream_ctx.keysize_b = 128; /* this is the one we have refrence vectors for */
	nessie_stream_ctx.ivsize_b  =   0;
	nessie_stream_ctx.name = algo_name;
	nessie_stream_ctx.ctx_size_B = sizeof(mickey128_ctx_t);
	nessie_stream_ctx.cipher_genctx = (nessie_stream_genctx_fpt)mickey128_genctx_dummy;
	nessie_stream_ctx.cipher_enc = (nessie_stream_genenc_fpt)mickey128_getbyte;
	
	nessie_stream_run();	
}

void testrun_ref_mickey128(void){
	uint8_t key[4][16] ={
		                  { 0x12, 0x34, 0x56, 0x78, 
	                        0x9a, 0xbc, 0xde, 0xf0, 
	                        0x01, 0x23, 0x45, 0x67, 
	                        0x89, 0xab, 0xcd, 0xef},
	                      { 0xf1, 0x1a, 0x56, 0x27, 
	                        0xce, 0x43, 0xb6, 0x1f, 
	                        0x89, 0x12, 0x29, 0x94, 
	                        0x86, 0x09, 0x44, 0x86},
	                      { 0x3b, 0x80, 0xfc, 0x8c, 
	                        0x47, 0x5f, 0xc2, 0x70, 
	                        0xfa, 0x26, 0xb4, 0x70, 
	                        0x64, 0xb3, 0x2d, 0x33},
						  { 0x82, 0xac, 0xb3, 0x8c,
						    0x5d, 0x7a, 0x3c, 0x78,
							0xd9, 0x8f, 0x15, 0x3c,
							0xa3, 0xf9, 0xde, 0x7c} };
	uint8_t iv_1[] = {0x21, 0x43, 0x65, 0x87};
	uint8_t iv_2[] = {0x9c, 0x53, 0x2f, 0x8a, 
	                  0xc3, 0xea, 0x4b, 0x2e, 
	                  0xa0, 0xf5, 0x96, 0x40, 
	                  0x30, 0x83, 0x77, 0xcc};
	uint8_t data[16];
	uint8_t i;
	mickey128_ctx_t ctx;
	cli_putstr_P(PSTR("\r\n-=Reference test=-\r\n"));
	
	cli_putstr_P(PSTR("Test 1:\r\n key:    "));
	cli_hexdump(key[0], 16);
	cli_putstr_P(PSTR("\r\n iv:     "));
	cli_hexdump(iv_1, 4);
	mickey128_init(&(key[0][0]), 128, iv_1, 4*8, &ctx);
	for(i=0; i<16; ++i){
		data[i] = mickey128_getbyte(&ctx);
	}
	cli_putstr_P(PSTR("\r\n stream: "));
	cli_hexdump(data, 16);
	
	cli_putstr_P(PSTR("\r\n\r\nTest 2:\r\n key:    "));
	cli_hexdump(key[1], 16);
	cli_putstr_P(PSTR("\r\n iv:     "));
	cli_hexdump(iv_2, 16);
	mickey128_init(key[1], 128, iv_2, 16*8, &ctx);
	for(i=0; i<16; ++i){
		data[i] = mickey128_getbyte(&ctx);
	}
	cli_putstr_P(PSTR("\r\n stream: "));
	cli_hexdump(data, 16);
	
	cli_putstr_P(PSTR("\r\n\r\nTest 3:\r\n key:    "));
	cli_hexdump(key[2], 16);
	cli_putstr_P(PSTR("\r\n iv:     "));
	mickey128_init(key[2], 128, NULL, 0, &ctx);
	for(i=0; i<16; ++i){
		data[i] = mickey128_getbyte(&ctx);
	}
	cli_putstr_P(PSTR("\r\n stream: "));
	cli_hexdump(data, 16);

	cli_putstr_P(PSTR("\r\n\r\nIteration test:\r\n key:    "));
	cli_hexdump(key[3], 16);
	mickey128_init(key[3], 128, NULL, 0, &ctx);
	uint16_t j;
	for(j=0; j<1000; ++j){
		for(i=0; i<16; ++i)
			key[0][i] = mickey128_getbyte(&ctx);
		for(i=0; i<4; ++i)
			iv_1[i] = mickey128_getbyte(&ctx);
		mickey128_init(key[0], 128, iv_1, 32, &ctx);
	}
		for(i=0; i<16; ++i){
		data[i] = mickey128_getbyte(&ctx);
	}
	cli_putstr_P(PSTR("\r\n stream: "));
	cli_hexdump(data, 16);
	cli_putstr_P(PSTR("\r\n"));

}



/*****************************************************************************
 *  main																	 *
 *****************************************************************************/

const char nessie_str[]      PROGMEM = "nessie";
const char test_str[]        PROGMEM = "test";
const char performance_str[] PROGMEM = "performance";
const char echo_str[]        PROGMEM = "echo";

cmdlist_entry_t cmdlist[] PROGMEM = {
	{ nessie_str,      NULL, testrun_nessie_mickey128},
	{ test_str,        NULL, testrun_ref_mickey128},
	{ echo_str,    (void*)1, (void_fpt)echo_ctrl},
	{ NULL,            NULL, NULL}
};

int main (void){
	DEBUG_INIT();
	
	cli_rx = uart_getc;
	cli_tx = uart_putc;	 	
	for(;;){
		cli_putstr_P(PSTR("\r\n\r\nCrypto-VS ("));
		cli_putstr(algo_name);
		cli_putstr_P(PSTR(")\r\nloaded and running\r\n"));
		cmd_interface(cmdlist);
	}
}
