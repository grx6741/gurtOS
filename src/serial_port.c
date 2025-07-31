#include "serial_port.h"
#include "stdlib.h"

#include "io.h"

#define OFFSET_DATA_BUFFER  0
#define OFFSET_IER	    1
#define OFFSET_DIVISOR_LSB  0
#define OFFSET_DIVISOR_MSB  1
#define OFFSET_IIR	    2
#define OFFSET_FCR 	    2
#define OFFSET_LCR	    3
#define OFFSET_MCR 	    4
#define OFFSET_LSR 	    5
#define OFFSET_MSR 	    6
#define OFFSET_SCRATCH	    7

#define DATA_BIT_CHARACTER_LEN_5 0b00
#define DATA_BIT_CHARACTER_LEN_6 0b01
#define DATA_BIT_CHARACTER_LEN_7 0b10
#define DATA_BIT_CHARACTER_LEN_8 0b11

#define STOP_BIT_COUNT_1		0b0
#define STOP_BIT_COUNT_NOT_1	0b1 // this was 1.5 or 2, so idk what else to name it

#define PORT_PARITY_NONE     0b000 	
#define PORT_PARITY_ODD      0b001 	
#define PORT_PARITY_EVEN     0b011 	
#define PORT_PARITY_MARK     0b101 	
#define PORT_PARITY_SPACE    0b111 	

#define INTERRUPT_TRIGGER_LEVEL_1_BYTE	0b00
#define INTERRUPT_TRIGGER_LEVEL_4_BYTE 	0b01
#define INTERRUPT_TRIGGER_LEVEL_8_BYTE 	0b10
#define INTERRUPT_TRIGGER_LEVEL_14_BYTE 0b11

#define IIR_INTERRUPT_PENDING		0b0
#define IIR_INTERRUPT_NOT_PENDING	0b1

#define INTERRUPT_STATE_MODEM_STATUS					0b00 // Lowest Priority
#define INTERRUPT_STATE_TRANSMITTER_HOLDING_REG_EMPTY	0b01 //  |
#define INTERRUPT_STATE_RECIEVED_DATA_AVAILABLE			0b10 //  |
#define INTERRUPT_STATE_RECIEVER_LINE_STATUS			0b11 // Highest Priority

#define FIFO_BUFFER_STATE_NO_FIFO				0b00
#define FIFO_BUFFER_STATE_ENABLED_BUT_UNUSABLE	0b01
#define FIFO_BUFFER_STATE_ENABLED				0b10
#define FIFO_BUFFER_STATE_INVALID				0b11

#define READ_DATA(port)			inb(port + OFFSET_DATA_BUFFER)
#define READ_IER(port)			inb(port + OFFSET_IER)
#define READ_DIVISOR_LSB(port)	inb(port + OFFSET_DIVISOR_LSB)
#define READ_DIVISOR_MSB(port)	inb(port + OFFSET_DIVISOR_MSB)
#define READ_DIVISOR(port)		READ_DIVISOR_LSB(port) | (READ_DIVISOR_MSB(port) << 8)
#define READ_IIR(port)			inb(port + OFFSET_IIR)
#define READ_LCR(port)			inb(port + OFFSET_LCR)
#define READ_MCR(port)			inb(port + OFFSET_MCR)
#define READ_LSR(port)			inb(port + OFFSET_LSR)
#define READ_MSR(port)			inb(port + OFFSET_MSR)
#define READ_SCRATCH(port)		inb(port + OFFSET_SCRATCH)

#define WRITE_DATA(port, data)			outb(port + OFFSET_DATA_BUFFER, data)
#define WRITE_IER(port, data)			outb(port + OFFSET_IER, data)
#define WRITE_DIVISOR_LSB(port, data)	outb(port + OFFSET_DIVISOR_LSB, data)
#define WRITE_DIVISOR_MSB(port, data)	outb(port + OFFSET_DIVISOR_MSB, data)
#define WRITE_DIVISOR(port, data)		do { outb(port + OFFSET_DIVISOR_LSB, data & 0xFF); outb(port + OFFSET_DIVISOR_MSB, (data >> 8) & 0xFF); } while(0)
#define WRITE_FCR(port, data)			outb(port + OFFSET_FCR, data)
#define WRITE_LCR(port, data)			outb(port + OFFSET_LCR, data)
#define WRITE_MCR(port, data)			outb(port + OFFSET_MCR, data)
#define WRITE_SCRATCH(port, data)		outb(port + OFFSET_SCRATCH, data)

