/*
 * servo.c
 *
 *  Created on: Apr 6, 2022
 *      Author: wmhudson
 */

#include "servo.h"

float servoMillisPerDegree = 500.0/180.0; // TODO: arbitrary temp value, TEST THIS

uint8_t initialized = 0;

unsigned int right = 0x4a380;
unsigned int left = 0x46500;
float degreeCount;

int currentDegree = -1;

void servo_init() {
    //Check if already initialized
    if(initialized){
        return;
    }

    SYSCTL_RCGCGPIO_R |= 0x2;
    while (!(SYSCTL_PRGPIO_R & 0x2));
    SYSCTL_RCGCTIMER_R |= 0x2;
    while (!(SYSCTL_PRTIMER_R & 0x2));

    GPIO_PORTB_DIR_R |= 0x20;
    GPIO_PORTB_AFSEL_R |= 0x20;
    GPIO_PORTB_DEN_R |= 0x20;
    GPIO_PORTB_PCTL_R |= 0x700000;

    TIMER1_CTL_R &= ~0x100;
    TIMER1_CFG_R |= 0x4;
    TIMER1_TBILR_R = 0xe200;
    TIMER1_TBPR_R = 0x4;
    TIMER1_TBMATCHR_R = 0xa380;
    TIMER1_TBPMR_R = 0x4;
    TIMER1_TBMR_R |= 0xa;

    TIMER1_CTL_R |= 0x100;

    degreeCount = (right - left) / 180.0f;
    servo_move(0); // initial move
    initialized = true;
}

void servo_setMatch(int match) {
    TIMER1_TBMATCHR_R = match & 0xffff;
    TIMER1_TBPMR_R = (unsigned long int)(match & 0xff0000) >> 16;
}

void servo_move(int degrees) {
    if (degrees == currentDegree) return;
    unsigned long int match = right - round(degrees * degreeCount);
    servo_setMatch(match);

    int dist = abs(currentDegree - degrees);
    currentDegree = degrees;
    timer_waitMillis(servoMillisPerDegree * dist + 100);
}
