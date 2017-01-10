/*
 * comm.c
 *
 *  Created on: 25 paź 2016
 *      Author: root
 */

#include "main.h"
#include "stm32f3xx_hal.h"
#include "tim.h"
#include "gpio.h"
#include "usart.h"

#include "commands.h"
#include "comm.h"
#include "driver_bldc1.h"
#include <string.h>
uint8_t transmissionSettled = 0;

uint8_t sendingBuffor[40];
/**
 *
 * @brief  Inicjalizacja obsługi komunikacji
 * @param  None
 * @retval None
 */
void commInit() {
	//inicjalizacja komunikacji UART
	commUARTInit();
}

/**************************************LED*****************************************/
/**
 * @brief  Włączenie sygnalizacji ledem
 * @param  None
 * @retval None
 */
void commLedSet(uint8_t type) {
	commLedReset();
	//0 - stale załączone
	if (type == 0) {
		//po prostu załączenie diody
		HAL_GPIO_WritePin(LEDRED_GPIO_Port, LEDRED_Pin, GPIO_PIN_SET);
	}
}

/**
 * @brief  Wyłączanie sygnalizacji led
 * @param  None
 * @retval None
 */
void commLedReset() {
	HAL_GPIO_WritePin(LEDRED_GPIO_Port, LEDRED_Pin, GPIO_PIN_RESET);
}

/**************************************UART*****************************************/

/**
 *
 * @brief  Inicjalizacja komunikacji UART
 * @param  None
 * @retval None
 */
void commUARTInit() {
	//nasłuchiwanie na przychodzące wiadomości
	HAL_UART_Receive_IT(&huart2, commUARTRecived, COMMUARTTRANLEN);
}

/**
 *
 * @brief  Przerwanie od UART - otrzymanie wiadomości
 * @param  None
 * @retval None
 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	//odczytaj i obsłuż wiadomość
	commUARTHandleRecivedMsg();
	// Ponowne włączenie nasłuchiwania
	commUARTInit();
}
/**
 *
 * @brief  Wysyłanie wiadomości UART, max 40 znaków
 * @param  None
 * @retval None
 */
void commUARTSent(uint8_t msg) {
	if (transmissionSettled != 1)
		return;

	//wysłanie
	sendingBuffor[0] = msg;
	sendingBuffor[1] = CMD_SPACER;
	sendingBuffor[2] = CMD_SPACER;
	sendingBuffor[3] = CMD_SPACER;
	sendingBuffor[4] = CMD_SPACER;
	HAL_UART_Transmit_IT(&huart2, sendingBuffor, 5);
}
/**
 *
 * @brief  Wysyłanie wiadomości UART, max 40 znaków
 * @param  None
 * @retval None
 */
void commUARTSent2(uint8_t msg, uint8_t msg2) {
	if (transmissionSettled != 1)
		return;

	//wysłanie
	//uint8_t size = sprintf(sendingBuffor, "%s", CMD_HELLO);
	sendingBuffor[0] = msg;
	sendingBuffor[1] = msg2;
	sendingBuffor[2] = CMD_SPACER;
	sendingBuffor[3] = CMD_SPACER;
	sendingBuffor[4] = CMD_SPACER;
	HAL_UART_Transmit_IT(&huart2, sendingBuffor, 5);
}
/**
 *
 * @brief  Wysyłanie pomiarów poprzez UART
 * @param  None
 * @retval None
 */
void commUARTdata(uint8_t msg, uint16_t data, uint16_t data2) {
	if (transmissionSettled != 1)
		return;

	//wysłanie
	//uint8_t size = sprintf(sendingBuffor, "%s", CMD_HELLO);
	sendingBuffor[0] = msg;
	sendingBuffor[1] = data >> 8;
	sendingBuffor[2] = data & 0xFF;
	sendingBuffor[3] = data2 >> 8;
	sendingBuffor[4] = data2 & 0xFF;

	HAL_UART_Transmit_IT(&huart2, sendingBuffor, 5);
}
/**
 *
 * @brief  Wysyłanie pomiarów poprzez UART
 * @param  None
 * @retval None
 */
