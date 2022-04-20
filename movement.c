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
#include "open_interface.h"
#include "data.h"
#include "uart.h"

void bumpLeft(oi_t* sensorData) {
    move_backward(sensorData, 50);
    turn_right(sensorData, 90);
    move_forward(sensorData, 50);
    turn_left(sensorData, 90);
    move_forward(sensorData, 50);
}

void bumpRight(oi_t* sensorData) {
    move_backward(sensorData, 50);
    turn_left(sensorData, 90);
    move_forward(sensorData, 50);
    turn_right(sensorData, 90);
    move_forward(sensorData, 50);
}

void turn_left(oi_t* sensorData, double degrees) {
    double target = degrees;
    oi_setWheels(speed, -speed);
    double sum = 0;
    while (sum < target) {
        if (command_byte == B_EMERGENCY_STOP) return;
        lcd_printf("Angle: %lf", sum);
        oi_update(sensorData);
        sum += sensorData->angle;
    }
    oi_setWheels(0,0);
    lcd_clear();
    uart_log();
    uart_sendFloat(degrees);
    uart_end();

    uart_move();
    uart_sendInt(degrees);
    uart_sendChar(',');
    uart_sendInt(0);
    uart_end();
    uart_stopWait();
}

void turn_right(oi_t* sensorData, double degrees) {
    double target = -degrees;
    oi_setWheels(-speed, speed);
    double sum = 0;
    while (sum > target) {
        if (command_byte == B_EMERGENCY_STOP) return;
        lcd_printf("Angle: %lf", sum);
        oi_update(sensorData);
        sum += sensorData->angle;
    }
    oi_setWheels(0,0);
    lcd_clear();
    uart_log();
    uart_sendFloat(degrees);
    uart_end();

    uart_move();
    uart_sendInt(target);
    uart_sendChar(',');
    uart_sendInt(0);
    uart_end();
    uart_stopWait();
}

void move_forward(oi_t* sensorData, double distanceMM) {
    oi_setWheels(speed, speed);
    int sum = 0;
    while (sum < distanceMM) {
        if (command_byte == B_EMERGENCY_STOP) return;
        oi_update(sensorData);
        sum += sensorData->distance;
        lcd_printf("Distance traveled: \n%d\nDistanceMM: \n%lf", sum, distanceMM);
        if (sensorData->bumpLeft) {
            bumpLeft(sensorData);
            oi_setWheels(speed, speed);
        } else if (sensorData->bumpRight) {
            bumpRight(sensorData);
            oi_setWheels(speed, speed);
        }
    }
    oi_setWheels(0,0);
    lcd_clear();
    uart_move();
    uart_sendInt(0);
    uart_sendChar(',');
    uart_sendFloat(distanceMM/10);
    uart_end();
    uart_stopWait();
}

void move_backward(oi_t* sensorData, double distanceMM) {
    oi_setWheels(-speed, -speed);
    int sum = 0;
    while (sum > -distanceMM) {
        if (command_byte == B_EMERGENCY_STOP) return;
        oi_update(sensorData);
        sum += sensorData->distance;
        lcd_printf("Backwards traveled: \n%d\nDistanceMM: \n%lf", sum, distanceMM);
    }
    oi_setWheels(0,0);
    lcd_clear();
    uart_move();
    uart_sendInt(0);
    uart_sendChar(',');
    uart_sendFloat(distanceMM/10);
    uart_end();
    uart_stopWait();
}

void move_square(oi_t* sensorData) {
    int x = 0;
    while (x < 4) {
        move_forward(sensorData, 500);
        turn_left(sensorData, 90);
        x++;
    }
}

double edge_detect(oi_t *sensor_data) {
    double line = 0;
    int sensor = 0;

    oi_update(sensor_data);
    if ((sensor_data -> cliffRightSignal) > 1500) {
        line = sensor_data -> cliffRightSignal;
        move_backward(sensor_data, 10);
        turn_left(sensor_data, 3);
    }
    if ((sensor_data -> cliffFrontRightSignal) > 1500) {
        line = sensor_data -> cliffFrontRightSignal;
        move_backward(sensor_data, 10);
        turn_left(sensor_data, 3);
    }
    if ((sensor_data -> cliffFrontLeftSignal) > 1500) {
        line = sensor_data -> cliffFrontLeftSignal;
        move_backward(sensor_data, 10);
        turn_right(sensor_data, 3);
    }
    if ((sensor_data -> cliffLeftSignal) > 1500) {
        line = sensor_data -> cliffLeftSignal;
        move_backward(sensor_data, 10);
        turn_right(sensor_data, 3);
    }


    return line;

}


void cliffSensor(oi_t *sensor_data){
    bool isCliffRight = 0;
    bool isCliffFrontRight = 0;
    bool isCliffFrontLeft = 0;
    bool isCliffLeft = 0;

    oi_update(sensor_data);
    isCliffRight = sensor_data -> cliffRight;
    isCliffFrontRight = sensor_data -> cliffFrontRight;
    isCliffFrontLeft = sensor_data -> cliffFrontLeft;
    isCliffLeft = sensor_data -> cliffLeft;

    if(isCliffFrontRight){
        move_backward(sensor_data, 100);
    }
    if(isCliffFrontLeft){
        move_backward(sensor_data, 100);
    }
    if(isCliffRight){
        move_backward(sensor_data, 100);
    }
    if(isCliffLeft){
        move_backward(sensor_data, 100);
    }
    return;
}
