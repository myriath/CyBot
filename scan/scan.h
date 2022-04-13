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

#include "adc.h"
#include "ping.h"
#include "scan.h"
#include "servo.h"

double toRadians(double degrees);
double toDegrees(double radians);

Scan getTrueScan(Scan scan, double dist, double degrees);

void scan_init();
void scan(TallObject* objects, int start, int end);

#endif /* SCAN_H_ */
