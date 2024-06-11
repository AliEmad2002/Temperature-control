/*
 * main_template.c
 *
 *  Created on: Sep 20, 2023
 *      Author: Ali Emad
 */

#if 1
/*	RTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

/*	LIB	*/
#include <stdint.h>
#include <string.h>
#include "LIB/HysteresisFilter/HysteresisFilter.h"
#include "LIB/LowPassFilter/LowPassFilter.h"
//#include "LIB/PID/PID.h"

/*	MCAL (Ported)	*/
#include "MCAL_Port/Port_HW.h"
#include "MCAL_Port/Port_Clock.h"
#include "MCAL_Port/Port_Interrupt.h"
#include "MCAL_Port/Port_ADC.h"

/*	HAL_OS	*/
#include "HAL/HAL_OS.h"


/*******************************************************************************
 * Tasks stacks and handles (initialization task and main tasks):
 ******************************************************************************/
static StackType_t puxTask1Stack[128];
static StaticTask_t xTask1StaticHandle;
static TaskHandle_t xTask1Handle;

static StackType_t puxTask2Stack[128];
static StaticTask_t xTask2StaticHandle;
static TaskHandle_t xTask2Handle;

static StackType_t puxTask3Stack[128];
static StaticTask_t xTask3StaticHandle;
static TaskHandle_t xTask3Handle;

static StackType_t puxTask4Stack[128];
static StaticTask_t xTask4StaticHandle;
static TaskHandle_t xTask4Handle;

static StackType_t puxTaskInitStack[200];
static StaticTask_t xTaskInitStaticHandle;
static TaskHandle_t xTaskInitHandle;


/*******************************************************************************
 * Function prototypes:
 ******************************************************************************/
static void init_main_tasks(void);

static float fGetTempRaw(void);

static void vRotaryEncoderChangeCallback(void* pvParams);

/*******************************************************************************
 * Helping macros:
 ******************************************************************************/
#define iINITIAL_SET_POINT		120

/*******************************************************************************
 * Global variables:
 ******************************************************************************/
xHOS_AnalogLinearTemperatureSensor_t xIntenalTemperatureSensor = {
		.ucAdcUnitNumber = ucPORT_ADC_TEMP_SENS_INT_UNIT_NUMBER,
		.ucAdcChannelNumber = ucPORT_ADC_TEMP_SENS_INT_CH_NUMBER,
		.iA = iPORT_ADC_TEMP_SENS_A,
		.iB = iPORT_ADC_TEMP_SENS_B,
		.iC = iPORT_ADC_TEMP_SENS_C
};

const uint8_t pucCtrlPortArr[] = {1, 1, 1};
const uint8_t pucCtrlPinArr[] = {0, 10, 11};

xHOS_CalibratableAmplifier_t xAmplifier = {
	.ucAdcUnitNumber = 0,
	.ucAdcChannelNumber = 5,

	.iGainUVPV = 33000000,

	.pucCtrlPortArr = (uint8_t*)pucCtrlPortArr,
	.pucCtrlPinArr = (uint8_t*)pucCtrlPinArr,
	.ucNumberOfCtrlPins = 3,

	.ucAmpOnCtrlHighStateMask = (0 << 0) | (0 << 1) | (0 << 2),
	.ucAmpOnCtrlLowStateMask =  (0 << 0) | (1 << 1) | (1 << 2),

	.ucAmpCalibCtrlHighStateMask = (0 << 0) | (0 << 1) | (0 << 2),
	.ucAmpCalibCtrlLowStateMask =  (1 << 0) | (0 << 1) | (1 << 2),

	.uiCalibTimePeriodMs = 2000,
	.uiSwitchTimeMs = 10,
	.uiCalibAvgTimeMs = 10
};

xHOS_Thermocouple_t xThermocouple = {
	.pxAmplifier = &xAmplifier,
	.pxColdJunctionTemperatureSensor = &xIntenalTemperatureSensor
};

