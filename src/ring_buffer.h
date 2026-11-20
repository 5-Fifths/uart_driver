#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include "uart.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t size;
    uint32_t head;
    uint32_t tail;
    uint8_t array[UART_BUFFER_SIZE];
} ring_buffer_t;

// Check if the buffer is empty
    // returns true if the array has no usable data
    // returns false if the array has data
bool is_empty(const ring_buffer_t *buffer);

// Check if the buffer is full
    // returns true if the array cannot take anymore data
    // returns false if the array has space
bool is_full(const ring_buffer_t *buffer);

// Adds a byte to the buffer, if there is space
    // returns true if there is space and the byte was queued
    // returns false if there was no space in the array
bool push(ring_buffer_t *buffer, uint8_t byte);

// Remove a byte from the buffer
    // returns true if there was data and a byte was dequeued
    // returns false if there was no data to be read
bool pop(ring_buffer_t *buffer, uint8_t *byte);

#endif