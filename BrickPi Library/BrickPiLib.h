//
// Useful Functions used by the BrickPi projects.
//

#include "BrickPi.h"
#include "wiringPi.h"
#include "tick.h"
#include <pthread.h>

#define BOOL char
#define TRUE 1
#define FALSE 0


p_thread setupKillSwitch(int KillSwitchPort);
void setupSensors(int Port1, int Port2, int Port3, int Port4);

void setupMotors(int mleft, int mright);

void LED(BOOL LedLeft, BOOL LedRight);