xLIB_HysteresisFilter_t xHystFilter;

xLIB_LowPassFilter_t xLPF = {
		.pfGetSample = fGetTempRaw,
		.uiSampleTimeMs = 10
};

xLIB_LowPassFilter_t xDisplayLPF = {
		.pfGetSample = fGetTempRaw,
		.uiSampleTimeMs = 100
};

xHOS_Relay_t xRelay = {
		.uiProtectionDelayMs = 1000,
		.ucPort = 1,
		.ucPin = 7,
		.ucActiveState = 0
};

uint8_t pucSegPortArr[] =	{0, 0, 1 , 1 , 1 , 1 , 0, 1};
uint8_t pucSegPinArr[] =	{0, 2, 15, 13, 14, 12, 1, 1};

uint8_t pucSSEnPortArr[] =	{2 , 2 , 0};
uint8_t pucSSEnPinArr[] =	{15, 14, 3};

uint8_t pucSSMMem[3];

xHOS_SevenSegmentMux_t xSSM = {
		.pxSegmentPortNumberArr = pucSegPortArr,
		.pxSegmentPinNumberArr = pucSegPinArr,
		.pxDigitEnablePortNumberArr = pucSSEnPortArr,
		.pxDigitEnablePinNumberArr = pucSSEnPinArr,
		.ucSegmentActiveLevel = 0,
		.ucEnableActiveLevel = 1,
		.ucNumberOfDigits = 3,
		.pucDisplayBuffer = pucSSMMem,
		.cPointIndex = -1,
		.uiUpdatePeriodMs = 5
};

xHOS_RotaryEncoder_t xRotaryEncoder = {
		.ucAPort = 1,
		.ucAPin = 8,
		.ucBPort = 1,
		.ucBPin = 9,
		.uiSamplePeriodMs = 1,
		.uiSpeedUpdatePeriodMs = 1,
		.uiSpeedDeadTimeMs = 10,
		.ucEnableSpeedUpdate = 1,
		.ucEnableCWCallback = 1,
		.ucEnableCCWCallback = 1,
		.pfCWCallback = vRotaryEncoderChangeCallback,
		.pfCCWCallback = vRotaryEncoderChangeCallback,
		.pvCWParams = NULL,
		.pvCCWParams = NULL
};

SemaphoreHandle_t xRotaryMovedSemphr;
StaticSemaphore_t xRotaryMovedSemphrStatic;

volatile int32_t iTTC;
volatile int32_t iT;
volatile int32_t iTSP = iINITIAL_SET_POINT;
volatile int32_t iHystTolerance = 1;
volatile uint8_t ucDisplayMode = 0;


/*******************************************************************************
 * Task functions:
 ******************************************************************************/
static void vInitTask(void* pvParams)
{
	/*	Initialize MCU's hardware	*/
	vPort_HW_init();

	/*	Startup delay	*/
	vTaskDelay(pdMS_TO_TICKS(100));

	/*	Initialize USB CDC	*/
	vHOS_UsbCdc_init();

	/*	Initialize ADC HAL driver	*/
	vHOS_ADC_init();

	/*	Initialize internal temperature sensor	*/
	vHOS_AnalogLinearTemperatureSensor_init(&xIntenalTemperatureSensor, 171);

	/*	Initialize amplifier	*/
	vHOS_CalibratableAmplifier_init(&xAmplifier, 10000);

	/*	Initialize thermocouple	*/
	vHOS_Thermocouple_init(&xThermocouple);

	/*	Initialize hysteresis filter	*/
	vLIB_HysteresisFilter_init(&xHystFilter);

	/*	Initialize low pass filters	*/
	vLIB_LowPassFilter_init(&xLPF, 31, 1);
	vLIB_LowPassFilter_init(&xDisplayLPF, 0.3, 1);

	/*	Initialize relay	*/
	vHOS_Relay_init(&xRelay);

	/*	Initialize display	*/
	vHOS_SevenSegmentMux_init(&xSSM);

	/*	Initialize rotary encoder	*/
	vHOS_RotaryEncoder_init(&xRotaryEncoder, 3);
	xRotaryEncoder.iPos = iINITIAL_SET_POINT;

	xRotaryMovedSemphr = xSemaphoreCreateBinaryStatic(&xRotaryMovedSemphrStatic);
	xSemaphoreTake(xRotaryMovedSemphr, 0);

	/*	Initialize main tasks	*/
	init_main_tasks();

	vTaskSuspend(xTaskInitHandle);
	while(1);
}

