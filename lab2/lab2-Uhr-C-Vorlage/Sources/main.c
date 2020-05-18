/*  Lab 2 - Main C file for Clock program

    Computerarchitektur / Computer Architecture
    (C) 2020 J. Friedrich, W. Zimmermann
    Hochschule Esslingen

    Author:  W.Zimmermann, Apr 15, 2020
    Modified: A.Baulig, J.Janusch
*/

#include <hidef.h>
#include <mc9s12dp256.h>

#pragma LINK_INFO DERIVATIVE "mc9s12dp256b"

#include "wrapper.h"
#include "clock.h"
#include "ui.h"
#include "thermometer.h"

static unsigned char tick_event = 0;

void handle_tick(void)
{
    tick_event = 1;
}

void main(void)
{
    EnableInterrupts;

    ui_init();

    clock_init();

    thermometer_init();

    init_ticker();

    for (;;)
    {
        if (tick_event)
        {
            tick_event = 0;

            thermometer_take_measurement();

            clock_tick();

            ui_tick();
        }

        /* In the simulator it was observed, that holding the button will STALL
         * the simulator, i.e. this loop is executed contiously without
         * interrupts correctly being processed. This seems to be a limitation
         * of the simulator, as real hardware would interrupt the normal
         * programm flow and prioritize the interrupt. With waitForInterrupt()
         * we instruct the CPU to halt until any interrupt occurs, with the neat
         * side effect of saving power.
         *
         * As we currently have the timer interrupt configured with a period of
         * 10ms, this instruction should halt 10ms max. */
        wait_for_interrupt();
    }
}
