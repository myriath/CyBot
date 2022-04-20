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

void getTrueScan(Scan scan, double dist, double degrees) {
    double x = dist * cos(toRadians(degrees));
    double y = dist * sin(toRadians(degrees)) + 18.75;
    scan.distance = sqrt(x * x + y * y);
    scan.angle = toDegrees(atan(y / x));
}

void scan_init() {
    adc_init();
    ping_init();
    servo_init();
}

double scan_ir(int degree) {
    servo_move(degree);
    int j;
    double ir = 0;
    for (j = 0; j < 3; j++) {
        ir += adc_read();
    }
    return raw_to_dist(ir / 3);
}

double scan_ping(int degree) {
    servo_move(degree);
    int j;
    double ping = 0;
    for (j = 0; j < 3; j++) {
        ping += ping_getDistance();
        timer_waitMillis(10);
    }
    return ping / 3;
}

void scan_test() {
    uart_log();
    uart_sendStr("IR val");
    uart_end();

    uart_log();
    uart_sendFloat(scan_ir(90));
    uart_end();
}

void scan_buildObject(int obj_degree_end, int obj_degree_start, TallObject* objects) {
    TallObject obj;
    obj.obj_num = object_num++;
    obj.radial_width = obj_degree_end - obj_degree_start;
    obj.angle = obj_degree_start + (obj.radial_width / 2);

    if (obj.radial_width > 2 && object_num <= MAX_OBJECTS) {
        objects[obj.obj_num] = obj;
        uart_log();
        uart_sendStr("end obj");
    } else {
        uart_log();
        uart_sendStr("ignored");
    }
    uart_end();
}

void scan_full(TallObject* objects) {
    bool found_obj = false;
    int obj_degree_start;
    servo_move(0);

    uart_log();
    uart_sendStr("Degrees\t\tIR val");
    uart_end();

    int i;
    for (i = 0; i <= 180; i += 2) {
        if (interrupt_stopScan || interrupt_emergency) return;

        double ir_val = scan_ir(i);

        uart_log();
        uart_sendInt(i);
        uart_sendStr("\t\t");
        uart_sendInt(ir_val);
        uart_end();

        bool finding_obj = ir_val > 10 && ir_val < 70;

        if (finding_obj) {
            uart_scan();
            uart_sendInt(i);
            uart_sendStr(",");
            uart_sendFloat(ir_val);
            uart_end();
        }

        if (found_obj != finding_obj && finding_obj == true) {
            obj_degree_start = i;
            uart_log();
            uart_sendStr("start obj");
            uart_end();
        } else if (found_obj != finding_obj && finding_obj == false) {
            scan_buildObject(i, obj_degree_start, objects);
        }
        found_obj = finding_obj;
    }
    if (found_obj) {
        scan_buildObject(180, obj_degree_start, objects);
    }

    for (i = 0; i < object_num; i++) {
        if (interrupt_stopScan || interrupt_emergency) return;
        objects[i].dist = scan_ping(objects[i].angle);
        lcd_printf("%d", i);

        double radians = (objects[i].radial_width * PI) / 360.0;
        objects[i].linear_width = 2 * objects[i].dist * tan(radians);
    }

    uart_log();
    uart_sendStr("Obj#\tAngle\tDist\tRWidth\tLWidth");
    uart_end();
    for (i = 0; i < object_num; i++) {
        if (interrupt_stopScan || interrupt_emergency) return;
        if (objects[i].dist < 50 + objects[i].linear_width) {
            uart_log();
            uart_sendInt(objects[i].obj_num); // send object data to screen
            uart_sendChar('\t');
            uart_sendInt(objects[i].angle);
            uart_sendChar('\t');
            uart_sendFloat(objects[i].dist);
            uart_sendChar('\t');
            uart_sendInt(objects[i].radial_width);
            uart_sendChar('\t');
            uart_sendFloat(objects[i].linear_width);
            uart_end();

            uart_object();
            uart_sendInt(objects[i].angle);
            uart_sendChar(',');
            uart_sendFloat(objects[i].dist);
            uart_sendChar(',');
            uart_sendFloat(objects[i].linear_width);
            uart_end();
        }
    }

    object_num = 0;
}
