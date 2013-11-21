#include <stdio.h>
#include <pthread.h>

// some useful defines that I am going to use

#define DWORD int
#define BOOL char
#define TRUE 1
#define FALSE 0 

#include "../tick.h"
#include "../BrickPi.h"

#define MOTORR	PORT_A
#define MOTORL	PORT_B
#define MOTORNULL PORT_C
#define MOTORNULL PORT_D

#define US_PORT PORT_1
#define CS_PORT PORT_2
#define KS_PORT PORT_3
#define NULL_PORT PORT_4

#define TURN_SCALE 20
#define POWERSCALE 40

#define threadResult(tRes) if(tRes == 0) {dwRunningThreads++;} else {bShutdown = TRUE;}


BOOL bShutdown = FALSE;
DWORD dwRunningThreads = 0;
DWORD dwMotorPower = 0;
DWORD dwMotorSteer = 0;
DWORD dwMidValue;
pthread_t tMovement, tKillSwitch, tSensor;


void* MotorControl(void* args)
{
  
  BrickPi.MotorEnable[MOTORL] = 1;
  BrickPi.MotorEnable[MOTORR] = 1;
	
  while(!bShutdown)
  {
	BrickPi.MotorSpeed[MOTORL] = (dwMotorPower*POWERSCALE) + (dwMotorSteer*TURN_SCALE);
	BrickPi.MotorSpeed[MOTORR] = (dwMotorPower*POWERSCALE) - (dwMotorSteer*TURN_SCALE);
	usleep(250);	
  }
 
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
	//	printf("Light Sensor: %d\r\n",BrickPi.Sensor[CS_PORT]);
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
	BrickPi.SensorType[CS_PORT] = TYPE_SENSOR_COLOR_NONE;
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
	BrickPi.SensorType[US_PORT] = TYPE_SENSOR_RAW;
	BrickPi.SensorType[CS_PORT] = TYPE_SENSOR_RAW;
	BrickPi.SensorType[KS_PORT] = TYPE_SENSOR_RAW;
	BrickPi.SensorType[NULL_PORT] = TYPE_SENSOR_RAW;
	BrickPiSetupSensors();
	while(dwRunningThreads)
	{
		printf("Threads Running: %d\r\n",dwRunningThreads);
		sleep(1);
	}
}

void CalibrateSensors()
{

	// rotate swing the wheels back to the other side.
	dwMotorSteer = 128;
	// while that is happening take sensor readings and record the max and min values.
	DWORD i, max=0, min=512;	
	for(i = 0; i< 2000;i++)
	{
		DWORD val = BrickPi.Sensor[CS_PORT];
		if(val>max) max = val;
		if(val<min) min = val;		
		usleep(2000);
	}
	dwMotorSteer = 0;
	// calculate the mid point.
	dwMidValue = (max+min)/2;
	printf("Max:%d Min:%d Mid:%d\r\n",max,min,dwMidValue);

}
DWORD main(DWORD argc, char argv[])
{
	Setup();

	//CalibrateSensors();
	dwMotorPower = 100;
	while(!bShutdown)
	{
		if (BrickPi.Sensor[US_PORT] < 20)
		{
			dwMotorPower = 100;
			dwMotorSteer = -40;
			sleep(2);
			dwMotorPower = 100;
			dwMotorSteer = 0;

		}
		printf("Senosr Val=%d\r\n",BrickPi.Sensor[US_PORT]);
		usleep(2000);

	}

	Shutdown();
	return 0;

}
