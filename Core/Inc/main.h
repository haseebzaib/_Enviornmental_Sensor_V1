/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
extern SD_HandleTypeDef hsd;
extern RTC_HandleTypeDef hrtc;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern I2C_HandleTypeDef hi2c1;
extern TIM_HandleTypeDef htim2;
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;
extern DMA_HandleTypeDef hdma_tim1_up;

#define PACKED __attribute__((packed))

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);
void MX_SDIO_SD_Init(void);
void MX_TIM2_Init(void);
void MX_TIM1_Init(void);
void MX_TIM3_Init(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BATT_ADC_IN10_Pin GPIO_PIN_0
#define BATT_ADC_IN10_GPIO_Port GPIOC
#define OUT_MOTION_Pin GPIO_PIN_0
#define OUT_MOTION_GPIO_Port GPIOA
#define EN_MOTION_Pin GPIO_PIN_1
#define EN_MOTION_GPIO_Port GPIOA
#define PM_USART2_TX_Pin GPIO_PIN_2
#define PM_USART2_TX_GPIO_Port GPIOA
#define PM_USART2_RX_Pin GPIO_PIN_3
#define PM_USART2_RX_GPIO_Port GPIOA
#define EN_5V_Pin GPIO_PIN_15
#define EN_5V_GPIO_Port GPIOB
#define USB_DETECT_Pin GPIO_PIN_9
#define USB_DETECT_GPIO_Port GPIOA
#define USB_DETECT_EXTI_IRQn EXTI9_5_IRQn
#define SW_DET_Pin GPIO_PIN_15
#define SW_DET_GPIO_Port GPIOA
#define SW_DET_EXTI_IRQn EXTI15_10_IRQn
#define BLUE_LED_Pin GPIO_PIN_3
#define BLUE_LED_GPIO_Port GPIOB
#define GREEN_LED_Pin GPIO_PIN_4
#define GREEN_LED_GPIO_Port GPIOB
#define RED_LED_Pin GPIO_PIN_5
#define RED_LED_GPIO_Port GPIOB
#define CLI_UART1_TX_Pin GPIO_PIN_6
#define CLI_UART1_TX_GPIO_Port GPIOB
#define CLI_UART1_RX_Pin GPIO_PIN_7
#define CLI_UART1_RX_GPIO_Port GPIOB
#define SCD4x_SCL_Pin GPIO_PIN_8
#define SCD4x_SCL_GPIO_Port GPIOB
#define SCD4x_SDA_Pin GPIO_PIN_9
#define SCD4x_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
