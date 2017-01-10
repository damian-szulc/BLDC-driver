/**
  ******************************************************************************
  * File Name          : main.h
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define Button_Pin GPIO_PIN_13
#define Button_GPIO_Port GPIOC
#define CurrFdbC_Pin GPIO_PIN_0
#define CurrFdbC_GPIO_Port GPIOC
#define CurrFdbB_Pin GPIO_PIN_1
#define CurrFdbB_GPIO_Port GPIOC
#define BemfFdb1_Pin GPIO_PIN_3
#define BemfFdb1_GPIO_Port GPIOC
#define CurrFdbA_Pin GPIO_PIN_0
#define CurrFdbA_GPIO_Port GPIOA
#define BUSV_Pin GPIO_PIN_1
#define BUSV_GPIO_Port GPIOA
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define DIAGEN_Pin GPIO_PIN_6
#define DIAGEN_GPIO_Port GPIOA
#define BemfFdb3_Pin GPIO_PIN_7
#define BemfFdb3_GPIO_Port GPIOA
#define BemfFdb2_Pin GPIO_PIN_0
#define BemfFdb2_GPIO_Port GPIOB
#define POTENTIOMETER_Pin GPIO_PIN_1
#define POTENTIOMETER_GPIO_Port GPIOB
#define LEDRED_Pin GPIO_PIN_2
#define LEDRED_GPIO_Port GPIOB
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOB
#define TEST2_Pin GPIO_PIN_6
#define TEST2_GPIO_Port GPIOC
#define TEST_Pin GPIO_PIN_8
#define TEST_GPIO_Port GPIOC
#define IN1_Pin GPIO_PIN_8
#define IN1_GPIO_Port GPIOA
#define IN2_Pin GPIO_PIN_9
#define IN2_GPIO_Port GPIOA
#define IN3_Pin GPIO_PIN_10
#define IN3_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define EN1_Pin GPIO_PIN_10
#define EN1_GPIO_Port GPIOC
#define EN2_Pin GPIO_PIN_11
#define EN2_GPIO_Port GPIOC
#define EN3_Pin GPIO_PIN_12
#define EN3_GPIO_Port GPIOC
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define CurrRef_Pin GPIO_PIN_4
#define CurrRef_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
