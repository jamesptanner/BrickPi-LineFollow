#include <stdio.h>
#include <pthread.h>

// some useful defines that I am going to use

#define DWORD int
#define BOOL char
#define TRUE 1
#define FALSE 0 

#include "tick.h"
#include "BrickPi.h"

#define	MOTORS	PORT_B
#define MOTORR	PORT_C
#define MOTORL	PORT_A

#define US_PORT PORT_2
#define LS_PORT PORT_1
#define KS_PORT PORT_3
#define NULL_PORT PORT_4

#define TURN_SCALE 40
#define POWERSCALE 2

#define threadResult(tRes) if(tRes == 0) {dwRunningThreads++;} else {bShutdown = TRUE;}


BOOL bShutdown = FALSE;
DWORD dwRunningThreads = 0;
DWORD dwMotorPower = 0;
DWORD dwMotorSteer = 0;
DWORD dwMidValue;
pthread_t tMovement, tKillSwitch, tSensor;


void* MotorControl(void* args)
{
  
  BrickPi.MotorEnable[MOTORS] = 1;
  BrickPi.MotorEnable[MOTORL] = 1;
  BrickPi.MotorEnable[MOTORR] = 1;
	
  while(!bShutdown)
  {
	BrickPi.MotorSpeed[MOTORS] = dwMotorSteer * TURN_SCALE;
	BrickPi.MotorSpeed[MOTORL] = dwMotorPower * POWERSCALE;
	BrickPi.MotorSpeed[MOTORR] = dwMotorPower * POWERSCALE;
	
	usleep(2500);	
  }
 
  BrickPi.MotorEnable[MOTORS] = 0;
  BrickPi.MotorEnable[MOTORL] = 0;
  BrickPi.MotorEnable[MOTORR] = 0;
  dwRunningThreads--;
}

void* KillSwitch(void* args)
{
	while(TRUE)
	{
		if (BrickPi.Sensor[KS_PORT])
		{
			printf("Shutting Down\r\n");
			bShutdown = TRUE;
			break;

		}
		usleep(5000);
	}
		dwRunningThreads--;
}

void* SensorCapture(void* args)
{
	while(!bShutdown)
	{
	//	printf("Updating Sensor Values\r\n");
	//	printf("Light Sensor: %d\r\n",BrickPi.Sensor[LS_PORT]);
	//	printf("Ultra Sensor: %d\r\n",BrickPi.Sensor[US_PORT]);
		BrickPiUpdateValues();
		usleep(5000);
	}
	dwRunningThreads--;
}

void Setup(void)
{
	ClearTick();
  
	DWORD result = BrickPiSetup();
	BrickPiSetTimeout();
	printf("BrickPiSetup: %d\n", result);
	  
	BrickPi.Address[0] = 1;
	BrickPi.Address[1] = 2;
	BrickPi.Timeout = 1000; 
	  
	BrickPi.SensorType[US_PORT] = TYPE_SENSOR_ULTRASONIC_CONT;
	BrickPi.SensorType[LS_PORT] = TYPE_SENSOR_COLOR_NONE;
	BrickPi.SensorType[KS_PORT] = TYPE_SENSOR_TOUCH;
	BrickPi.SensorType[NULL_PORT] = TYPE_SENSOR_COLOR_NONE;
	bShutdown = FALSE;
	  
	result = BrickPiSetupSensors();
	printf("BrickPiSetupSensors: %d\n", result); 

	threadResult(pthread_create(&tMovement, NULL, MotorControl, NULL));
	threadResult(pthread_create(&tKillSwitch, NULL, KillSwitch, NULL));
	threadResult(pthread_create(&tSensor, NULL, SensorCapture , NULL));
	printf("Running Threads: %d\r\n", dwRunningThreads);
}

void Shutdown()
{

	while(dwRunningThreads)
	{
		printf("Threads Running: %d\r\n",dwRunningThreads);
		sleep(1);
	}
}

void CalibrateSensors()
{

	// swing the wheels to the left/right as far as possible.
	dwMotorSteer = -128;
	sleep(1);					//hacked in until i find a better way of doing this. Sorry motors :(
	dwMotorSteer = 0;
	// slowly swing the wheels back to the other side.
	dwMotorSteer = 4;
	// while that is happening take sensor readings and record the max and min values.
	DWORD i, max=0, min=512;	
	for(i = 0; i< 2000;i++)
	{
		DWORD val = BrickPi.Sensor[LS_PORT];
		if(val>max) max = val;
		if(val<min) min = val;		
		usleep(2000);
	}
	dwMotorSteer = 0;
	// calculate the mid point.
	dwMidValue = (max+min)/2;
	printf("Max:%d Min:%d Mid:%d\r\n",max,min,dwMidValue);

}

void updateDrive()
{
	// take current sensor value.
	long lCurval = BrickPi.Sensor[LS_PORT];
	// calculate difference from mid point.
	DWORD diff = dwMidValue - lCurval; 
	// adjust steering.
	dwMotorSteer = diff;
	// adjust speed.
	dwMotorPower = 40;
	printf("lCurval:%d diff:%d dwMotorSteer:%d dwMotorPower:%d\r\n",lCurval,diff,dwMotorSteer,dwMotorPower);
	
}

DWORD main(DWORD argc, char argv[])
{
	Setup();
  
	CalibrateSensors();
	while(!bShutdown)
	{
		updateDrive();
		usleep(5000);
	}
  
	Shutdown();
	return 0;

}
