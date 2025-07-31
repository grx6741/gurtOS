#define LOG_PREFIX "PS2"
#include "stdio.h"

#include "ps2.h"

#include "io.h"

void ps2_wait_to_read_response()
{
	status_register_t status = { 0 };
	do {
		status.whole = inb(PS2_STATUS_REGISTER);
	} while (status.is_output_buffer_ready == 0);
}

void ps2_wait_to_send_command()
{
	status_register_t status = { 0 };
	do {
		status.whole = inb(PS2_STATUS_REGISTER);
	} while (status.is_input_buffer_ready == 1);
}

uint8_t ps2_read_response()
{
	ps2_wait_to_read_response();
	return inb(PS2_DATA_PORT);
}

void ps2_send_command(uint8_t command)
{
	ps2_wait_to_send_command();
	outb(PS2_COMMAND_REGISTER, command);
}

void ps2_send_command_with_arg(uint8_t command, uint8_t arg)
{
	ps2_wait_to_send_command();
	outb(PS2_COMMAND_REGISTER, command);

	ps2_wait_to_send_command();
	outb(PS2_DATA_PORT, arg);
}

bool ps2_init()
{
	status_register_t status;
	uint8_t resp;
	controller_config_t config;

	/*	Disable Devices
	 *
	 *	Devices connected to the PS/2 port(s) must be disabled so that 
	 *	they won't send data at the wrong time and mess up your initialisation; 
	 *	start by sending a command 0xAD and command 0xA7 to the PS/2 controller. 
	 *	If the controller is a single channel device, it will ignore the command 0xA7. 
	 */

	ps2_send_command(COMMAND_FIRST_PS2_DISABLE);
	ps2_send_command(COMMAND_SECOND_PS2_DISABLE);

	LOG("Disabled Divices\n");

	/* Flush The Output Buffer
	 *
	 * Sometimes (e.g. due to interrupt controller initialisation causing a lost IRQ), 
	 * data can get stuck in the PS/2 controller's output buffer. To guard against 
	 * this, now that the devices are disabled and can't send more data to 
	 * the output buffer, it is a good idea to flush the controller's output buffer. 
	 * You can do this by reading from port 0x60 and discarding the data.
	 */

	do {
		status.whole = inb(PS2_STATUS_REGISTER);
		if (status.is_output_buffer_ready) {
			inb(PS2_DATA_PORT);
		}
	} while (status.is_output_buffer_ready);

	LOG("Flush the Output Buffer\n");

	/* Set the Controller Configuration Byte
	 *
	 * This is as simple as reading in the Controller Configuration Byte (command 0x20), 
	 * changing some bits and then writing the new value back (command 0x60). 
	 * You want to disable IRQs and translation for port 1 by clearing bits 0 and 6. 
	 * You should also make sure the clock signal is enabled by clearing bit 4. 
	 */


	ps2_send_command(COMMAND_READ_CONTROLLER_CONFIG);

	config.whole = ps2_read_response();
	config.first_ps2_interrupt_enabled = 0;
	config.first_ps2_port_translation_enabled = 0;
	config.first_ps2_clock_disabled = 0;

	ps2_send_command_with_arg(COMMAND_WRITE_CONTROLLER_CONFIG, config.whole);

	LOG("Set the Controller Configuration Byte\n");

	/* Perform Controller Self Test
	 *
	 * To test the PS/2 controller, send command 0xAA to it, then wait for it to 
	 * respond and check if it replied with 0x55 (any value other than 0x55 indicates 
	 * a self-test fail). This can reset the PS/2 controller on some hardware (tested 
	 * on a 2016 laptop). At the very least, the Controller Configuration Byte should 
	 * be restored for compatibility with such hardware. You can either determine the 
	 * correct value yourself based on the above table or restore the value read 
	 * before issuing 0xAA.
	 */

	ps2_send_command(COMMAND_PS2_CONTROLLER_TEST);
	resp = ps2_read_response();
	if (resp != RESPONSE_CONTROLLER_TEST_PASSED) {
		LOG("Controller Test Failed\n");
		return false;
	}

	// Restore the previous config
	ps2_send_command_with_arg(COMMAND_WRITE_CONTROLLER_CONFIG, config.whole);

	LOG("PS/2 Controller Test Passed\n");

	/* Determine If There Are 2 Channels
	 *
	 * To determine if the controller is a dual channel one, send a command 0xA8 to 
	 * enable the second PS/2 port and read the Controller Configuration Byte (command 0x20). 
	 * Bit 5 of the Controller Configuration Byte should be clear - if it's set then 
	 * it can't be a dual channel PS/2 controller, because the second PS/2 port 
	 * should be enabled. If it is a dual channel device, send a command 0xA7 to 
	 * disable the second PS/2 port again and clear bits 1 and 5 of the 
	 * Controller Configuration Byte to disable IRQs and enable the clock for port 2 
	 * (You need not worry about disabling translation, because it is never 
	 * supported by the second port).
	 */

	ps2_send_command(COMMAND_SECOND_PS2_ENABLE);

	ps2_send_command(COMMAND_READ_CONTROLLER_CONFIG);
	config.whole = ps2_read_response();

	if (!config.second_ps2_clock_disabled) {
		LOG("Controller is Dual Channelled\n");
		ps2_send_command(COMMAND_SECOND_PS2_DISABLE);

		ps2_send_command(COMMAND_READ_CONTROLLER_CONFIG);
		config.whole = ps2_read_response();

		config.second_ps2_interrupt_enabled = 0;
		config.second_ps2_clock_disabled = 0;

		ps2_send_command_with_arg(COMMAND_WRITE_CONTROLLER_CONFIG, config.whole);
	} else {
		LOG("Controller is Single Channelled\n");
	}

	/* Perform Interface tests
	 *
	 * This step tests the PS/2 ports. Use command 0xAB to test the first PS/2 port, 
	 * then check the result. Then (if it's a dual channel controller) use command 
	 * 0xA9 to test the second PS/2 port, then check the result.
	 *
	 * At this stage, check to see how many PS/2 ports are left. If there aren't 
	 * any that work you can just give up (display errors and terminate the PS/2 
	 * Controller driver). Note: If one of the PS/2 ports on a dual PS/2 
	 * controller fails, then you can still keep using/supporting the other PS/2 port.
	 */

	ps2_send_command(COMMAND_FIRST_PS2_TEST);

	resp = ps2_read_response();
	if (resp != RESPONSE_TEST_PASSED) {
		switch (resp) {
			case RESPONSE_TEST_CLOCK_LINE_STUCK_LOW:	LOG("RESPONSE_TEST_CLOCK_LINE_STUCK_LOW\n"); break;
			case RESPONSE_TEST_CLOCK_LINE_STUCK_HIGHT:	LOG("RESPONSE_TEST_CLOCK_LINE_STUCK_HIGHT\n"); break;
			case RESPONSE_TEST_CLOCK_DATA_STUCK_LOW:	LOG("RESPONSE_TEST_CLOCK_DATA_STUCK_LOW\n"); break;
			case RESPONSE_TEST_CLOCK_DATA_STUCK_HIGHT:	LOG("RESPONSE_TEST_CLOCK_DATA_STUCK_HIGHT\n"); break;
			default: break;
		}

		// I'm Skipping the test for PS/2's second port
		return false;
	}

	LOG("First Port Passed the test\n");

	/* Enable Devices
	 *
	 * Enable any PS/2 port that exists and works using command 0xAE 
	 * (for the first port) and command 0xA8 (for the second port). 
	 * If you're using IRQs (recommended), also enable interrupts for any (usable) 
	 * PS/2 ports in the Controller Configuration Byte (set bit 0 for the first PS/2 
	 * port, and/or bit 1 for the second PS/2 port, then set it with command 0x60).
	 */

	ps2_send_command(COMMAND_FIRST_PS2_ENABLE);

	ps2_send_command(COMMAND_READ_CONTROLLER_CONFIG);
	config.whole = ps2_read_response();
	config.first_ps2_interrupt_enabled = 1;

	ps2_send_command_with_arg(COMMAND_WRITE_CONTROLLER_CONFIG, config.whole);

	/* Reset devices
	 *
	 * Once controller initialisation is done, all PS/2 devices (if any) 
	 * should be reset by the driver. This can be done by sending byte 0xFF 
	 * (reset command, supported by all PS/2 devices) to port 1 (and port 2 for 
	 * dual channel controllers, see "Sending Bytes to Device/s") and waiting 
	 * for a response. If the response is 0xFA, 0xAA (Note: the order in 
	 * which devices send these two seems to be ambiguous) followed by the 
	 * device PS/2 ID (see "Detecting PS/2 Device Types" for a list of IDs), 
	 * then the device reset was successful. If the response is 0xFC then the 
	 * self test failed and the device should be ignored. If theres no response 
	 * at all then the port is not populated.
	 */

	ps2_send_command_with_arg(COMMAND_WRITE_TO_FIRST_PS2_OUTPUT, 0xFF);

	resp = ps2_read_response();
	if (resp == 0xFA || resp == 0xAA) {
		resp = ps2_read_response();
		if (resp == 0xFA || resp == 0xAA) {
			LOG("Controller Self Test passed");
		}
	}

	return true;
}
