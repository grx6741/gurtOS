#pragma once

#include <stdint.h>
#include <stdbool.h>

/*
 *	To send a command to the controller, write the command byte to IO port 0x64. 
 *	If the command is 2 bytes long, then the next byte needs to be written to IO Port 0x60 after 
 *	making sure that the controller is ready for it (by making sure bit 1 of the Status Register is clear). 
 *	If there is a response byte, then the response byte needs to be read from IO Port 0x60 after 
 *	making sure it has arrived (by making sure bit 0 of the Status Register is set). 
 * 
 *  From :: https://wiki.osdev.org/I8042_PS/2_Controller
 * */

#define PS2_DATA_PORT 0x60
#define PS2_STATUS_REGISTER 0x64
#define PS2_COMMAND_REGISTER 0x64

// Read only Status register at port PS2_STATUS_REGISTER
typedef union {
	uint8_t whole;
	struct {
		// 0 => empty, 1 => full
		uint8_t is_output_buffer_ready	: 1; 

		// 0 => empty, 1 => full
		uint8_t is_input_buffer_ready	: 1; 

		uint8_t system_flag				: 1;

		// 0 => data, 1 => command
		uint8_t data_or_command			: 1; 

		uint8_t _unknown1				: 1;
		uint8_t _unknown2				: 1;

		// 0 => error, 1 => timeout
		uint8_t error_or_timeout		: 1; 

		// 0 => no error, 1 => error
		uint8_t is_parity_error			: 1; 
	};
} status_register_t;

// helper Macros for status_register_t
#define IS_DATA(reg)					!(reg).data_or_command
#define IS_COMMAND(reg)					(reg).data_or_command
#define IS_ERROR(reg)					!(reg).error_or_timeout
#define IS_TIMEOUT(reg)					(reg).error_or_timeout

#define COMMAND_INVALID						0x00
#define COMMAND_READ_CONTROLLER_CONFIG		0x20
#define COMMAND_WRITE_CONTROLLER_CONFIG		0x60
#define COMMAND_SECOND_PS2_DISABLE			0xA7
#define COMMAND_SECOND_PS2_ENABLE			0xA8
#define COMMAND_SECOND_PS2_TEST				0xA9
#define COMMAND_PS2_CONTROLLER_TEST			0xAA
#define COMMAND_FIRST_PS2_TEST				0xAB
#define COMMAND_DIAGNOSTIC_DUMP				0xAC
#define COMMAND_FIRST_PS2_DISABLE			0xAD
#define COMMAND_FIRST_PS2_ENABLE			0xAE
#define COMMAND_READ_CONTROLLER_INPUT		0xC0
#define COMMAND_CPY_IN_LOW_TO_HIGH_STATUS	0xC1	// Copy bits 0 to 3 of input port to status bits 4 to 7 
#define COMMAND_CPY_IN_HIGH_TO_HIGH_STATUS	0xC2 	// Copy bits 4 to 7 of input port to status bits 4 to 7 
#define COMMAND_READ_CONTROLLER_OUTPUT		0xD0
#define COMMAND_WRITE_TO_CONTROLLER_OUTPUT	0xD1 	// Write next byte to Controller Output Port
#define COMMAND_WRITE_TO_FIRST_PS2_OUTPUT	0xD2 	// Write next byte to first PS/2 port output buffer
#define COMMAND_WRITE_TO_SECOND_PS2_OUTPUT	0xD3 	// Write next byte to second PS/2 port output buffer
#define COMMAND_WRITE_TO_SECOND_PS2_INPUT	0xD3 	// Write next byte to second PS/2 port input buffer
#define COMMAND_PULSE_STATUS_LINE(mask) (0xF0 | (mask)) // 	Pulse output line low for 6 ms. 
														// 	Bits 0 to 3 are used as a mask 
														// 	(0 = pulse line, 1 = don't pulse line) 
														// 	and correspond to 4 different output lines.

#define RESPONSE_TEST_PASSED					0x00
#define RESPONSE_TEST_CLOCK_LINE_STUCK_LOW		0x01
#define RESPONSE_TEST_CLOCK_LINE_STUCK_HIGHT	0x02
#define RESPONSE_TEST_CLOCK_DATA_STUCK_LOW		0x03
#define RESPONSE_TEST_CLOCK_DATA_STUCK_HIGHT	0x04
#define RESPONSE_CONTROLLER_TEST_PASSED			0x55
#define RESPONSE_CONTROLLER_TEST_FAILED			0xFC

typedef union {
	uint8_t whole;

	struct {
		// 1 => enabled, 0 => disabled
		uint8_t first_ps2_interrupt_enabled			: 1;

		uint8_t second_ps2_interrupt_enabled		: 1;

		// if this is 0, then OS shouldn't be running. wtf?? 😭 🥀🥀🥀
		uint8_t system_flag							: 1; 

		// this better be zero
		uint8_t should_be_zero						: 1;

		// FOR SOME FUCKING REASON, 1 => disabled, 0 => enabled
		uint8_t first_ps2_clock_disabled			: 1;

		// SAME SHI AS THE ONE ABOVE
		uint8_t second_ps2_clock_disabled			: 1;

		uint8_t first_ps2_port_translation_enabled	: 1;

		// bit 3 should be zero, BUT bit 7 MUST be zero
		uint8_t must_be_zero						: 1;
	};

} controller_config_t;

typedef union {
	uint8_t whole;

	struct {
		// Always set to 1
		uint8_t system_reset				: 1; 

		uint8_t a20_gate					: 1;

		uint8_t second_ps2_clock			: 1;

		uint8_t second_ps2_data				: 1;

		// Output buffer full with byte from first PS/2 port
		uint8_t output_from_first_ps2		: 1;

		// Output buffer full with byte from second PS/2 port
		uint8_t output_from_with_second_ps2	: 1;

		uint8_t first_ps2_clock				: 1;

		uint8_t first_ps2_data				: 1;
	};

} controller_output_t;

void ps2_wait_to_read_response();
void ps2_wait_to_send_command();

uint8_t ps2_read_response();
void ps2_send_command(uint8_t command);
void ps2_send_command_with_arg(uint8_t command, uint8_t arg);

bool ps2_init();
