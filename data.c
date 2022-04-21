/**
 * data.c
 *
 * @author Mitch Hudson
 * @author Wyatt Rayl
 * @author Samuel Schaphorst
 *
 * Handles structs and constants used by the rest of the program
 */

#include <stdint.h>

// Tracks max number of objects and how many there are
int object_num = 0;

// Speed for the robot
int speed = 50;

// App byte commands
const uint8_t B_WAIT = 0x01;
const uint8_t B_EMERGENCY_STOP = 0x02;
const uint8_t B_SCAN = 0x03;
const uint8_t B_SCAN_RESET = 0x04;
const uint8_t B_NEWLINE = 0x0a;
const uint8_t B_MOVE_STOP = 0x10;
const uint8_t B_MOVE_FORWARD = 0x11;
const uint8_t B_MOVE_FORWARD_INC = 0x15;
const uint8_t B_MOVE_LEFT = 0x12;
const uint8_t B_MOVE_LEFT_INC = 0x16;
const uint8_t B_MOVE_REVERSE = 0x13;
const uint8_t B_MOVE_REVERSE_INC = 0x17;
const uint8_t B_MOVE_RIGHT = 0x14;
const uint8_t B_MOVE_RIGHT_INC = 0x18;
