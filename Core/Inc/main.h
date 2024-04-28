/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

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

/* USER CODE BEGIN EFP */
uint32_t keyboard_read(void);
uint32_t keyboard_get_key(uint32_t state);
void buzzer_play(uint32_t freq, uint32_t volume);
void buzzer_stop(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define KBD_A_Pin GPIO_PIN_12
#define KBD_A_GPIO_Port GPIOB
#define KBD_B_Pin GPIO_PIN_13
#define KBD_B_GPIO_Port GPIOB
#define KBD_C_Pin GPIO_PIN_14
#define KBD_C_GPIO_Port GPIOB
#define KBD_D_Pin GPIO_PIN_15
#define KBD_D_GPIO_Port GPIOB
#define KBD_1_Pin GPIO_PIN_3
#define KBD_1_GPIO_Port GPIOB
#define KBD_2_Pin GPIO_PIN_4
#define KBD_2_GPIO_Port GPIOB
#define KBD_3_Pin GPIO_PIN_5
#define KBD_3_GPIO_Port GPIOB
#define KBD_4_Pin GPIO_PIN_6
#define KBD_4_GPIO_Port GPIOB
#define KBD_5_Pin GPIO_PIN_7
#define KBD_5_GPIO_Port GPIOB
#define KBD_6_Pin GPIO_PIN_8
#define KBD_6_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
