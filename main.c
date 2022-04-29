/**
 * Main.c
 *
 * @author Mitch Hudson
 * @author Wyatt Rayl
 * @author Samuel Schaphorst
 */

#define PI 3.1415926535

#include "main.h"

// Array of objects used when scanning.
TallObject objects[MAX_OBJECTS];

/**
 * Calibration method for the servo.
 * Press button 4 while in main to enter.
 * First you set the right value using 1 and 2 to turn left and right, and 3 to confirm.
 * Then you set the left value using 1 and 2 to turn and 4 to confirm.
 */
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
    // Prints new values to the lcd so you can update it in main.
    lcd_printf("Left:  %d\nRight: %d", left, right);
}

/**
 * Parses the command given after typing a ":"
 */
int parse(char* command, oi_t* sensorData) {
    // Create temporary arrays/pointers for use later.
    char str[MAX_COMMAND_LEN];
    char* params[MAX_COMMAND_LEN];
    int paramCnt = 0;
    // Copies string from constant input to workable array.
    strcpy(str, command);

    // Continually breaks up the command by spaces and puts each pointer into an array for parameters.
    char* ptr = strtok(str, " ");
    while (ptr != NULL) {
        params[paramCnt++] = ptr;
        ptr = strtok(NULL, " ");
    }

    // Based on params from above, execute the desired command.
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
        float dist = atof(params[1]);
        if (dist < 0) {
            move_backward(sensorData, -dist);
        } else {
            move_forward(sensorData, dist);
        }
        return 4;
    } else if (strcmp(params[0], ":speed") == 0 && paramCnt == 2) {
        speed = atoi(params[1]);
        uart_logEdge("Speed set to", speed);
        return 5;
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
        return 9;
    }
    // If no command is known, return -1 which prints unknown command to the screen.
    return -1;
}

/**
 * Handles the button inputs every cycle of main's while loop.
 */
uint8_t handle_buttons(uint8_t last) {
    // Gets the button.
    uint8_t button = button_getButton();
    // Depending on which buttons are pressed, do something.
    if (button != last) {
        if (button == 1) {
            uart_log("Pressed 1");
        } else if (button == 2) {
            uart_log("Pressed 2");
        } else if (button == 3) {
            uart_log("Pressed 3");
        } else if (button == 4) {
            uart_log("Pressed 4");
            calibrate();
        }
        return button;
    }
    return last;
}

// Enumeration used to store the state of the robot.
enum{NONE, STOPPED, TURNING, FORWARD, REVERSE} STATE_ROBOT = NONE;

/**
 * Main function
 * Initializes all functions, starts uart comms, and starts reading commands.
 */
