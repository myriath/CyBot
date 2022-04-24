/*
*
*   uart.c
*
*
*
*   @author
*   @date
*/

#include <inc/tm4c123gh6pm.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>

#include "interrupts.h"
#include "uart.h"
#include "data.h"
#include "driverlib/interrupt.h"

volatile char command_byte = 0;
bool sending = false;
bool scanning = false;

void UART1_Handler(void) {
    char byte_received;
    //check if handler called due to RX event
    if (UART1_MIS_R & 0x10)
    {
        //byte was received in the UART data register
        //clear the RX trigger flag (clear by writing 1 to ICR)
        UART1_ICR_R |= 0b00010000;

        //read the byte received from UART1_DR_R and echo it back to PuTTY
        //ignore the error bits in UART1_DR_R
        command_byte = UART1_DR_R & 0xff;

        if (command_byte == B_EMERGENCY_STOP) {
            interrupt_emergency = true;
            command_byte = 0;
        }
        else if (command_byte == B_SCAN && scanning) {
            interrupt_stopScan = true;
            command_byte = 0;
        }
    }
}

void uart_init_interrupts() {
    //first clear RX interrupt flag (clear by writing 1 to ICR)
    UART1_ICR_R |= 0b00010000;

    //enable RX raw interrupts in interrupt mask register
    UART1_IM_R |= 0x10;

    //NVIC setup: set priority of UART1 interrupt to 1 in bits 21-23
    NVIC_PRI1_R = (NVIC_PRI1_R & 0xFF0FFFFF) | 0x00200000;

    //NVIC setup: enable interrupt for UART1, IRQ #6, set bit 6
    NVIC_EN0_R |= 0x40;

    //tell CPU to use ISR handler for UART1 (see interrupt.h file)
    //from system header file: #define INT_UART1 22
    IntRegister(INT_UART1, UART1_Handler);

    //globally allow CPU to service interrupts (see interrupt.h file)
    IntMasterEnable();
}

void uart_enable() {
    SYSCTL_RCGCGPIO_R |= 0x2;

    SYSCTL_RCGCUART_R |= 0x2;

    while ((SYSCTL_PRGPIO_R & 0x2) == 0);
    while ((SYSCTL_PRUART_R & 0x2) == 0);

    GPIO_PORTB_AFSEL_R |= 0x3;

    GPIO_PORTB_DEN_R |= 0x3;

    GPIO_PORTB_PCTL_R = 0x11;

    uint16_t iBRD = 16000000 / (16 * 115200); //use equations
    uint16_t fBRD = (int)(0.6808 * 64 + 0.5); //use equations

    UART1_CTL_R &= ~0x0000cbbf;

    UART1_IBRD_R = iBRD;
    UART1_FBRD_R = fBRD;

    UART1_LCRH_R = 0x60;

    UART1_CC_R = 0x0;
}

void uart_init(void){
    uart_enable();
    uart_init_interrupts();

    //re-enable UART1 and also enable RX, TX (three bits)
    //note from the datasheet UARTCTL register description:
    //RX and TX are enabled by default on reset
    //Good to be explicit in your code
    //Be careful to not clear RX and TX enable bits
    //(either preserve if already set or set them)
    UART1_CTL_R |= 0x301;
}

void uart_sendByte(uint8_t data) {
    while (UART1_FR_R & 0x20) {}

    UART1_DR_R = data;
}

void uart_sendData(uint8_t* ptr, size_t len) {
    int i;
    for (i = 0; i < len; i++) {
        uart_sendByte(*(ptr + i));
    }
}

void uart_sendChar(char data){
	while (UART1_FR_R & 0x20) {}

	UART1_DR_R = data;
}

char uart_receive(void){
	while (UART1_FR_R & UART_FR_RXFE) {}
	return (char)(UART1_DR_R & 0xff);
}

char uart_receive_no_wait(void){
    if (!(UART1_FR_R & UART_FR_RXFE)) {
        return (char)(UART1_DR_R & 0xff);
    }
    return 0;
}


void uart_sendInt(int data) {
    uart_sendData((uint8_t *) &data, sizeof(data));
//    if (data < 0) {
//        uart_sendChar('-');
//        data = -data;
//    }
//    int arr[INT_ARR_DIGITS];
//    int i = 0;
//    while (i < INT_ARR_DIGITS - 1) {                    // build array of digits backwards
//                                                        // 102 becomes {2, 0, 1, ?, ?, ..., ?}
//        arr[i] = data % 10;
//        data /= 10;
//        i++;
//        if (!data) {
//            break;
//        }
//    }
//    while (i > 0) {
//        i--;
//        uart_sendChar(arr[i] + 48);                    //read array in reverse and add 48 to get ascii value of digit (ascii 48 is 0, 49 is 1, etc)
//    }
}

void uart_sendFloat(float data) {
    uart_sendData((uint8_t *) &data, sizeof(data));

//    if (data < 0) {
//        uart_sendChar('-');
//        data = -data;
//    }
//    uart_sendInt((int) data);                                // send whole part of float
//    uart_sendChar('.');                                      // send decimal point
//    int multiplier = pow(10, SEND_FLOAT_DECIMAL_PLACES);
//    uart_sendInt((int)(data * multiplier) % multiplier);     // send decimal places
}

void uart_sendVarInt(int data) {
    while (true) {
        if ((data & ~0x7f) == 0) {
            uart_sendByte(data & 0xff);
            return;
        }
        uart_sendByte((data & 0x7f) | 0x80);

        data = data >> 7;
    }
}

void uart_sendStr(const char *string){
    int len = 0;
    while (string[len] != '\0') {
        len++;
    }
    uart_sendVarInt(len);
    while (string[0] != '\0'){
        if (string[0] == '\\') {
            if (string[1] == 't') {
                uart_sendChar('\t');
            }
            string += 2;
            continue;
        }

        uart_sendChar(string[0]);
        string++;
    }
}

void uart_log(const char* str) {
//    uart_sendStr("LOG:");
//    sending = true;
    uart_sendByte(B_LOG);
    uart_sendStr(str);
}

void uart_scan(int degree, double ir) {
//    uart_sendStr("SCN:");
//    sending = true;
    uart_sendByte(B_SCAN_DATA);
    uart_sendVarInt(degree);
    uart_sendData((uint8_t*) &ir, sizeof(ir));

}

void uart_object(TallObject obj) {
//    uart_sendStr("OBJ:");
//    sending = true;
    uart_sendByte(B_OBJ);
    uart_sendVarInt(obj.angle);
    uart_sendData((uint8_t*) &obj.dist, sizeof(obj.dist));
    uart_sendData((uint8_t*) &obj.linear_width, sizeof(obj.linear_width));
    lcd_printf("%d %0.2f\n%0.2f", obj.angle, obj.dist, obj.linear_width);
}

void uart_move(double dist, double angle) {
//    uart_sendStr("MOV:");
//    sending = true;
    uart_sendByte(B_MOVE);
    uart_sendData((uint8_t *) &angle, sizeof(angle));
    uart_sendData((uint8_t *) &dist, sizeof(dist));
}

void uart_logEdge(const char* str, int val) {
    uart_sendByte(B_LOG_EDGE);
    uart_sendStr(str);
    uart_sendVarInt(val);
}

void uart_end() {
    uart_sendChar(B_NEWLINE);
    sending = false;
}

void uart_stopWait() {
    uart_sendChar(B_WAIT);
    sending = false;
}
