# UART Driver
### Register Map
The registers are by 4 32-bit blocks (DATA, STATUS, CONTROL, BAUD) in the uart_registers_t struct, which represents a register block. They are accessed using uart_registers_t *registers. There is a read buffer RX and a transmit buffer TX, which are fixed-size ring buffers.

#### DATA Register
DATA stores the byte that is either being read into RX or transmitted from TX. If a byte in DATA has not yet fully been pushed to RX before another byte has come into DATA, UART_STATUS_RX_OVERFLOW is set in the STATUS register to indicate that this has occurred and that some data was lost.

#### STATUS Register
STATUS stores peripheral state, like if there is a byte read to be read or written. UART_STATUS_RX_READY and UART_STATUS_TX_READY each indicate if there is a byte ready to be read into RX from DATA and if there is a byte in TX ready to be written to DATA. UART_STATUS_RX_OVERFLOW can also be set on this register to indicate that there was an overflow on the DATA register, where a byte was not finished being pushed into RX before another byte overwrote it. 

#### CONTROL Register
CONTROL stores whether the driver is enabled, as well as if interrupts are allowed to trigger a read/write. There are 3 bits set for the registers: UART_CONTROL_TX_IE, UART_CONTROL_RX_IE, and UART_CONTROL_ENABLE. It's initialized to set the UART_CONTROL_ENABLE and UART_CONTROL_RX_IE bit on initialization so that we know that data can start being read. UART_CONTROL_TX_IE is set whenever uart_write_byte is called, indicating that an interrupt is allowed to trigger and upload to DATA.

#### BAUD Register
BAUD stores the baud rate. 115200 is the baud rate selected in testing because it's one of the standard rates for UART drivers. It has no bearing on any of the other logic because there is no hardware to communicate with to set a specific baud rate. 

---

### Buffer Design
The TX and RX buffers are fixed-size ring buffers, implemented using an array. The head variable tracks where the array is going to be pushed to and tail variable tracks where data is popped from. The size variable tracks how many elements are in the array. The total size of the array is decided by UART_BUFFER_SIZE, found in the uart.h file. To ensure that these variables stays in sync, every update using push/pop in the uart read/write functions is wrapped in a critical section.

---

### Atomicity
Using volatile by itself doesn't enforce atomicity, so it is mocked using critical.h. When in a critical section (sections wrapped in enter_critical() and exit_critical()), an interrupted_enabled flag is set to false and the nesting level is incremented to mock the function of the critical section. Interrupts are not allowed to access or modify the data wrapped in the section, thus allowing any changes to be made safely and reflected across all things accessing the shared resource in question. Again, this is currently mocked and not actually effective in the current code.

---

### Build Command
`gcc -Wall -Wextra -std=c11 -Isrc src/ring_buffer.c src/uart.c src/critical.c tests/test_uart_driver.c -o build/test_uart_driver`

---

### Test Results
##### Test: RX FIFO Order
PASS: RX should start empty\
PASS: First RX read should be 'A'\
PASS: Second RX read should be 'B'\
PASS: RX should end empty

##### Test: TX only updates DATA when UART_STATUS_TX_READY is set
PASS: UART_STATUS_TX_READY starts as unset\
PASS: DATA is not updated without UART_STATUS_TX_READY\
PASS: DATA is updated once UART_STATUS_TX_READY has been set\
PASS: UART_CONTROL_TX_IE is unset after TX empties

##### Test: RX buffer overruns are tracked
PASS: No overrun if the buffer is merely full\
PASS: Adding a byte to filled RX buffer triggers an overrun

##### Test: Hardware RX overflows are tracked
PASS: DATA overwritten before it was shifted to RX\
PASS: RX overflow logged\
PASS: UART_STATUS_RX_OVERFLOW was unset\
PASS: New byte was serviced instead of overwritten byte
