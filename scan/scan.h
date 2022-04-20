/*
 * scan.h
 *
 *  Created on: Mar 19, 2022
 *      Author: wmhudson
 */

#ifndef SCAN_H_
#define SCAN_H_

#include <math.h>
#include "structs.h"
#include "../interrupts.h"

#include "adc.h"
#include "ping.h"
#include "scan.h"
#include "servo.h"
#include "../uart.h"
#include "../structs.h"
#include "../data.h"

double toRadians(double degrees);
double toDegrees(double radians);

double raw_to_dist(double ir_val);

void getTrueScan(Scan scan, double dist, double degrees);

void scan_init();
void scan_test();
double scan_ir(int degree);
double scan_ping(int degree);
void scan_full(TallObject* objects);

#endif /* SCAN_H_ */
