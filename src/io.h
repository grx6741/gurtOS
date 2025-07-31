#pragma once

#include <stdint.h>

// Sends a byte 'value' to a hardware 'port'
void outb(uint16_t port, uint8_t value);

// Receives a byte from a hardware 'port'
uint8_t inb(uint16_t port);
