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

// Converts degrees to radians.
double toRadians(double degrees);
// Converts radians to degrees.
double toDegrees(double radians);

// Converts raw ir values to cm.
double raw_to_dist(double ir_val);

// Initializes the scan functions.
void scan_init();
// Gets an averaged ir value over 6 scans.
double scan_ir(int degree);
// Gets an averaged ping value over 6 scans.
double scan_ping(int degree);
// Conducts a full 180 degree scan.
void scan_full(TallObject* objects);

#endif /* SCAN_H_ */
