#ifndef CRITICAL_H
#define CRITICAL_H

// Enters a critical state to prevent concurrent modifications
void enter_critical();

// Exits the state to allow interrupts to happen again
void exit_critical();

#endif