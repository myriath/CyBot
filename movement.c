/**
 * movement.c
 *
 * @author Mitch Hudson
 * @author Wyatt Rayl
 * @author Samuel Schaphorst
 *
 * Handles different move operations for the robot.
 */

#include "movement.h"

/**
 * Log a message then stop moving.
 */
void msgStop(const char* str) {
    uart_log(str);
    interrupt_stopMove = true;
}

/**
 * Checks if the robot has bumped into anything, then stops if we have.
 */
bool bump(oi_t* sensorData) {
    bool left = sensorData->bumpLeft;
    bool right = sensorData->bumpRight;

    bool lRight = sensorData->lightBumperRight;
    bool lFrontRight = sensorData->lightBumperFrontRight;
    bool lCenterRight = sensorData->lightBumperCenterRight;
    bool lCenterLeft = sensorData->lightBumperCenterLeft;
    bool lFrontLeft = sensorData->lightBumperFrontLeft;
    bool lLeft = sensorData->lightBumperLeft;

    if (left) msgStop("BUMP LEFT");
    else if (right) msgStop("BUMP RIGHT");
    else if (lRight) msgStop("LIGHT BUMP RIGHT");
    else if (lFrontRight) msgStop("LIGHT BUMP FRONT RIGHT");
    else if (lCenterRight) msgStop("LIGHT BUMP CENTER RIGHT");
    else if (lCenterLeft) msgStop("LIGHT BUMP CENTER LEFT");
    else if (lFrontLeft) msgStop("LIGHT BUMP FRONT LEFT");
    else if (lLeft) msgStop("LIGHT BUMP LEFT");

    return left || right || lRight || lFrontRight || lCenterRight || lCenterLeft || lFrontLeft || lLeft;
}

void turn(oi_t* sensorData, double degrees, int speedRight, int speedLeft) {
    oi_setWheels(speedRight, speedLeft);
    double sum = 0;
    while (sum < abs(degrees)) {
        if (command_byte == B_EMERGENCY_STOP) return;
        lcd_printf("Angle: %lf", sum);
        oi_update(sensorData);
        sum += abs(sensorData->angle);
    }
    oi_setWheels(0,0);
    lcd_clear();

    uart_move(0, degrees);
    uart_stopWait();
}

/**
 * Turns the given number of degrees to the left.
 */
void turn_left(oi_t* sensorData, double degrees) {
    turn(sensorData, degrees, speed, -speed);
}

/**
 * Turns the given number of degrees to the right.
 */
void turn_right(oi_t* sensorData, double degrees) {
    turn(sensorData, degrees, -speed, speed);
}

/**
 * Move the given distance in a straight line with the given speed.
 */
void move(oi_t* sensorData, double distanceMM, int speed) {
    oi_setWheels(speed, speed);
    int sum = 0;
    while (sum < abs(distanceMM)) {
        if (interrupt_emergency || interrupt_stopMove) return;
        oi_update(sensorData);
        sum += abs(sensorData->distance);
        lcd_printf("Distance traveled: \n%d\nDistanceMM: \n%lf", sum, distanceMM);
        cliffSensor(sensorData);
        edge_detect(sensorData);
        bump(sensorData);
    }
    oi_setWheels(0,0);
    lcd_clear();
    uart_move(distanceMM/10, 0);
    uart_stopWait();
}

/**
 * Moves the given distance forward.
 */
void move_forward(oi_t* sensorData, double distanceMM) {
    move(sensorData, distanceMM, speed);
}

/**
 * Move the given distance backwards.
 */
void move_backward(oi_t* sensorData, double distanceMM) {
    move(sensorData, distanceMM, -speed);
}

// Cumulative distance and angle as the robot moves.
double cumulativeDistance = 0;
double cumulativeAngle = 0;

/**
 * Detects if the robot is crossing a line.
 */
bool edge_detect(oi_t *sensor_data) {
    // Threshold for the line sensor
    int threshold = 2550;
    bool right = sensor_data->cliffRightSignal > threshold;
    bool left = sensor_data->cliffLeftSignal > threshold;
    bool frontRight = sensor_data->cliffFrontRightSignal > threshold;
    bool frontLeft = sensor_data->cliffFrontLeftSignal > threshold;

    bool move = false;

    if (right) {
        uart_logEdge("LINE R", sensor_data->cliffRightSignal);
        move = true;
    }
    if (frontRight) {
        uart_logEdge("LINE FR", sensor_data->cliffFrontRightSignal);
        move = true;
    }
    if (frontLeft) {
        uart_logEdge("LINE FL", sensor_data->cliffFrontLeftSignal);
        move = true;
    }
    if (left) {
        uart_logEdge("LINE L", sensor_data->cliffLeftSignal);
        move = true;
    }

    if (move) {
        oi_setWheels(0, 0);
        timer_waitMillis(100);
        oi_update(sensor_data);
        cumulativeDistance += sensor_data->distance / 10;
        cumulativeAngle += sensor_data->angle;
        uart_move(cumulativeDistance, cumulativeAngle);
        cumulativeDistance = 0;
        cumulativeAngle = 0;
        move_backward(sensor_data, 10);
        interrupt_stopMove = true;
    }

    return move;
}

/**
 * Stops and backs the robot up when a cliff is detected.
 */
void cliff(oi_t* sensor_data, const char* str) {
    oi_setWheels(0, 0);
    timer_waitMillis(100);
    oi_update(sensor_data);
    cumulativeDistance += sensor_data->distance / 10;
    cumulativeAngle += sensor_data->angle;
    uart_move(cumulativeDistance, cumulativeAngle);
    cumulativeDistance = 0;
    cumulativeAngle = 0;
    move(sensor_data, 100, -speed * 2);
    uart_log(str);
    interrupt_stopMove = true;
}

/**
 * Detects when the robot is going off a cliff.
 */
bool cliffSensor(oi_t *sensor_data){
    bool isCliffRight = 0;
    bool isCliffFrontRight = 0;
    bool isCliffFrontLeft = 0;
    bool isCliffLeft = 0;

    isCliffRight = sensor_data -> cliffRight;
    isCliffFrontRight = sensor_data -> cliffFrontRight;
    isCliffFrontLeft = sensor_data -> cliffFrontLeft;
    isCliffLeft = sensor_data -> cliffLeft;

    if(isCliffFrontRight){
        cliff(sensor_data, "CLIFF FR");
    }
    else if(isCliffFrontLeft){
        cliff(sensor_data, "CLIFF FL");
    }
    else if(isCliffRight){
        cliff(sensor_data, "CLIFF R");
    }
    else if(isCliffLeft){
        cliff(sensor_data, "CLIFF L");
    }

    return isCliffRight || isCliffFrontRight || isCliffFrontLeft || isCliffLeft;
}
