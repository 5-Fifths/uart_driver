#include "ring_buffer.h"
#include "uart.h"

bool is_empty(const ring_buffer_t *buffer) {
    if (buffer == NULL) return true;

    return buffer->size == 0;
}

bool is_full(const ring_buffer_t *buffer) {
    if (buffer == NULL) return false;

    return buffer->size == UART_BUFFER_SIZE;
}

bool push(ring_buffer_t *buffer, const uint8_t byte) {
    if (buffer == NULL) return false;

    if (!is_full(buffer)) {
        buffer->array[buffer->head] = byte;
        buffer->size = buffer->size + 1;
        buffer->head = (buffer->head + 1) % UART_BUFFER_SIZE;

        return true;
    }

    return false;
}

bool pop(ring_buffer_t *buffer, uint8_t *byte) {
    if (buffer == NULL || byte == NULL) return false;

    if (!is_empty(buffer)) {
        *byte = buffer->array[buffer->tail];
        buffer->size = buffer->size - 1;
        buffer->tail = (buffer->tail + 1) % UART_BUFFER_SIZE;

        return true;
    }

    return false;
}