uint16_t getValueFromRcvData(char a, char b) {
	uint16_t lo = b;
	uint16_t hi = a << 8;
	uint16_t rtn = lo | hi;

	return rtn;
}
/**
 *
 * @brief  Zareaguj na otrzymaną instrukcję
 * @param  None
 * @retval None
 */

void commUARTHandleRecivedMsg() {
	//nawiazywanie transmisji
	if (transmissionSettled == 0) {
		if (commUARTRecived[0] == CMD_HELLO) {
			transmissionSettled = 1;
			commUARTSent2(CMD_HELLO, CMD_TRYB_OL);
			return;
		} else
			return;
	}

	if (commUARTRecived[0] == CMD_STER) {
		//start silnika
		if (commUARTRecived[1] == CMD_START) {
			if (bldcMotorState == 0) {

				uint16_t val = commUARTRecived[4]
				                               | (((uint16_t) commUARTRecived[3]) << 8);
				zadaneWypelnienie = val;
				bldcStart();
			}

		}
		//stop silnika
		else if (commUARTRecived[1] == CMD_STOP)
			bldcStop();
	}
	//intrukcje wyboru parametrów pracy
	else if (commUARTRecived[0] == CMD_PARAM) {

		uint16_t val = commUARTRecived[4]
		                               | (((uint16_t) commUARTRecived[3]) << 8);

		//zmiana prametrów prędkości, ew wypelnienia
		if (commUARTRecived[1] == CMD_PRACA) {
			if (commUARTRecived[2] == CMD_PRACA_WYP && val < 500) {
				zadaneWypelnienie = val;
				if (bldcMotorState == 4)
					setDutyCycle(val);
			}
		}
		//zmiana parametrów zerowania
		else if (commUARTRecived[1] == CMD_ZER) {
			//zmiana czasu
			if (commUARTRecived[2] == CMD_ZER_T)
				positioningTime = val;
			//zmiana wypelnienia
			else if (commUARTRecived[2] == CMD_ZER_D)
				positioningDutyCycle = val;
		}
		//zmiana parametrów rozruchu w otwartej pętli
		else if (commUARTRecived[1] == CMD_ROZR_OTW) {
			//zmiana czasu
			if (commUARTRecived[2] == CMD_ROZR_OTW_T)
				bldcStartOLtime = val;
			//zmiana fmin
			else if (commUARTRecived[2] == CMD_ROZR_OTW_Fmin)
				bldcStartOLfmin = val;
			//zmiana fmax
			else if (commUARTRecived[2] == CMD_ROZR_OTW_Fmax)
				bldcStartOLfmax = val;
			//zmiana dmin
			else if (commUARTRecived[2] == CMD_ROZR_OTW_Dmin)
				bldcStartOLdmin = val;
			//zmiana dmax
			else if (commUARTRecived[2] == CMD_ROZR_OTW_Dmax)
				bldcStartOLdmax = val;
		}
		//zmiana parametrów rozruchu w otwartej pętli
		else if (commUARTRecived[1] == CMD_RAMPA)
			rampaScale = val;


	}
}

/*************************************BUTTON*****************************************/
/**
 *
 * @brief  Naciśnięcie przycisku
 * @param  None
 * @retval None
 */
void commButtonClicked() {
	//jeżeli silnik jest zatrzymany i nie ustanowiono połączenia to możesz go
	if (bldcMotorState == 0 && transmissionSettled == 0)
		bldcStart();
	else if (bldcMotorState != 0)
		bldcStop();
}
/**
 *
 * @brief  Przerwanie od naciśnięcia przycisku BUTTON
 * @param  None
 * @retval None
 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == Button_Pin) {
		commButtonClicked();
	}
}
