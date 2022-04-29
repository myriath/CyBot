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

// Converts raw ir values to the distance based on calibration.
double raw_to_dist(double ir_val) {
    if (ir_val <= 118.22901) return -1;
    return 28541.58384 / (ir_val - 118.22901);
}

/**
 * Converts degrees to radians.
 */
double toRadians(double degrees) {
    return degrees * PI / 180;
}

/**
 * Converts radians to degrees
 */
double toDegrees(double radians) {
    return radians * 180 / PI;
}

/**
 * Initializes all components of the scan functions.
 */
void scan_init() {
    // Initializes ADC
    adc_init();
    // Initializes Ping sensor
    ping_init();
    // Initializes servo
    servo_init();
}

/**
 * Scans the ir value for the given degree.
 */
double scan_ir(int degree) {
    // Moves the servo
    servo_move(degree);
    int j;
    double ir = 0;
    for (j = 0; j < 6; j++) {
        // Scans with the ir sensor 6 times.
        ir += adc_read();
    }
    // Gets average ir value then converts it to distance.
    return raw_to_dist(ir / 6);
}

/**
 * Scans the ping value for the given degree.
 */
double scan_ping(int degree) {
    // Moves the servo
    servo_move(degree);
    int j;
    double ping = 0;
    for (j = 0; j < 6; j++) {
        // Pings 6 times.
        ping += ping_getDistance();
        timer_waitMillis(10);
    }
    // Returns the average ping distance.
    return ping / 6;
}

/**
 * Builds an object given the required data points and puts it into the object array.
 */
void scan_buildObject(int obj_degree_end, int obj_degree_start, TallObject* objects) {
    // Puts data into an object structure.
    TallObject obj;
    obj.obj_num = object_num++;
    obj.radial_width = obj_degree_end - obj_degree_start;
    obj.angle = obj_degree_start + (obj.radial_width / 2);

    // Only adds if the width is > 2 and there is space available in the array.
    if (obj.radial_width > 2 && object_num <= MAX_OBJECTS) {
        objects[obj.obj_num] = obj;
        uart_log("end obj");
    } else {
        uart_log("ignored");
    }
}

/**
 * Conducts a full 180 degree scan and builds objects when it sees them.
 */
void scan_full(TallObject* objects) {
    // Resets the scan view on the app.
    uart_sendChar(B_SCAN_RESET);
    // Default boolean for when finding an object.
    bool found_obj = false;
    // Keeps track of the starting degree of an object.
    int obj_degree_start;
    // Moves to 0.
    servo_move(0);

    // Log the label for data to come.
    uart_log("Degrees\t\tIR val");

    int i;
    for (i = 0; i <= 180; i += 2) {
        // Stop the scan if an interrupt is received.
        if (interrupt_stopScan || interrupt_emergency) return;

        // Gets the ir distance.
        double ir_val = scan_ir(i);
        // Send scan data to the user.
        uart_scan(i, ir_val);
        // If the ir distance is within 10 and 70 cm, we are looking at an object.
        bool finding_obj = ir_val > 10 && ir_val < 70;

        // If starting to find an object, set degree start and tell the app.
        if (found_obj != finding_obj && finding_obj == true) {
            obj_degree_start = i;
            uart_log("start obj");
        // Otherwise, if finding the end of an object, build it and add it to the array.
        } else if (found_obj != finding_obj && finding_obj == false) {
            scan_buildObject(i, obj_degree_start, objects);
        }
        found_obj = finding_obj;
    }
    // If we are finding an object and the scan has finished, build it ending at 180.
    if (found_obj) {
        scan_buildObject(180, obj_degree_start, objects);
    }
    // Log the labels for the data to come.
    uart_log("Angle\tDist\tWidth");
    // For each object, get accurate ping distance, set the linear width, and send the data to the app.
    for (i = 0; i < object_num; i++) {
        // Break if interrupt received.
        if (interrupt_stopScan || interrupt_emergency) return;
        // Get accurate distance to object.
        objects[i].dist = scan_ping(objects[i].angle);
        // Calculate linear width of the object.
        double radians = (objects[i].radial_width * PI) / 360.0;
        objects[i].linear_width = 2 * objects[i].dist * tan(radians);
        // If the object is within 50 cm, send it to the app.
        if (objects[i].dist < 50 + objects[i].linear_width) {
            uart_object(objects[i]);
        }
    }

    // Reset the object num for the next scan.
    object_num = 0;
}
