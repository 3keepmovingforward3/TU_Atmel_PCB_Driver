/*
 * i2c.h
 *
 * Created: 7/15/2020 11:35:22 AM
 *  Author: Benjamin Blouin
 */


#ifndef I2C_H_
#define I2C_H_

#ifdef __cplusplus
extern "C" {
	#endif

	#define F_I2C			400000UL	// clock i2c
	#define PSC_I2C			1		// prescale i2c
	#define SET_TWBR		(F_CPU/F_I2C-16UL)/(PSC_I2C*2UL)

	#include <stdio.h>
	#include <avr/io.h>

	extern uint8_t I2C_ErrorCode;		// variable for communication error at twi
						// 0 means no error
	
	// I2C-ErrorCode Bit Definitions:
	//
	#define I2C_START		0			// bit 0: timeout start-condition
	#define I2C_SENDADRESS		1			// bit 0: timeout device-address
	#define I2C_BYTE		2			// bit 0: timeout byte-transmission
	#define I2C_READACK		3			// bit 0: timeout read acknowledge
	#define I2C_READNACK		4			// bit 0: timeout read no-acknowledge

	void i2c_init(void);				// init hardware-i2c
	void i2c_start(uint8_t i2c_addr);		// Send i2c_start_condition
	void i2c_stop(void);				// Send i2c_stop_condition
	void i2c_byte(uint8_t byte);			// Send data_byte

	// Helper Functions
	//
	void i2c_tx_start(void);
	void i2c_tx_address(uint8_t addr);
	void i2c_tx_byte(uint8_t ch);
	void i2c_tx_stop(void);

	// Data Read Functions
	//
	uint8_t i2c_readAck(void);          // read byte with ACK
	uint8_t i2c_readNAck(void);         // read byte with NACK

	#ifdef __cplusplus
}
#endif

#endif /* I2C_H_ */
