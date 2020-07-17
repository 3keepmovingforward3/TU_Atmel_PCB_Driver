/*
 * avr_i2c.c
 *
 * Created: 7/15/2020 11:35:10 AM
 *  Author: Benjamin Blouin
 */

#define F_CPU 16000000UL // Micro controller CPU

// Two-Wire(TWI) Register Macro
//
#define TWBR TWBR0 //TWI Bit Rate Register
#define TWCR TWCR0 //TWI
#define TWDR TWDR0
#define TWSR TWSR0
#define TWAR TWAR0

#include "avr_i2c.h"

/*
	Compiler Configuration
*/

// Compiler MCU Macro Check
// Checks that avr-GCC has these MCUs listed
// This is probably for those not using Atmel Studio, i.e. CLI
#if defined (__AVR_ATmega328__) || defined(__AVR_ATmega328P__) || \
defined(__AVR_ATmega168P__) || defined(__AVR_ATmega168PA__) || \
defined(__AVR_ATmega88P__) || \
defined(__AVR_ATmega48P__) || \
defined(__AVR_ATmega1284P__) || \
defined (__AVR_ATmega324A__) || defined (__AVR_ATmega324PB__) || defined (__AVR_ATmega324PA__) || \
defined (__AVR_ATmega644__) || defined (__AVR_ATmega644A__) || defined (__AVR_ATmega644P__) || defined (__AVR_ATmega644PA__) || \
defined (__AVR_ATmega1284P__) || \
defined (__AVR_ATmega2560__)

// Project Configuration Compiler Error Flags
// Checks out-of-bounds prescale and I2C bit-rate
// Trying to catch specific problems
//
#if PSC_I2C != 1 && PSC_I2C != 4 && PSC_I2C != 16 && PSC_I2C != 64  //Check prescale is valid
#error "Wrong prescaler for TWI !" //Compiler error
#elif SET_TWBR < 0 || SET_TWBR > 255 
#error "TWBR out of range, change PSC_I2C or F_I2C !"
#endif

uint8_t I2C_ErrorCode; // I2C-ErrorCode Variable

/**********************************************
 Public Function: i2c_init

 Purpose: Initialize TWI/I2C interface

 Input Parameter: none

 Return Value: none
 **********************************************/
void i2c_init(void){
    // Set clock prescale
	//
    switch (PSC_I2C) {
        case 4:
            TWSR = 0x1;
            break;
        case 16:
            TWSR = 0x2;
            break;
        case 64:
            TWSR = 0x3;
            break;
        default:
            TWSR = 0x00;  //1x prescale
            break;
    }

	// Set TWI bit-rate
	// Must recast from macro to variable
	//
    TWBR = (uint8_t)SET_TWBR;

    TWCR = (1 << TWEN); //TWI enable in TWI Control Register
}

/**********************************************
 Public Function: i2c_start

 Purpose: Start TWI/I2C interface

 Input Parameter:
 - uint8_t i2c_addr: Address of receiver

 Return Value: none
 **********************************************/
void i2c_start(uint8_t i2c_addr){

	// i2c start
	// A START condition is sent by writing TWCRn=1x10x10x
	// TWI Enable bit (TWCRn.TWEN) must be written to '1' to enable the 2-wire Serial Interface
	// TWI Start Condition bit (TWCRn.TWSTA) must be written to '1' to transmit a START condition
	// TWI Interrupt Flag (TWCRn.TWINT) must be written to '1' to clear the flag
	//
    TWCR = (1 << TWINT)|(1 << TWSTA)|(1 << TWEN);

	uint16_t timeout = F_CPU/F_I2C*2.0; // e.g. (16E6Hz)/(2.0* 4E5Hz) = 20

	// TWINT: TWI Interrupt Flag
	// This bit is set by hardware when the TWI n has finished its current job
	// and expects application software response.
	//
    while((TWCR & (1 << TWINT)) == 0 && timeout !=0){
		timeout--;
		if(timeout == 0){
			I2C_ErrorCode |= (1 << I2C_START);
			return;
		}
	};

	// TWIn Register Update
    // When the TWINT Flag is set, user must update all TWIn Registers
	// with the value relevant for the next TWIn bus cycle.
	//
	TWDR = i2c_addr;
    TWCR = (1 << TWINT)|( 1 << TWEN); // Put i2c_addr on TWI bus

	while((TWCR & (1 << TWINT)) == 0 && timeout !=0){ // wait to finish
		timeout--;
		if(timeout == 0){
			I2C_ErrorCode |= (1 << I2C_SENDADRESS);
			return;
		}
	};

}

/**********************************************
 Public Function: i2c_stop

 Purpose: Stop TWI/I2C interface

 Input Parameter: none

 Return Value: none

 Note TWSTO
 **********************************************/
void i2c_stop(void){
    // i2c stop
    TWCR = (1 << TWINT)|(1 << TWSTO)|(1 << TWEN);
}

/**********************************************
 Public Function: i2c_byte

 Purpose: Send byte at TWI/I2C interface

 Input Parameter:
 - uint8_t byte: Byte to send to reciever

 Return Value: none
 **********************************************/
void i2c_byte(uint8_t byte){
    TWDR = byte;
    TWCR = (1 << TWINT)|( 1 << TWEN);
    uint16_t timeout = F_CPU/F_I2C*2.0;
    while((TWCR & (1 << TWINT)) == 0 &&
		  timeout !=0){
		timeout--;
		if(timeout == 0){
			I2C_ErrorCode |= (1 << I2C_BYTE);
			return;
		}
	};
}

/**********************************************
 Public Function: i2c_readAck

 Purpose: read acknowledge from TWI/I2C Interface

 Input Parameter: none

 Return Value: uint8_t
  - TWDR: recieved value at TWI/I2C-Interface, 0 at timeout
  - 0:    Error at read
 **********************************************/
uint8_t i2c_readAck(void){
	// Notice TWI Enable Acknowledge bit
	// Acknowledging
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
    uint16_t timeout = F_CPU/F_I2C*2.0;
    while((TWCR & (1 << TWINT)) == 0 &&
		  timeout !=0){
		timeout--;
		if(timeout == 0){
			I2C_ErrorCode |= (1 << I2C_READACK);
			return 0;
		}
	};
    return TWDR;
}

 /**********************************************
 Public Function: i2c_readNAck

 Purpose: read non-acknowledge from TWI/I2C Interface

 Input Parameter: none

 Return Value: uint8_t
  - TWDR: received value at TWI/I2C-Interface
  - 0:    Error at read
 **********************************************/
uint8_t i2c_readNAck(void){
    TWCR = (1<<TWINT)|(1<<TWEN);
    uint16_t timeout = F_CPU/F_I2C*2.0;
    while((TWCR & (1 << TWINT)) == 0 &&
		  timeout !=0){
		timeout--;
		if(timeout == 0){
			I2C_ErrorCode |= (1 << I2C_READNACK);
            return 0;
		}
	};
    return TWDR;
}

//Modified section below
void i2c_tx_start(void){
	i2c_init();
}
void i2c_tx_address(uint8_t addr){
	i2c_start(addr);
}
void i2c_tx_byte(uint8_t ch){
	i2c_byte(ch);
}
void i2c_tx_stop(void){
	i2c_stop();
}

#else
#error "Micorcontroller not supported now!"
#endif
