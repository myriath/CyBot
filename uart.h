/*
*
*   uart.h
*
*   Used to set up the RS232 connector and WIFI module
*   Functions for communicating between CyBot and PC via UART1
*   Serial parameters: Baud = 115200, 8 data bits, 1 stop bit,
*   no parity, no flow control on COM1, FIFOs disabled on UART1
*
*   @author Dane Larson
*   @date 07/18/2016
*   Phillip Jones updated 9/2019, removed WiFi.h, Timer.h
*   Diane Rover updated 2/2020, changed receive return type to char
*/

#ifndef UART_H_
#define UART_H_

#include <inc/tm4c123gh6pm.h>
#include <stdint.h>
#include <stdbool.h>
#include "driverlib/interrupt.h"

// The following externals are global variables defined in uart-interrupt.c for use with the interrupt handler.
// Using extern here, the global variables become visible to other c files that include uart-interrupt.h
// Extern does not allocate storage for a variable. It tells the compiler that the variable is defined in another file.
//extern volatile char receive_buffer[]; // buffer for characters received from PuTTY
//extern volatile int receive_index; // index to keep track of characters in buffer
extern volatile char command_byte; // byte value for special character used as a command
extern volatile int command_flag; // flag to tell the main program a special command was received

// UART1 device initialization for CyBot to PuTTY
void uart_init(void);

// Send a byte over UART1 from CyBot to PuTTY
void uart_sendChar(char data);

// CyBot waits (i.e. blocks) to receive a byte from PuTTY
// returns byte that was received by UART1
char uart_receive(void);

char uart_receive_no_wait(void);

void uart_sendInt(int data);

void uart_sendFloat(float data);

// Send a string over UART1
// Sends each char in the string one at a time
void uart_sendStr(const char *data);

void uart_log();
void uart_scan();
void uart_object();
void uart_move();
void uart_end();
void uart_stopWait();

#endif /* UART_H_ */
