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
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "../premade/Timer.h"
#include "driverlib/interrupt.h"
#include "../premade/lcd.h"

extern unsigned int left;
extern unsigned int right;

void servo_init();
void servo_move(int degrees);
void servo_setMatch(int match);

#endif /* SERVO_H_ */
