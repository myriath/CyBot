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

extern double cumulativeAngle;
extern double cumulativeDistance;

void turn_left(oi_t* sensorData, double degrees);
void turn_right(oi_t* sensorData, double degrees);
void move_forward(oi_t* sensorData, double distanceMM);
void move_backward(oi_t* sensorData, double distanceMM);
void move_square(oi_t* sensorData);
bool bump(oi_t* sensorData);
bool edge_detect(oi_t* sensorData);
bool cliffSensor(oi_t* sensorData);

#endif /* MOVEMENT_H_ */