/*	Controls relay	*/
static void vMainTask1(void* pvParams)
{
	while(!xAmplifier.ucIsInitialCalibDone);

	ucHOS_Relay_lock(&xRelay, portMAX_DELAY);

	while(1)
	{
		/*	Read thermocouple (Raw)	*/
		if (!ucHOS_Thermocouple_getTemperature(&xThermocouple, (int32_t*)&iT))
			iT = xThermocouple.uiTLPrev * 1000;

		/*	Read thermocouple (Filtered)	*/
		iTTC = xLPF.fOutput;

		/*	Update hysteresis filter	*/
		xHystFilter.iLowThreshold = iTSP - iHystTolerance;
		xHystFilter.iHighThreshold = iTSP + iHystTolerance;
		vLIB_HysteresisFilter_update(&xHystFilter, iTTC);

		/*	Update output	*/
		if (xHystFilter.ucCurrentOutputState)
			vHOS_Relay_switch(&xRelay, 1);
		else
			vHOS_Relay_switch(&xRelay, 0);
	}
}

/*	Controls display	*/
static void vMainTask2(void* pvParams)
{
	vHOS_SevenSegmentMux_Enable(&xSSM);

	while(1)
	{
		/*	If display mode is normal	*/
		if (ucDisplayMode == 0)
		{
			/*	Update display with measured temperature	*/
			vHOS_SevenSegmentMux_write(&xSSM, (int32_t)xDisplayLPF.fOutput, -1);
			
			/*	Ensure display is on	*/
			vHOS_SevenSegmentMux_Enable(&xSSM);
		}

		/*	Otherwise, if display mode is "set point" mode	*/
		else
		{
			/*	Update display with set point temperature	*/
			vHOS_SevenSegmentMux_write(&xSSM, iTSP, -1);

			/*	Toggle the display	*/
			if (xSSM.ucIsEnabled)
				vHOS_SevenSegmentMux_Disable(&xSSM);
			else
				vHOS_SevenSegmentMux_Enable(&xSSM);
		}

		/*	Delay until next period	*/
		vTaskDelay(pdMS_TO_TICKS(250));
	}
}

/*	Controls display mode	*/
static void vMainTask3(void* pvParams)
{
	vHOS_RotaryEncoder_enable(&xRotaryEncoder);

	while(1)
	{
		/*	Block until rotary encoder is moved	*/
		xSemaphoreTake(xRotaryMovedSemphr, portMAX_DELAY);

		/*	Change mode to "set point" mode	*/
		ucDisplayMode = 1;

		/*	Wait for one second	*/
		vTaskDelay(pdMS_TO_TICKS(1000));

		/*	Change mode to normal mode	*/
		ucDisplayMode = 0;
	}
}

