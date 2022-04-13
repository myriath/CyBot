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
TallObject smallest;

double raw_to_dist(double ir_val) {
    if (ir_val <= 118.22901) return -1;
    return 28541.58384 / (ir_val - 118.22901);
}

void scan_test() {
//    cyBOT_Scan(90, &scanData);
    uart_log();
    uart_sendStr("IR val");
    uart_end();

    int ir_val = 0;
    int j = 0;
    for (j = 0; j < 10; j++) {
//        cyBOT_Scan(90, &scanData);
//        ir_val += scanData.IR_raw_val;
        ir_val += adc_read();
    }
    ir_val /= 10;
    uart_log();
    uart_sendInt(ir_val);
    uart_end();
}

void measure_dist() {
    uart_log();
    uart_sendFloat(raw_to_dist(adc_read()));
    uart_end();
}

void build_object(int i, int obj_degree_start, TallObject* objects) {
    TallObject obj;
    obj.obj_num = object_num++;
    obj.radial_width = i - obj_degree_start;
    obj.angle = obj_degree_start + (obj.radial_width / 2);

    if (obj.radial_width > 2) {
        objects[obj.obj_num] = obj;
        uart_log();
        uart_sendStr("end obj");
    } else {
        uart_log();
        uart_sendStr("ignored");
    }
    uart_end();
}

void full_scan() {
    TallObject objects[MAX_OBJECTS];
    bool found_obj = false;
    int obj_degree_start;

//    cyBOT_Scan(0, &scanData);
    uart_log();
    uart_sendStr("Degrees\t\tIR val");
    uart_end();

    int i;
    for (i = 0; i <= 180; i += 2) {
        if (command_byte == B_EMERGENCY_STOP) return;

        int j;
        int ir_val = 0;
        for (j = 0; j < 2; j++) {
//            cyBOT_Scan(i, &scanData);
//            ir_val += scanData.IR_raw_val;
        }
        ir_val /= 2;

        uart_log();
        uart_sendInt(i);
        uart_sendStr("\t\t");
        uart_sendInt(ir_val);
        uart_sendStr("\t\t");

        float ir_dist = 21602.74 / (ir_val - 664.47);
        uart_sendFloat(ir_dist);
        uart_end();
        bool finding_obj = ir_dist > 10 && ir_dist < 50;

        if (finding_obj) {
            uart_scan();
            uart_sendInt(i);
            uart_sendStr(",");
            uart_sendFloat(ir_dist);
            uart_end();
        }

        if (found_obj != finding_obj && finding_obj == true) {
            obj_degree_start = i;
            uart_log();
            uart_sendStr("start obj");
            uart_end();
        } else if (found_obj != finding_obj && finding_obj == false) {
            build_object(i, obj_degree_start, objects);
        }
        found_obj = finding_obj;
    }
    if (found_obj) {
        build_object(180, obj_degree_start, objects);
    }

    for (i = 0; i < object_num; i++) {
        if (command_byte == B_EMERGENCY_STOP) return;
        float ping_dist = 0;
        int j;
        for (j = 0; j < 6; j++) {
//            cyBOT_Scan(objects[i].angle, &scanData);
//            ping_dist += scanData.sound_dist;
        }
        ping_dist /= 6;

        objects[i].dist = ping_dist;

        double radians = (objects[i].radial_width * PI) / 360.0;
        objects[i].linear_width = 2 * ping_dist * tan(radians);
    }

    uart_log();
    uart_sendStr("Obj#\tAngle\tDist\tRWidth\tLWidth");
    uart_end();
    for (i = 0; i < object_num; i++) {
        if (command_byte == B_EMERGENCY_STOP) return;
        uart_log();
        uart_sendInt(objects[i].obj_num); // send object data to screen
        uart_sendChar('\t');
        uart_sendInt(objects[i].angle);
        uart_sendChar('\t');
        uart_sendFloat(objects[i].dist);
        uart_sendChar('\t');
        uart_sendInt(objects[i].radial_width);
        uart_sendChar('\t');
        uart_sendFloat(objects[i].linear_width);
        uart_end();

        uart_object();
        uart_sendInt(objects[i].angle);
        uart_sendChar(',');
        uart_sendFloat(objects[i].dist);
        uart_sendChar(',');
        uart_sendFloat(objects[i].linear_width);
        uart_end();
    }

    object_num = 0;
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
//        cyBOT_SERVRO_cal_t calibration = cyBOT_SERVO_cal();
//        right_calibration_value = calibration.right;
//        left_calibration_value = calibration.left;

        uart_log();
        uart_sendStr("Right Calibration Value: ");
//        uart_sendInt(calibration.right);
        uart_end();
        uart_log();
        uart_sendStr("Left Calibration Value: ");
//        uart_sendInt(calibration.left);
        uart_end();

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
        full_scan();
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
    }

    return -1;
}

