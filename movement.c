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
#include "interrupts.h"

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

void msgStop(const char* str) {
    uart_log();
    uart_sendStr(str);
    uart_end();
    interrupt_stopMove = true;
}

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
        if (interrupt_emergency || interrupt_stopMove) return;
        oi_update(sensorData);
        sum += sensorData->distance;
        lcd_printf("Distance traveled: \n%d\nDistanceMM: \n%lf", sum, distanceMM);
        cliffSensor(sensorData);
        edge_detect(sensorData);
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

void move_backward_speed(oi_t* sensorData, double distanceMM, int speed) {
    oi_setWheels(-speed, -speed);
    int sum = 0;
    while (sum > -distanceMM) {
        if (interrupt_emergency || interrupt_stopMove) return;
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

void move_backward(oi_t* sensorData, double distanceMM) {
    oi_setWheels(-speed, -speed);
    int sum = 0;
    while (sum > -distanceMM) {
        if (interrupt_emergency || interrupt_stopMove) return;
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

int threshold = 2550;
double cumulativeDistance = 0;
double cumulativeAngle = 0;

bool edge_detect(oi_t *sensor_data) {

    int r = sensor_data->cliffRightSignal;
    int l = sensor_data->cliffLeftSignal;
    int fr = sensor_data->cliffFrontRightSignal;
    int fl = sensor_data->cliffFrontLeftSignal;
//    oi_update(sensor_data);
//    cumulativeDistance += sensor_data->distance;
//    r += sensor_data->cliffRightSignal;
//    l += sensor_data->cliffLeftSignal;
//    fr += sensor_data->cliffFrontRightSignal;
//    fl += sensor_data->cliffFrontLeftSignal;
    bool right = r > threshold;
    bool left = l > threshold;
    bool frontRight = fr > threshold;
    bool frontLeft = fl > threshold;

    if (right) {
        uart_log();
        uart_sendFloat((sensor_data -> cliffRightSignal));
        uart_end();
        move_backward(sensor_data, 10);
        turn_left(sensor_data, 3);
        uart_log();
        uart_sendStr("LINE R");
        uart_end();
        interrupt_stopMove = true;
    }
    else if (frontRight) {
        uart_log();
        uart_sendFloat((sensor_data -> cliffFrontRightSignal));
        uart_end();
        move_backward(sensor_data, 10);
        turn_left(sensor_data, 3);
        uart_log();
        uart_sendStr("LINE FR");
        uart_end();
        interrupt_stopMove = true;
    }
    else if (frontLeft) {
        uart_log();
        uart_sendFloat((sensor_data -> cliffFrontLeftSignal));
        uart_end();
        move_backward(sensor_data, 10);
        turn_right(sensor_data, 3);
        uart_log();
        uart_sendStr("LINE FL");
        uart_end();
        interrupt_stopMove = true;
    }
    else if (left) {
        uart_log();
        uart_sendFloat((sensor_data -> cliffLeftSignal));
        uart_end();
        move_backward(sensor_data, 10);
        turn_right(sensor_data, 3);
        uart_log();
        uart_sendStr("LINE L");
        uart_end();
        interrupt_stopMove = true;
    }

    return right || left || frontRight || frontLeft;
}

void cliff(oi_t* sensor_data, const char* str) {
    move_backward_speed(sensor_data, 100, speed * 2);
    uart_log();
    uart_sendStr(str);
    uart_end();
    interrupt_stopMove = true;
}

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
