/**
 * Main.c
 *
 * @author Mitch Hudson
 * @author Wyatt Rayl
 * @author Samuel Schaphorst
 */

#define PI 3.1415926535

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "button.h"
#include "open_interface.h"
#include "Timer.h"
#include "lcd.h"
//#include "cyBot_Scan.h"

#include "uart.h"
#include "data.h"
#include "movement.h"
#include "structs.h"
#include "scan/scan.h"

//cyBOT_Scan_t scanData;
TallObject objects[MAX_OBJECTS];
TallObject smallest;

void measure_dist() {
    uart_log();
    uart_sendFloat(raw_to_dist(adc_read()));
    uart_end();
}

void calibrate() {
    uint8_t last = 0;
    servo_setMatch(right);
    lcd_printf("calibrating");
    while (true) {
        uint8_t button = button_getButton();
        if (button != last) {
            if (button == 1) {
                right -= 200;
            } else if (button == 2) {
                right += 200;
            } else if (button == 3) {
                break;
            }
            servo_setMatch(right);
            last = button;
        }
    }
    servo_setMatch(left);
    while (true) {
        uint8_t button = button_getButton();
        if (button != last) {
            if (button == 1) {
                left -= 200;
            } else if (button == 2) {
                left += 200;
            } else if (button == 4) {
                break;
            }
            servo_setMatch(left);
            last = button;
        }
    }

    lcd_printf("Left:  %d\nRight: %d", left, right);
}

int parse(char* command, oi_t* sensorData) {
    char str[MAX_COMMAND_LEN];
    char* params[MAX_COMMAND_LEN];
    int paramCnt = 0;
    strcpy(str, command);

    char* ptr = strtok(str, " ");
    while (ptr != NULL) {
        params[paramCnt++] = ptr;
        ptr = strtok(NULL, " ");
    }

    if (strcmp(params[0], ":q") == 0 && paramCnt == 1) {
        return 0;
    } else if (strcmp(params[0], ":calibrate") == 0 && paramCnt == 1) {
        calibrate();
        return 2;
    } else if (strcmp(params[0], ":print") == 0 && paramCnt > 1) {
        int i = 1;
        while (i < paramCnt) {
            lcd_puts(params[i++]);
            if (i != paramCnt - 1) lcd_puts(" ");
        }
        return 3;
    } else if (strcmp(params[0], ":move") == 0 && paramCnt == 2) {
        if (strcmp(params[1], "square") == 0) move_square(sensorData);
        else if (strcmp(params[1], "smallest") == 0) {
            if (smallest.angle < 90) turn_right(sensorData, 90 - smallest.angle);
            else turn_left(sensorData, smallest.angle - 90);
            move_forward(sensorData, 10 * smallest.dist);
        } else {
            float dist = atof(params[1]);
            if (dist < 0) {
                move_backward(sensorData, -dist);
            } else {
                move_forward(sensorData, dist);
            }
        }
        return 4;
    } else if (strcmp(params[0], ":speed") == 0 && paramCnt == 2) {
        speed = atoi(params[1]);
        uart_log();
        uart_sendStr("Speed set to ");
        uart_sendInt(speed);
        uart_end();
        return 5;
    } else if (strcmp(params[0], ":scantest") == 0 && paramCnt == 1) {
        scan_test();
        return 6;
    } else if (strcmp(params[0], ":fullscan") == 0 && paramCnt == 1) {
        scan_full(objects);
        uart_stopWait();
        return 7;
    } else if (strcmp(params[0], ":turn") == 0 && paramCnt == 2) {
        float degrees = atof(params[1]);
        if (degrees < 0) {
            turn_right(sensorData, -degrees);
        } else {
            turn_left(sensorData, degrees);
        }
        return 8;
    } else if (strcmp(params[0], ":servo") == 0 && paramCnt == 2) {
        int degrees = atoi(params[1]);
        servo_move(degrees);
        oi_setWheels(100,100);
    }
    return -1;
}

