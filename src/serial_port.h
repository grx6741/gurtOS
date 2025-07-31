#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// https://wiki.osdev.org/Serial_Ports

// Address and Offsets

#define COM1_PORT ((uintptr_t)0x3F8)
#define COM2_PORT ((uintptr_t)0x2F8)

typedef union {
	uint8_t whole;
	struct {
		uint8_t recieved_data_available				: 1;
		uint8_t transmitter_holding_register_empty	: 1;
		uint8_t reciever_line_status				: 1;
		uint8_t modem_status        				: 1;
		uint8_t _reserved							: 4;
	};
} IER_t; // Interrupt Enable Register
IER_t read_IER(uint16_t port);
void write_IER(uint16_t port, const IER_t IER_data);

typedef union {
	uint8_t whole;
	struct {
		uint8_t pending							: 1; // use IIR_INTERRUPT_(PENDING, NOT_PENDING)
		uint8_t interrupt_state                	: 2; // use INTERRUPT_STATE_*
		uint8_t timeout_interrupt_pending		: 1;
		uint8_t _reserved                		: 2;
		uint8_t fifo_buffer_state        		: 2;
	};
} IIR_t; //Interrupt Identification Register 
IIR_t read_IIR(uint16_t port);

typedef union {
	uint8_t whole;
	struct {
		uint8_t enable                	: 1;
		uint8_t clear_recieve        	: 1; 
		uint8_t clear_transmit        	: 1;
		uint8_t dma_mode_select         : 1;
		uint8_t _reserved               : 2;
		uint8_t interrupt_trigger_level	: 2; // use INTERRUPT_TRIGGER_LEVEL_(1, 4, 8, 14)_BYTE
	};
} FCR_t; // FIFO Control Register
void write_FCR(uint16_t port, const FCR_t FCR_data);

typedef union {
	uint8_t whole;
	struct {
		uint8_t data_bits        	: 2; // use DATA_BIT_CHARACTER_LEN_(5, 6, 7, 8)
		uint8_t stop_bit        	: 1; // use STOP_BIT_COUNT_(1, NOT_1)
		uint8_t parity_bits        	: 3; // use PORT_PARITY_(NONE, ODD, EVEN, MARK, SPACE)
		uint8_t break_enable_bit	: 1; // 
		uint8_t DLAB				: 1; // Divisor Latch Access Register
	};
} LCR_t; // Line Control Register
LCR_t read_LCR(uint16_t port);
void write_LCR(uint16_t port, const LCR_t LCR_data);

typedef union {
	uint8_t whole;
	struct {
		uint8_t DTR        	: 1; // Data Terminal Ready
		uint8_t RTS     	: 1; // Request to Send
		uint8_t OUT1        : 1;
		uint8_t OUT2        : 1;
		uint8_t loop        : 1;
		uint8_t _reserved	: 3;
	};
} MCR_t; // Modem Controll register
MCR_t read_MCR(uint16_t port);
void write_MCR(uint16_t port, const MCR_t MCR_data);

typedef union {
	uint8_t whole;
	struct {
		uint8_t DR	    : 1; // Data Ready
		uint8_t OE 	    : 1; // Overrun error
		uint8_t PE 	    : 1; // Parity Error
		uint8_t FE 	    : 1; // Framing Error
		uint8_t BI 	    : 1; // Break Indicator
		uint8_t THRE    : 1; // Transmitter holding register empty
		uint8_t TEMT    : 1; // Transmitter empty
		uint8_t IE	    : 1; // Impending Error
	};
} LSR_t; // Line Status Register
LSR_t read_LSR(uint16_t port);

typedef union {
	uint8_t whole;
	struct {
		uint8_t DCTS : 1; // Delta Clear to Send           
		uint8_t DDSR : 1; // Delta Data Set Ready	   
		uint8_t TERI : 1; // Trailing Edge of Ring Indicator
		uint8_t DDCD : 1; // Delta Data Carrier Detect	   
		uint8_t CTS  : 1; // Clear to Send           
		uint8_t DSR  : 1; // Data Set Ready           
		uint8_t RI   : 1; // Ring Indicator           
		uint8_t DCD  : 1; // Data Carrier Detect           
	};
} MSR_t; // Modem Status Register
MSR_t read_MSR(uint16_t port);

bool init_serial_port(uint16_t port);
void com1_putchar(const char data);
void com1_write(const char* data, size_t len);
void com1_writestring(const char* data);
