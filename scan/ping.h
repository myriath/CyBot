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
#include "Timer.h"
#include "driverlib/interrupt.h"
#include "lcd.h"

void ping_init();
void ping_trigger();
float ping_getDistance();

void TIMER3B_Handler();

#endif /* PING_H_ */