/*	Logs device's data	*/
static void vMainTask4(void* pvParams)
{
	TickType_t xLastWkpTime = xTaskGetTickCount();

//	char str[10];
	uint8_t ucSOF = 0x03;
	uint8_t ucEOF = 0xFC;

	/*	Lock USB CDC 0	*/
	ucHOS_UsbCdc_lockTransmission(0, portMAX_DELAY);

	while(1)
	{
		/*	print temperature and relay state	*/
//		sprintf(str, "%d, %d\r\n", (int)iTTC, (int)xHystFilter.ucCurrentOutputState);

		/*	Send	*/
//		vHOS_UsbCdc_send(0, (uint8_t*)str, strlen(str));

		vHOS_UsbCdc_send(0, &ucSOF, sizeof(uint8_t));
		vHOS_UsbCdc_send(0, (uint8_t*)&iTTC, sizeof(int32_t));
		vHOS_UsbCdc_send(0, &(xHystFilter.ucCurrentOutputState), sizeof(uint8_t));
		vHOS_UsbCdc_send(0, &ucEOF, sizeof(uint8_t));

//		vHOS_UsbCdc_send(0, (uint8_t*)&iTTC, sizeof(int32_t));

//		/*	Send relay state	*/
//		vHOS_UsbCdc_send(0, &(xHystFilter.ucCurrentOutputState), sizeof(uint8_t));

		/*	Delay until next sample	*/
		vTaskDelayUntil(&xLastWkpTime, pdMS_TO_TICKS(100));
	}
}

/*******************************************************************************
 * Callbacks:
 ******************************************************************************/
void Error_Handler(void)
{
	while(1);
}

void vApplicationIdleHook( void )
{
   for( ;; )
   {

   }
}

static float fGetTempRaw(void)
{
	return (float)iT / 1000;
}

static void vRotaryEncoderChangeCallback(void* pvParams)
{
	/*	Update set point	*/
	if (xRotaryEncoder.iPos < 0)
	{
		xRotaryEncoder.iPos = 0;
		iTSP = 0;
	}
	else
		iTSP = xRotaryEncoder.iPos;

	/*	Acknowledge display control task	*/
	xSemaphoreGive(xRotaryMovedSemphr);
}


/*******************************************************************************
 * Override HAL weak functions (To work properly with the RTOS):
 ******************************************************************************/
void HAL_Delay(uint32_t Delay)
{
	vTaskDelay(pdMS_TO_TICKS(Delay));
}

/*******************************************************************************
 * Tasks initialization (initialization task and main tasks):
 ******************************************************************************/
static void init_initialization_task(void)
{
	xTaskInitHandle = xTaskCreateStatic(
		vInitTask,
		"initTask",
		200,
		NULL,
		configHOS_INIT_REAL_TIME_TASK_PRI,
		puxTaskInitStack,
		&xTaskInitStaticHandle);
}

static void init_main_tasks(void)
{
	xTask1Handle = xTaskCreateStatic(
		vMainTask1,
		"mainTask1",
		128,
		NULL,
		configHOS_SOFT_REAL_TIME_TASK_PRI,
		puxTask1Stack,
		&xTask1StaticHandle	);

	xTask2Handle = xTaskCreateStatic(
		vMainTask2,
		"mainTask2",
		128,
		NULL,
		configHOS_SOFT_REAL_TIME_TASK_PRI,
		puxTask2Stack,
		&xTask2StaticHandle	);

	xTask3Handle = xTaskCreateStatic(
		vMainTask3,
		"mainTask3",
		128,
		NULL,
		configHOS_SOFT_REAL_TIME_TASK_PRI,
		puxTask3Stack,
		&xTask3StaticHandle	);

	xTask4Handle = xTaskCreateStatic(
		vMainTask4,
		"mainTask4",
		128,
		NULL,
		configHOS_SOFT_REAL_TIME_TASK_PRI,
		puxTask4Stack,
		&xTask4StaticHandle	);
}

/*******************************************************************************
 * Main:
 ******************************************************************************/
int main(void)
{
	vPORT_INTERRUPT_DISABLE_GLOBAL_INTERRUPT();

	/*	Initialize clock	*/
	vPort_Clock_initCpuClock();

	/*	Initialize initialization task	*/
	init_initialization_task();

	/*	Run scheduler	*/
	vTaskStartScheduler();

	vPORT_INTERRUPT_ENABLE_GLOBAL_INTERRUPT();

	while(1)
	{

	}

	return 0;
}

#endif
