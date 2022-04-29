/*
 * servo.c
 *
 *  Created on: Apr 6, 2022
 *      Author: wmhudson
 */

#include "servo.h"

// Milliseconds to wait per servo degrees.
float servoMillisPerDegree = 500.0/180.0;

// 1 when initialized, 0 when not.
uint8_t initialized = 0;

// Default left and right timer values, gets reset in main.
unsigned int right = 0x4a380;
unsigned int left = 0x46500;
// Timer value per degree of servo movement.
float degreeCount;
// Keeps track of current position.
int currentDegree = -1;

/**
 * Initializes the servo and gpio
 */
void servo_init() {
    //Check if already initialized
    if(initialized){
        return;
    }

    // Enables clock to the gpio pins and timer.
    SYSCTL_RCGCGPIO_R |= 0x2;
    while (!(SYSCTL_PRGPIO_R & 0x2));
    SYSCTL_RCGCTIMER_R |= 0x2;
    while (!(SYSCTL_PRTIMER_R & 0x2));

    // Sets direction for the gpio pins
    GPIO_PORTB_DIR_R |= 0x20;
    // Enables alternate function for the gpio.
    GPIO_PORTB_AFSEL_R |= 0x20;
    // Turns on digital functions for the gpio.
    GPIO_PORTB_DEN_R |= 0x20;
    // Sets the alternate function to timer.
    GPIO_PORTB_PCTL_R |= 0x700000;

    // Ensures timer is off for config.
    TIMER1_CTL_R &= ~0x100;
    // Timer configuration
    TIMER1_CFG_R |= 0x4;
    // Reset value for the timer.
    TIMER1_TBILR_R = 0xe200;
    TIMER1_TBPR_R = 0x4;
    // Match value for the timer.
    TIMER1_TBMATCHR_R = 0xa380;
    TIMER1_TBPMR_R = 0x4;
    // Sets the timer mode.
    TIMER1_TBMR_R |= 0xa;
    // Enables the timer after config.
    TIMER1_CTL_R |= 0x100;

    // Sets our degree count.
    degreeCount = (right - left) / 180.0f;
    servo_move(0); // initial move
    // Sets initialized.
    initialized = true;
}

/**
 * Sets a new 24 bit match value for the timer.
 */
void servo_setMatch(int match) {
    // Gets the bottom 16 bits of the 24 bit new match value.
    TIMER1_TBMATCHR_R = match & 0xffff;
    // Gets the top 8 bits of the 24 bit new match value.
    TIMER1_TBPMR_R = (unsigned long int)(match & 0xff0000) >> 16;
}

/**
 * Moves the servo to a given angle. Waits time based on how far it travels.
 */
void servo_move(int degrees) {
    // Checks that we actually need to move.
    if (degrees == currentDegree) return;
    // Calculates the new match value for the servo.
    unsigned long int match = right - round(degrees * degreeCount);
    // Sets the new match value (moves the servo).
    servo_setMatch(match);

    // Calculates the distance in degrees traveled.
    int dist = abs(currentDegree - degrees);
    // Sets current degree.
    currentDegree = degrees;
    // Calculates wait time for how far the servo traveled.
    timer_waitMillis(servoMillisPerDegree * dist + 100);
}
