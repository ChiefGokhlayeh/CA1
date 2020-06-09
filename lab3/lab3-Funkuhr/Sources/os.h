/*  Header for Operating System module

    Computerarchitektur / Computer Architecture
    (C) 2020 J. Friedrich, W. Zimmermann 
    Hochschule Esslingen

    Author:   W.Zimmermann, May 06, 2020
    Modified: 
*/

#define OSNUMTASKS 8	 		// Number of operating system tasks

typedef struct 				// Data type for tasks
{   void (*osTaskFunction)(enum event);	// Function pointer to task
    int *osPEvent;			// Event, which trigger task execution
} osTCB;

void initOS(osTCB osTaskList[]);	// Function to start the operating system, does never return