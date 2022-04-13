/*
 * servo.h
 *
 *  Created on: Apr 6, 2022
 *      Author: wmhudson
 */

#ifndef SERVO_H_
#define SERVO_H_

#include <stdint.h>
#include <inc/tm4c123gh6pm.h>
#include "Timer.h"
#include "driverlib/interrupt.h"
#include "lcd.h"

extern unsigned int left;
extern unsigned int right;

void servo_init();
void servo_move(int degrees);
void servo_setMatch(unsigned int match);

#endif /* SERVO_H_ */
