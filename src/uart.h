#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stdbool.h>

// CONSTANT CONFIGURATION
#define UART_BUFFER_SIZE 64

// BIT MASKS
// STATUS Register Bits
#define UART_STATUS_TX_READY    (1U << 0)
#define UART_STATUS_RX_READY    (1U << 1)
#define UART_STATUS_RX_OVERFLOW (1U << 2)

// CONTROL Register Bits
#define UART_CONTROL_TX_IE  (1U << 0)
#define UART_CONTROL_RX_IE  (1U << 1)
#define UART_CONTROL_ENABLE (1U << 2)

typedef struct {
    volatile uint32_t STATUS;
    volatile uint32_t DATA;
    volatile uint32_t CONTROL;
    volatile uint32_t BAUD;
} uart_registers_t;

// Initializes the driver, buffer, and register pointer
void uart_init(uart_registers_t *registers, uint32_t baud_rate);

// Write a byte to the buffer
    // returns true if byte is successfully queued
    // returns false if there isn't enough space in the buffer or the register is busy
bool uart_write_byte(uint8_t byte);

// Read a byte from the buffer
    // returns true if byte is successfully dequeued
    // returns false if the buffer is empty or the register is busy
bool uart_read_byte(uint8_t *byte);

// Interrupt handler
void uart_irq_handler(void);

// Gets the number of times the buffer has overfilled
uint32_t get_overrun_count(void);

// Reset the overrun count
void reset_overrun_count(void);

// Gets the number of times that DATA was overwritten before it was shifted into RX
uint32_t get_hardware_rx_overflow_count(void);

// Reset the RX overflow count
void reset_hardware_rx_overflow_count(void);
#endif