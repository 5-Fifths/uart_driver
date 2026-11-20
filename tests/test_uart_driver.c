#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "uart.h"

static int failures = 0;

// Check if state is as expected
static void check(bool condition, const char *message) {
    if (!condition) {
        printf("FAIL: %s\n", message);

        failures++;
    }
    else {
        printf("PASS: %s\n", message);
    }
}

// Reset variables after each test due to static variables maintaining state
static void clear(uart_registers_t *registers) {
    if (registers == NULL) return;

    // Clear RX
    uint8_t byte = 0;
    while (uart_read_byte(&byte)) continue;

    // Clear TX
    uint32_t previousControl = registers->CONTROL;
    uint32_t previousStatus = registers->STATUS;
    registers->STATUS |= UART_STATUS_TX_READY;
    registers->CONTROL |= UART_CONTROL_TX_IE;
    while (registers->CONTROL & UART_CONTROL_TX_IE) {
        uart_irq_handler();
    }
    registers->STATUS = previousStatus;
    registers->CONTROL = previousControl;

    // Clear rx_overrun
    reset_overrun_count();
}

// Test: Checking if RX operates in FIFO pattern
static void test_rx_fifo_order(void) {
    printf("\nTest: RX FIFO Order\n");
    printf("-------------------\n");

    uart_registers_t registers = { 0 };
    uart_init(&registers, 115200);

    uint8_t output = 0;
    check(!uart_read_byte(&output), "RX should start empty");

    // Simulate data entering the register from the outside
    // and then shift the data into the buffer
    registers.DATA = 'A';
    registers.STATUS |= UART_STATUS_RX_READY;
    uart_irq_handler();
    registers.STATUS &= ~UART_STATUS_RX_READY;

    registers.DATA = 'B';
    registers.STATUS |= UART_STATUS_RX_READY;
    uart_irq_handler();
    registers.STATUS &= ~UART_STATUS_RX_READY;

    check(uart_read_byte(&output) && output == 'A', "First RX read should be 'A'");
    check(uart_read_byte(&output) && output == 'B', "Second RX read should be 'B'");
    check(!uart_read_byte(&output), "RX should end empty");

    clear(&registers);
}

// Test: Ensure that TX only pushes to DATA when UART_STATUS_TX_READY is set
void test_tx_updates_data(void) {
    printf("\nTest: TX only updates DATA when UART_STATUS_TX_READY is set\n");
    printf("-------------------\n");

    uart_registers_t registers = { 0 };
    uart_init(&registers, 115200);

    // Initialize DATA register
    registers.DATA = 0;
    uart_write_byte('A');

    // Attempt to write while UART_STATUS_TX_READY is not set
    check((registers.STATUS & UART_STATUS_TX_READY) != UART_STATUS_TX_READY, "UART_STATUS_TX_READY starts as unset");
    check(registers.DATA != 'A', "DATA is not updated without UART_STATUS_TX_READY");

    // Attempt to write while UART_STATUS_TX_READY is set
    registers.STATUS |= UART_STATUS_TX_READY;
    uart_irq_handler();
    registers.STATUS &= ~UART_STATUS_TX_READY;
    check(registers.DATA == 'A', "DATA is updated once UART_STATUS_TX_READY has been set");

    // Check to make sure that TX was emptied successfully
    check((registers.CONTROL & UART_CONTROL_TX_IE) != UART_CONTROL_TX_IE, "UART_STATUS_TX_IE is unset after TX empties");

    clear(&registers);
}

// Test: Ensure that RX overflows are logged
void test_rx_overflow(void) {
    printf("\nTest: RX overflows are logged\n");
    printf("-------------------\n");

    uart_registers_t registers = { 0 };
    uart_init(&registers, 115200);

    // Fill up the buffer
    for (int i = 0; i < UART_BUFFER_SIZE; i++) {
        registers.DATA = 'A' + (i % 26);
        registers.STATUS |= UART_STATUS_RX_READY;
        uart_irq_handler();
        registers.STATUS &= ~UART_STATUS_RX_READY;
    }

    // Attempt to add another byte to the filled buffer
    registers.DATA = '1';
    registers.STATUS |= UART_STATUS_RX_READY;
    uart_irq_handler();
    registers.STATUS &= ~UART_STATUS_RX_READY;
    check(get_overrun_count() == 1, "Adding a byte to filled RX buffer triggers an overrun");

    clear(&registers);
}

int main(void) {
    test_rx_fifo_order();
    test_tx_updates_data();
    test_rx_overflow();

    return failures ? 1 : 0;
}