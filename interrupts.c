//
// Created by mitch on 04/19/2022.
//

#include "interrupts.h"

bool interrupt_emergency = false;
bool interrupt_stopScan = false;
bool interrupt_stopMove = false;

void interrupt_reset() {
    interrupt_emergency = false;
    interrupt_stopScan = false;
    interrupt_stopMove = false;
}
