#include <stdbool.h>
#include <stdint.h>
#include "critical.h"

static volatile bool interrupt_enabled = true;
static volatile uint32_t nesting = 0;

void enter_critical() {
    if (nesting == 0) {
        interrupt_enabled = false;
    }
    nesting++;
}

void exit_critical() {
    if (nesting > 0) {
        nesting--;

        if (nesting == 0) {
            interrupt_enabled = true;
        }
    }
}