#define REGISTER_READ_FUNC_MAKER(reg)	\
	reg##_t read_##reg(uint16_t port) 	\
	{									\
		return (reg##_t) {				\
			.whole = READ_##reg(port)	\
		};								\
	}

REGISTER_READ_FUNC_MAKER(IER);
REGISTER_READ_FUNC_MAKER(IIR);
REGISTER_READ_FUNC_MAKER(LCR);
REGISTER_READ_FUNC_MAKER(MCR);
REGISTER_READ_FUNC_MAKER(LSR);
REGISTER_READ_FUNC_MAKER(MSR);

#define REGISTER_WRITE_FUNC_MAKER(reg)					\
	void write_##reg(uint16_t port, const reg##_t data) \
	{													\
		WRITE_##reg(port, (data).whole);				\
	}

REGISTER_WRITE_FUNC_MAKER(IER);
REGISTER_WRITE_FUNC_MAKER(FCR);
REGISTER_WRITE_FUNC_MAKER(LCR);
REGISTER_WRITE_FUNC_MAKER(MCR);

bool init_serial_port(uint16_t port)
{
	// outb(PORT + 1, 0x00);    Disable all interrupts
	write_IER(port, (IER_t) {
				.whole = 0
			});

	// outb(PORT + 3, 0x80);    Enable DLAB (set baud rate divisor)
	// 0x80 = 10000000

	write_LCR(port, (LCR_t) {
				.DLAB = 1
			});

	// outb(PORT + 0, 0x03);    Set divisor to 3 (lo byte) 38400 baud
	// outb(PORT + 1, 0x00);                     (hi byte)

	WRITE_DIVISOR(port, 0x0003);

	// outb(PORT + 3, 0x03);    8 bits, no parity, one stop bit
	// 0x03 = 00000011
	write_LCR(port, (LCR_t) { 
				.DLAB			= 0, 
				.data_bits		= DATA_BIT_CHARACTER_LEN_8, 
				.stop_bit		= STOP_BIT_COUNT_1, 
				.parity_bits	= PORT_PARITY_NONE
			});

	// outb(PORT + 2, 0xC7);    Enable FIFO, clear them, with 14-byte threshold
	// 0xC7 = 11000111
	write_FCR(port, (FCR_t) {
				.enable						= 1,
				.clear_recieve				= 1,
				.clear_transmit				= 1,
				.interrupt_trigger_level	= INTERRUPT_TRIGGER_LEVEL_14_BYTE
			});

	// outb(PORT + 4, 0x0B);    IRQs enabled, RTS/DSR set
	// 0x0B = 00001011
	write_MCR(port, (MCR_t) {
				.DTR	= 1,
				.RTS	= 1,
				.OUT2	= 1
			});

	// outb(PORT + 4, 0x1E);    Set in loopback mode, test the serial chip
	// 0x1E = 00011110
	write_MCR(port, (MCR_t) {
				.RTS   = 1,
				.OUT1  = 1,
				.OUT2  = 1,
				.loop  = 1
			});

	// outb(PORT + 0, 0xAE);    Test serial chip (send byte 0xAE and check if serial returns same byte)
	uint8_t data = 0xAE;
	WRITE_DATA(port, data);

	// Check if serial is faulty (i.e: not same byte as sent)
	// if(inb(PORT + 0) != 0xAE) {
	//     return 1;
	// }
	if (READ_DATA(port) != data) {
		return false;
	}

	// If serial is not faulty set it in normal operation mode
	// (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
	// outb(PORT + 4, 0x0F);
	// 0x0F = 00001111
	write_MCR(port, (MCR_t) {
				.DTR	= 1,
				.RTS 	= 1,
				.OUT1	= 1,
				.OUT2 	= 1,
				.loop 	= 0
			});

	return true;
}

void com1_putchar(const char data)
{
	while (read_LSR(COM1_PORT).THRE == 0);

	WRITE_DATA(COM1_PORT, data);
}

void com1_write(const char* data, size_t len)
{
	for (size_t i = 0; i < len; i++)
		com1_putchar(data[i]);
}

void com1_writestring(const char* data)
{
	com1_write(data, strlen(data));
}
