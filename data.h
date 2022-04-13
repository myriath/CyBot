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

extern const int SCAN_RIGHT;
extern const int SCAN_LEFT;
extern const int INT_ARR_DIGITS;
extern const int SEND_FLOAT_DECIMAL_PLACES;
extern const int MAX_OBJECTS;
extern int object_num;
extern int speed;
extern const int MAX_COMMAND_LEN;

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
