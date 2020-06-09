/*  Header for Clock module

    Computerarchitektur / Computer Architecture
    (C) 2020 J. Friedrich, W. Zimmermann 
    Hochschule Esslingen

    Author:   W.Zimmermann, May 06, 2020
*/

// Data type for clock events
typedef enum { NOCLOCKEVENT=0, SECONDTICK } CLOCKEVENT;

// Global variable holding the last clock event
extern CLOCKEVENT clockEvent;

// Public functions, for details see clock.c
void initClock(void);
void processEventsClock(CLOCKEVENT event);
void setClock(char hours, char minutes, char seconds);
void displayTimeClock(void);