uint8_t handle_buttons(uint8_t last) {
    uint8_t button = button_getButton();
    if (button != last) {
        if (button > 0 && button < 5) {
            uart_log();
        }
        if (button == 1) {
            uart_sendStr("Pressed 1");
        } else if (button == 2) {
            uart_sendStr("Pressed 2");
        } else if (button == 3) {
            uart_sendStr("Pressed 3");
        } else if (button == 4) {
            uart_sendStr("Pressed 4");
            calibrate();
        }
        if (button > 0 && button < 5) {
            uart_end();
        }
        return button;
    }
    return last;
}


 int main() {
    button_init();
    timer_init();
    lcd_init();

    oi_t* sensorData = oi_alloc();
    oi_init(sensorData);

    uart_init();

    // Calibration data, set this BEFORE scan init
    left = 284400;
    right = 312600;
    scan_init();

    uint8_t last = 0;
    int commandLen = 0;
    char command[MAX_COMMAND_LEN];

//    int overflows = 0;
//    while (true) {
//        last = handle_buttons(last);
//
//        ping_trigger();
//        timer_waitMillis(500);
//        float dist = ping_getDistanceCM();
//        if (dist < 0) {
//            overflows++;
//        }
//
//        lcd_printf("RAW: %0.2f\n CM: %0.2f\nOVF: %d", ping_getDistanceRAW(), dist, overflows);
//    }

//    while (true) {
//        char i = command_byte;
//        if (i == B_MOVE_FORWARD) {
//            command_byte = 0;
//            measure_dist();
//        }

//    }
//    servo_move(180);
    while (true){
        interrupt_reset();
        // Buttons
        last = handle_buttons(last);
//        uart_log();
//        uart_sendStr("hello");
//        uart_end();
//        timer_waitMillis(1000);

        // PuTTY
        char i = command_byte;
//        lcd_printf("%d", i);
        if (i != 0) {
            command_byte = 0;
            if (!commandLen) {
                if (i == B_SCAN) {
                    scan_full(objects);
                } else if (i == B_MOVE_STOP) {
                    oi_setWheels(0, 0);
                } else if (i == B_MOVE_FORWARD) {
                    oi_setWheels(speed, speed);
                } else if (i == B_MOVE_LEFT) {
                    oi_setWheels(speed, -speed);
                } else if (i == B_MOVE_REVERSE) {
                    oi_setWheels(-speed, -speed);
                } else if (i == B_MOVE_RIGHT) {
                    oi_setWheels(-speed, speed);
                } else if (i == B_MOVE_FORWARD_INC) {
                    move_forward(sensorData, 50);
                } else if (i == B_MOVE_LEFT_INC) {
                    turn_left(sensorData, 5);
                } else if (i == B_MOVE_REVERSE_INC) {
                    move_backward(sensorData, 50);
                } else if (i == B_MOVE_RIGHT_INC) {
                    turn_right(sensorData, 5);
                } else if (i == ':') {
                    command[commandLen++] = i;
                    command[commandLen] = '\0';

                    lcd_printf("%c : %d | %d\n%s", i, i, commandLen, command); // debug output
                }
            } else if (i == B_NEWLINE) {
                // clear lcd
                lcd_printf("");

                commandLen = 0;
                int result = parse(command, sensorData);
                if (result == -1) {
                    uart_log();
                    uart_sendStr("Error parsing command: ");
                    uart_sendStr(command);
                    uart_end();
                } else if (result == 0) { // quit
                    break;
                } else if (result == 1) { // scan
                    uart_log();
                    uart_sendStr("Scan Complete");
                    uart_end();
                }
            } else if (i == 8 || i == 127) {
                if (commandLen > 0) {
                    command[--commandLen] = '\0';

                    lcd_printf("%c : %d | %d\n%s", i, i, commandLen, command); // debug output
                }
            } else if (commandLen > MAX_COMMAND_LEN - 1) {
                continue;
            } else {
                command[commandLen++] = i;
                command[commandLen] = '\0';

                lcd_printf("%c : %d | %d\n%s", i, i, commandLen, command); // debug output
            }
        }
    }

    oi_free(sensorData);
    return 0;
}
