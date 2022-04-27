/*
 * scan.h
 *
 *  Created on: Mar 19, 2022
 *      Author: wmhudson
 */

#ifndef SCAN_H_
#define SCAN_H_

#include <math.h>
#include "../data/structs.h"
#include "../data/data.h"
#include "../data/interrupts.h"

#include "adc.h"
#include "ping.h"
#include "scan.h"
#include "servo.h"
#include "../comms/uart.h"

double toRadians(double degrees);
double toDegrees(double radians);

double raw_to_dist(double ir_val);

void scan_init();
double scan_ir(int degree);
double scan_ping(int degree);
void scan_full(TallObject* objects);

#endif /* SCAN_H_ */