int main() {
    // Initialize button, timer, uart, and lcd.
    button_init();
    timer_init();
    uart_init();
    lcd_init();

    // Connect to oi for movement and sensors.
    oi_t* sensorData = oi_alloc();
    oi_init(sensorData);

    // Calibration data, set this BEFORE scan init
    left = 283800;
    right = 311200;
    // Initializes the scan functions with the given servo values.
    scan_init();

    // Keeps track of previous button pressed.
    uint8_t last = 0;
    // Array for storing the command as it is written.
    int commandLen = 0;
    char command[MAX_COMMAND_LEN];

    // Main loop.
    while (true) {
        // Reset interrupts each loop.
        interrupt_reset();
        // Based on the current state, update cumulative distance values with new data.
        if (STATE_ROBOT == FORWARD) {
            oi_update(sensorData);
            cumulativeDistance += sensorData->distance / 10;
            cumulativeAngle += sensorData->angle;
            if (cliffSensor(sensorData)) STATE_ROBOT = STOPPED;
            else if (edge_detect(sensorData)) STATE_ROBOT = STOPPED;
            else if (bump(sensorData)) STATE_ROBOT = STOPPED;
        } else if (STATE_ROBOT == REVERSE) {
            oi_update(sensorData);
            cumulativeDistance += sensorData->distance / 10;
            cumulativeAngle += sensorData->angle;
            if (cliffSensor(sensorData)) STATE_ROBOT = STOPPED;
            else if (edge_detect(sensorData)) STATE_ROBOT = STOPPED;
        } else if (STATE_ROBOT == TURNING) {
            oi_update(sensorData);
            cumulativeDistance += sensorData->distance / 10;
            cumulativeAngle += sensorData->angle;
            if (cliffSensor(sensorData)) STATE_ROBOT = STOPPED;
            else if (edge_detect(sensorData)) STATE_ROBOT = STOPPED;
            else if (bump(sensorData)) STATE_ROBOT = STOPPED;
        } else if (STATE_ROBOT == STOPPED) {
            // Once the button is let go, turn off wheels, wait until robot comes to a stop, send move data to the app.
            oi_setWheels(0, 0);
            timer_waitMillis(100);
            oi_update(sensorData);
            cumulativeDistance += sensorData->distance / 10;
            cumulativeAngle += sensorData->angle;

            uart_move(cumulativeDistance, cumulativeAngle);
            STATE_ROBOT = NONE;
            cumulativeDistance = 0;
            cumulativeAngle = 0;
            continue;
        }
        // Buttons
        last = handle_buttons(last);

        // App
        char i = command_byte;
        if (i != 0) {
            command_byte = 0;
            // If we are not writing a command.
            if (!commandLen) {
                // Depending on byte received, do a certain command. These are defined in data.c
                if (i == B_SCAN) {
                    scanning = true;
                    scan_full(objects);
                    scanning = false;
                } else if (i == B_MOVE_STOP) {
                    STATE_ROBOT = STOPPED;
                    oi_setWheels(0, 0);
                } else if (i == B_MOVE_FORWARD) {
                    STATE_ROBOT = FORWARD;
                    oi_setWheels(speed, speed);
                    cumulativeDistance = 0;
                    cumulativeAngle = 0;
                } else if (i == B_MOVE_LEFT) {
                    STATE_ROBOT = TURNING;
                    oi_setWheels(speed, -speed);
                    cumulativeDistance = 0;
                    cumulativeAngle = 0;
                } else if (i == B_MOVE_REVERSE) {
                    STATE_ROBOT = REVERSE;
                    oi_setWheels(-speed, -speed);
                    cumulativeDistance = 0;
                    cumulativeAngle = 0;
                } else if (i == B_MOVE_RIGHT) {
                    STATE_ROBOT = TURNING;
                    oi_setWheels(-speed, speed);
                    cumulativeDistance = 0;
                    cumulativeAngle = 0;
                } else if (i == B_MOVE_FORWARD_INC) {
                    move_forward(sensorData, 50);
                } else if (i == B_MOVE_LEFT_INC) {
                    turn_left(sensorData, 5);
                } else if (i == B_MOVE_REVERSE_INC) {
                    move_backward(sensorData, 50);
                } else if (i == B_MOVE_RIGHT_INC) {
                    turn_right(sensorData, 5);
                } else if (i == ':') {
                    // Begin reading a command.
                    command[commandLen++] = i;
                    command[commandLen] = '\0';

                    lcd_printf("%c : %d | %d\n%s", i, i, commandLen, command); // debug output
                }
            // B_NEWLINE indicates the end of the command and tells the robot to parse the command.
            } else if (i == B_NEWLINE) {
                // clear lcd
                lcd_printf("");

                // Reset commandLen for next command.
                commandLen = 0;
                // Parse the command received and based on result send data back to the app.
                int result = parse(command, sensorData);
                if (result == -1) {
                    uart_log("Error parsing command:");
                    uart_log(command);
                } else if (result == 0) { // quit
                    break;
                } else if (result == 1) { // scan
                    uart_log("Scan Complete");
                }
            // If receiving a backspace, remove most recent character.
            } else if (i == 8 || i == 127) {
                if (commandLen > 0) {
                    command[--commandLen] = '\0';

                    lcd_printf("%c : %d | %d\n%s", i, i, commandLen, command); // debug output
                }
            // If there is no space in the array, ignore input.
            } else if (commandLen > MAX_COMMAND_LEN - 1) {
                continue;
            // If none of the above, add the character received to the array.
            } else {
                command[commandLen++] = i;
                command[commandLen] = '\0';

                lcd_printf("%c : %d | %d\n%s", i, i, commandLen, command); // debug output
            }
        }
    }

    // After finishing main, release the oi data.
    oi_free(sensorData);
    // Return
    return 0;
}
