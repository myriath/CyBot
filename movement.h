/**
 * movement.h
 *
 * @author Mitch Hudson
 * @author Wyatt Rayl
 * @author Samuel Schaphorst
 *
 * Handles different move operations for the robot.
 */

#include "premade/open_interface.h"
#include "data/data.h"
#include "comms/uart.h"
#include "data/interrupts.h"
#ifndef MOVEMENT_H_
#define MOVEMENT_H_

// Cumulative values to track movement defined in movement.c
extern double cumulativeAngle;
extern double cumulativeDistance;

// Turns the desired number of degrees to the left.
void turn_left(oi_t* sensorData, double degrees);
// Turns the desired number of degrees to the right.
void turn_right(oi_t* sensorData, double degrees);
// Moves forward the desired number of millimeters.
void move_forward(oi_t* sensorData, double distanceMM);
// Moves backwards the desired number of millimeters.
void move_backward(oi_t* sensorData, double distanceMM);
// Detects if the robot has bumped into something.
bool bump(oi_t* sensorData);
// Detects if the robot is crossing a line.
bool edge_detect(oi_t* sensorData);
// Detects if the robot is going over a cliff.
bool cliffSensor(oi_t* sensorData);

#endif /* MOVEMENT_H_ */
