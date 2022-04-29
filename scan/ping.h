/*
 * ping.h
 *
 *  Created on: Mar 30, 2022
 *      Author: wmhudson
 */

#ifndef PING_H_
#define PING_H_

#include <stdint.h>
#include <inc/tm4c123gh6pm.h>
#include "../premade/Timer.h"
#include "driverlib/interrupt.h"
#include "../premade/lcd.h"

// Initializes the ping sensor.
void ping_init();
// Uses the ping sensor to get the distance of the current position.
float ping_getDistance();

// Interrupt handler for the timer.
void TIMER3B_Handler();

#endif /* PING_H_ */