void calibrate() {
    uint8_t last = 0;
    servo_setMatch(right);
    while (true) {
        uint8_t button = button_getButton();
        if (button != last) {
            if (button == 1) {
                right -= 200;
                unsigned int r = right;
                servo_setMatch(right);
            } else if (button == 2) {
                right += 200;
                unsigned int r = right;
                servo_setMatch(right);
            } else if (button == 3) {
                break;
            }
            last = button;
        }
    }
    servo_setMatch(left);
    while (true) {
        uint8_t button = button_getButton();
        if (button != last) {
            if (button == 1) {
                left -= 200;
                servo_setMatch(left);
            } else if (button == 2) {
                left += 200;
                unsigned int r = left;
                servo_setMatch(left);
            } else if (button == 4) {
                break;
            }
            last = button;
        }
    }

    lcd_printf("Left:  %d\nRight: %d", left, right);
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
//    cyBOT_init_Scan(0x7);
    adc_init();
     ping_init();
     left = 284400;
     right = 312600;

//    right_calibration_value = SCAN_RIGHT;
//    left_calibration_value = SCAN_LEFT;

    uint8_t last = 0;
    int commandLen = 0;
    char command[MAX_COMMAND_LEN];

    servo_init();
    servo_move(90);
    while (true) {
        last = handle_buttons(last);
    }

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
//    while (true){
//        // Buttons
//        last = handle_buttons(last);
////        uart_log();
////        uart_sendStr("hello");
////        uart_end();
////        timer_waitMillis(1000);
//
//        // PuTTY
//        char i = command_byte;
////        lcd_printf("%d", i);
//        if (i != 0) {
//            command_byte = 0;
//            if (!commandLen) {
//                if (i == B_SCAN) {
//                    full_scan();
//                } else if (i == B_MOVE_STOP) {
//                    oi_setWheels(0, 0);
//                } else if (i == B_MOVE_FORWARD) {
//                    oi_setWheels(speed, speed);
//                } else if (i == B_MOVE_LEFT) {
//                    oi_setWheels(speed, -speed);
//                } else if (i == B_MOVE_REVERSE) {
//                    oi_setWheels(-speed, -speed);
//                } else if (i == B_MOVE_RIGHT) {
//                    oi_setWheels(-speed, speed);
//                } else if (i == ':') {
//                    command[commandLen++] = i;
//                    command[commandLen] = '\0';
//
//                    lcd_printf("%c : %d | %d\n%s", i, i, commandLen, command); // debug output
//                }
//            } else if (i == B_NEWLINE) {
//                // clear lcd
//                lcd_printf("");
//
//                commandLen = 0;
//                int result = parse(command, sensorData);
//                if (result == -1) {
//                    uart_log();
//                    uart_sendStr("Error parsing command: ");
//                    uart_sendStr(command);
//                    uart_end();
//                } else if (result == 0) { // quit
//                    break;
//                } else if (result == 1) { // scan
//                    uart_log();
//                    uart_sendStr("Scan Complete");
//                    uart_end();
//                }
//            } else if (i == 8 || i == 127) {
//                if (commandLen > 0) {
//                    command[--commandLen] = '\0';
//
//                    lcd_printf("%c : %d | %d\n%s", i, i, commandLen, command); // debug output
//                }
//            } else if (commandLen > MAX_COMMAND_LEN - 1) {
//                continue;
//            } else {
//                command[commandLen++] = i;
//                command[commandLen] = '\0';
//
//                lcd_printf("%c : %d | %d\n%s", i, i, commandLen, command); // debug output
//            }
//        }
//    }

    oi_free(sensorData);
    return 0;
}
