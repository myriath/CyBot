/*
 * scan.c
 *
 *  Created on: Mar 19, 2022
 *      Author: wmhudson
 */

#ifndef PI
#define PI 3.1415926535
#endif

#include "scan.h"

double raw_to_dist(double ir_val) {
    if (ir_val <= 118.22901) return -1;
    return 28541.58384 / (ir_val - 118.22901);
}

double toRadians(double degrees) {
    return degrees * PI / 180;
}

double toDegrees(double radians) {
    return radians * 180 / PI;
}

Scan getTrueScan(Scan scan, double dist, double degrees) {
    double x = dist * cos(toRadians(degrees));
    double y = dist * sin(toRadians(degrees)) + 18.75;
    scan.distance = sqrt(x * x + y * y);
    scan.angle = toDegrees(atan(y / x));

    return scan;
}

void scan_init() {
    adc_init();
    ping_init();
    servo_init();
}

// TODO
void scan(TallObject* objects, int start, int end) {
    servo_move(start);
    int IR_ITER = 3;
    int i;
    int j;
    for (i = start; i < end; i += 2) {
        servo_move(i);
        double ir = 0;
        for (int j = 0; j < IR_ITER; j++) {
            ir += adc_read();
        }
        ir = raw_to_dist(ir / IR_ITER);

        bool finding_obj = ir > 10 && ir < 50;
    }

    return getTrueScan(scan, , degrees)
}
