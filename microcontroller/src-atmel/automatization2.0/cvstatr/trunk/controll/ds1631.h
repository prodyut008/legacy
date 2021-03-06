#ifndef _DS1631_H
#define _DS1631_H


extern uint8_t ds1631_init(uint8_t addr);
extern uint8_t ds1631_get_temp(uint8_t *p, uint8_t addr);

/* Initiates temperature conversions. If the part is in one-shot mode (1SHOT = 1), only one conversion is
 * performed. In continuous mode (1SHOT = 0), continuous temperature conversions are performed until a
 * Stop Convert T command is issued.
 */
#define DS1631_START_CONVERT		0x51

// Stops temperature conversions when the device is in continuous conversion mode (1SHOT = 0).
#define DS1631_STOP_CONVERT		0x22

// Reads last converted temperature value from the 2-byte temperature register.
#define DS1631_READ_TEMPERATURE	0xaa

// Reads or writes the 2-byte TH register.
#define DS1631_ACCESS_TH		0xa1

// Reads or writes the 2-byte TL register.
#define DS1631_ACCESS_TL		0xa2

// Reads or writes the 1-byte configuration register.
#define DS1631_ACCESS_CONFIG		0xac

/* Initiates a software power-on-reset (POR), which stops temperature conversions and resets all registers
 * and logic to their power-up states. The software POR allows the user to simulate cycling the power
 * without actually powering down the device.
 */
#define DS1631_SOFTWARE_POR		0x54

/* Config byte for DS1632
 * DONE—Temperature Conversion Done (Read Only) def = 1
 * THF—Temperature High Flag (Read/Write)       def = 0
 * TLF—Temperature Low Flag (Read/Write)        def = 0
 * NVB—NV Memory Busy (Read Only)               def = 0
 * R1—Resolution Bit 1 (Read/Write)             def = 1
 * R0—Resolution Bit 0 (Read/Write)             def = 1
 * POL*—TOUT Polarity (Read/Write)              factory = 0
 * 1SHOT*—Conversion Mode (Read/Write)          factory = 0
 * *EEPROM BACKED
 */
 
enum {
	ONESHOT	= 0x00,
	POL		= 0x01,
	R0		= 0x02,
	R1		= 0x03,
	NVB		= 0x04,
	TLF		= 0x05,
	THF		= 0x06,
	DONE	= 0x07
	} ds1631_config_reg;
#define DS1631_I2CDEFAULTCONFIG		(_BV(DONE) | _BV(R1) | _BV(R0))

#endif // _DS1631_H

