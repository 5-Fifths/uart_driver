#include "uart.h"
#include "ring_buffer.h"
#include "critical.h"

static ring_buffer_t TX = { 0 };
static ring_buffer_t RX = { 0 };
static uint32_t rx_overrun = 0;

static uart_registers_t *uart_registers = NULL;

void uart_init(uart_registers_t *registers, uint32_t baud_rate) {
    if (registers == NULL) return;

    // Initialization
    uart_registers = registers;
    uart_registers->BAUD = baud_rate;
    uart_registers->STATUS = 0;
    uart_registers->DATA = 0;
    
    // Enable interrupts
    uart_registers->CONTROL = UART_CONTROL_ENABLE;
}

bool uart_write_byte(uint8_t byte) {
    if (uart_registers == NULL) return false;

    enter_critical();
    bool success = push(&TX, byte);
    exit_critical();

    if (success) {
        enter_critical();
        uart_registers->CONTROL |= UART_CONTROL_TX_IE;
        exit_critical();
    }

    return success;
}

bool uart_read_byte(uint8_t *byte) {
    if (uart_registers == NULL || byte == NULL) return false;

    enter_critical();
    bool success = pop(&RX, byte);
    exit_critical();

    return success;
}

void uart_irq_handler(void) {
    // Register hasn't been initialized
    if (!uart_registers) return;

    // Checks if the RX status bit is set and reads from registers->DATA if it is
    if (uart_registers->STATUS & UART_STATUS_RX_READY) {
        uint8_t incoming_byte = (uint8_t) uart_registers->DATA;

        enter_critical();
        bool success = push(&RX, incoming_byte);
        exit_critical();

        if (!success) {
            enter_critical();
            rx_overrun++;
            exit_critical();
        }
    }

    // Checks if the TX status bit is set and writes to registers->DATA if it is 
    if (uart_registers->STATUS & UART_STATUS_TX_READY) {
        uint8_t outgoing_byte = 0;
        
        enter_critical();
        bool success = pop(&TX, &outgoing_byte);
        bool tx_empty = is_empty(&TX);
        exit_critical();

        if (success) {
            uart_registers->DATA = outgoing_byte;
        } 
        
        if (tx_empty) {
            uart_registers->CONTROL &= ~UART_CONTROL_TX_IE;
        }
    }
}

uint32_t get_overrun_count(void) {
    enter_critical();
    uint32_t res = rx_overrun;
    exit_critical();

    return res;
}

void reset_overrun_count(void) {
    enter_critical();
    rx_overrun = 0;
    exit_critical();
}