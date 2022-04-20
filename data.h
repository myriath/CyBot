/**
 * data.h
 *
 * @author Mitch Hudson
 * @author Wyatt Rayl
 * @author Samuel Schaphorst
 *
 * Handles structs and constants used by the rest of the program
 */

#ifndef DATA_H_
#define DATA_H_

#define MAX_OBJECTS 10
#define INT_ARR_DIGITS 10
#define SEND_FLOAT_DECIMAL_PLACES 2
#define MAX_COMMAND_LEN 100

extern int object_num;
extern int speed;

extern const uint8_t B_WAIT;
extern const uint8_t B_EMERGENCY_STOP;
extern const uint8_t B_SCAN;
extern const uint8_t B_MOVE_STOP;
extern const uint8_t B_MOVE_FORWARD;
extern const uint8_t B_MOVE_FORWARD_INC;
extern const uint8_t B_MOVE_LEFT;
extern const uint8_t B_MOVE_LEFT_INC;
extern const uint8_t B_MOVE_REVERSE;
extern const uint8_t B_MOVE_REVERSE_INC;
extern const uint8_t B_MOVE_RIGHT;
extern const uint8_t B_MOVE_RIGHT_INC;
extern const uint8_t B_NEWLINE;

#endif
