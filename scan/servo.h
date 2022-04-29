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

// Left and right match values to be calibrated in main.
extern unsigned int left;
extern unsigned int right;

// Initializes the servo.
void servo_init();
// Moves the servo to the given degree (and waits until finished).
void servo_move(int degrees);
// Sets the match value (used only for calibration outside of servo.c).
void servo_setMatch(int match);

#endif /* SERVO_H_ */
