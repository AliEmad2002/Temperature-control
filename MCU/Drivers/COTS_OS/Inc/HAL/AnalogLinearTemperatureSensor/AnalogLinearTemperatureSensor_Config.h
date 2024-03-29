/*
 * LM35_Config.h
 *
 *  Created on: Dec 28, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_HAL_ANALOGLINEARTEMPERATURESENSOR_ANALOGLINEARTEMPERATURESENSOR_CONFIG_H_
#define COTS_OS_INC_HAL_ANALOGLINEARTEMPERATURESENSOR_ANALOGLINEARTEMPERATURESENSOR_CONFIG_H_


/*
 * Selects whether ADC readings of the sensor has to be calibrated using internal
 * reference voltage, or just use the defined reference voltage "uiPORT_ADC_VREF_IN_MV".
 */
#define ucHOS_CONF_ANALOG_LINEAR_TEMPERATURE_SENSOR_ENABLE_CALIB_ADC		1

#endif /* COTS_OS_INC_HAL_ANALOGLINEARTEMPERATURESENSOR_ANALOGLINEARTEMPERATURESENSOR_CONFIG_H_ */
