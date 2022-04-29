/*
 * ping.c
 *
 *  Created on: Mar 30, 2022
 *      Author: wmhudson
 */

#include "ping.h"

// 0 if not initialized, 1 if initialized.
static uint8_t initialized = 0;

// Enumeration of the ping states. Low means setting start time, High means waiting for end time, Done means has completed timer ready to be read.
volatile enum{LOW, HIGH, DONE} STATE = LOW;
// Start and end times used to calculate the distance.
float startTime = 0;
float endTime = 0;

/**
 * Interrupt handler for timer 3b.
 */
void TIMER3B_Handler() {
    // Only do something if the interrupt is received.
    if (TIMER3_MIS_R & 0x400) {
        // Clear the interrupt register.
        TIMER3_ICR_R |= 0x400;
        // Check the current state and act accordingly.
        if (STATE == LOW) {
            // Set start time and state to High
            startTime = TIMER3_TBR_R;
            STATE = HIGH;
        } else if (STATE == HIGH) {
            // Set end time and state to Done.
            endTime = TIMER3_TBR_R;
            STATE = DONE;
        }
    }
}

/**
 * Initialize the ping sensor
 */
void ping_init() {
    //Check if already initialized
    if(initialized){
        return;
    }

    // Enables clock to the timer and gpio.
    SYSCTL_RCGCGPIO_R |= 0x2;
    while (!(SYSCTL_PRGPIO_R & 0x2)) {}
    SYSCTL_RCGCTIMER_R |= 0x8;
    while (!(SYSCTL_PRTIMER_R & 0x8)) {}

    // Set the direction for the gpio pins.
    GPIO_PORTB_DIR_R &= ~0x8;
    // Enable alternate functions on the pins.
    GPIO_PORTB_AFSEL_R |= 0x8;
    // Enable digital functions on the pins.
    GPIO_PORTB_DEN_R |= 0x8;
    // Set alternate function to timer3.
    GPIO_PORTB_PCTL_R |= 0x7000;

    // Disable timer3 for config.
    TIMER3_CTL_R &= ~0x100;
    // Configure timer 3
    TIMER3_CFG_R |= 0x4;
    // Set mode for timer 3b
    TIMER3_TBMR_R |= 0x7;
    // Set control for timer 3
    TIMER3_CTL_R |= 0xc00;
    // Set reset values for the timer.
    TIMER3_TBPR_R |= 0xff;
    TIMER3_TBILR_R |= 0xffff;
    // Set Interrupt registers for the timer.
    TIMER3_IMR_R |= 0x400;
    TIMER3_ICR_R |= 0x400;
    // Enable interrupts for the timer in the nvic and set priority.
    NVIC_EN1_R |= 0x10;
    NVIC_PRI9_R |= 0x20;
    // Enable the interrupt in the vector table.
    IntRegister(INT_TIMER3B, TIMER3B_Handler);
    IntMasterEnable();
    // Turn on timer 3
    TIMER3_CTL_R |= 0x100;

    // Set initialized.
    initialized = 1;
}

/**
 * Trigger a new ping scan.
 */
void ping_trigger() {
    // Set state to low
    STATE = LOW;
    // Turn off the timer and disable interrupts.
    TIMER3_IMR_R &= ~0x400;
    TIMER3_CTL_R &= ~0x100;
    // Set gpio direction to output and disable alternate function
    GPIO_PORTB_DIR_R |= 0x8;
    GPIO_PORTB_AFSEL_R &= ~0x8;

    // Send a high signal for 5 microseconds, then set back to low.
    GPIO_PORTB_DATA_R &= ~0x8;
    GPIO_PORTB_DATA_R |= 0x8; //high
    timer_waitMicros(5);
    GPIO_PORTB_DATA_R &= ~0x8; //low

    // Reset gpio direction to input and enable alternate function.
    GPIO_PORTB_DIR_R &= ~0x8;
    GPIO_PORTB_AFSEL_R |= 0x8;
    // Re-enable interrupts and timer3.
    TIMER3_IMR_R |= 0x400;
    TIMER3_CTL_R |= 0x100;
}

/**
 * Full ping command to start a ping, wait until completion, and report distance calculated.
 */
float ping_getDistance() {
    // Trigger a ping.
    ping_trigger();
    // Wait until completed.
    while (STATE != DONE);
    // Calculate distance using formula based on speed of sound.
    return (startTime - endTime) / 32000000 * 34300;
}
