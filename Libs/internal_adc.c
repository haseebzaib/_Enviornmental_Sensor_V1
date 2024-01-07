/*
 * internal_adc.c
 *
 *  Created on: Dec 24, 2023
 *      Author: hzaib
 */


/************************************************************************/
/* Program:    Enviornmental_Sensor    Date: Dec 24, 2023               */
/* Version:     V1                                                      */
/* Autor: Haseeb Zaib                                                   */
/* This file handles internal ADC functions to measure batt voltage		*/
/************************************************************************/
#include "main.h"
#include "internal_adc.h"
#include "app_main.h"


#define adc_Start()	   HAL_ADC_Start(&hadc1)
#define adc_Stop()	    HAL_ADC_Stop(&hadc1)
#define adc_Disable() HAL_ADC_DeInit(&hadc1)

static void adc_InitOneShotMode ()
{
	adc_Disable();

	 ADC_MultiModeTypeDef multimode = {0};
	  hadc1.Instance = ADC1;
	  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
	  hadc1.Init.ScanConvMode = DISABLE;
	  hadc1.Init.ContinuousConvMode = DISABLE;
	  hadc1.Init.DiscontinuousConvMode = DISABLE;
	  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	  hadc1.Init.NbrOfConversion = 1;
	  hadc1.Init.DMAContinuousRequests = DISABLE;
	  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	  if (HAL_ADC_Init(&hadc1) != HAL_OK)
	  {
	    Error_Handler();
	  }


	  /** Configure the ADC multi-mode
	  */
	  multimode.Mode = ADC_MODE_INDEPENDENT;
	  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
	  {
	    Error_Handler();
	  }

		ADC_ChannelConfTypeDef sConfig = {0};
			  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
			  */
		sConfig.Channel = ADC_CHANNEL_10;
		  sConfig.Rank = 1;
		  sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES;
		  sConfig.Offset = 0;
		  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
		  {
		    Error_Handler();
		  }



}

static uint32_t adc_Getval ()
{

	return HAL_ADC_GetValue(&hadc1);

}

uint8_t adc_Measure (float *bat_volt)
{
	uint32_t val;
	adc_InitOneShotMode();
	HAL_Delay(1);
	adc_Start();
	HAL_Delay(1);

	if (HAL_ADC_PollForConversion(&hadc1, 1000) != HAL_OK)
		{
		   HAL_Delay(1);
		   adc_Start();
			if (HAL_ADC_PollForConversion(&hadc1, 1000) != HAL_OK)
			{
				HAL_Delay(1);
				adc_Start();
				if (HAL_ADC_PollForConversion(&hadc1, 1000) != HAL_OK)
				{
					return 0;
				}
			}
		}

	adc_Stop();

	val = adc_Getval();

	HAL_Delay(1);
	adc_Start();
	HAL_Delay(1);

	if (HAL_ADC_PollForConversion(&hadc1, 1000) != HAL_OK)
		{
		   HAL_Delay(1);
		   adc_Start();
			if (HAL_ADC_PollForConversion(&hadc1, 1000) != HAL_OK)
			{
				HAL_Delay(1);
				adc_Start();
				if (HAL_ADC_PollForConversion(&hadc1, 1000) != HAL_OK)
				{
					return 0;
				}
			}
		}

	adc_Stop();

	val =(uint32_t)((val + adc_Getval()) / 2);	//Read and average ADC


	// Max = 3.3Vref * 365k / 100k = 12.045V

	*bat_volt = (float) val * (12.045f / 4096.0f);	// 12 bit


	return 1;


}
