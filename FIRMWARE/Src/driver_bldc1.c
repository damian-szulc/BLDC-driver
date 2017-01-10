/*
 * driver_bldc1.c
 *
 *  Created on: 25 paź 2016
 *      Author: root
 */
#include "main.h"
#include "stm32f3xx_hal.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

#include "comm.h"
#include "commands.h"
#include "driver_bldc1.h"
/***********************************STEROWANIE********************************/
//stan pracy silnika 0-wyłączony, 1-zerowanie, 2-rozruch, 3-praca w układzie otwartym, 4-praca bezczujnikowa
uint8_t bldcMotorState = 0;

//krok pracy w danym cyklu
volatile uint8_t bldcMotorLastStp = 0;
//zadawanie prędkości poprzez potencjometr
uint8_t potentiometrCurrentControl = 1;
//stany na wyjściach EN w poszczególnych krokach
uint8_t DB1EN1States[] = { 1, 1, 0, 1, 1, 0 };
uint8_t DB1EN2States[] = { 1, 0, 1, 1, 0, 1 };
uint8_t DB1EN3States[] = { 0, 1, 1, 0, 1, 1 };
/**********************************ZEROWANIE************************************/
//flaga
uint16_t positioningStepsCounter = 0;
//czas trwania zerowania, ms
uint16_t positioningTime = 200;
//wypelenienie przy zerowaniu, 0 - 499
uint16_t positioningDutyCycle = 499;

/*******************PARAMETRY ROZRUCHU W OTWARTEJ PĘTLI***********************/
//minimalna częstotliwość podczas rozruchu w otwartej pętli, Hz
uint32_t bldcStartOLfmin = 18;
//maksymalna częstotliwość podczas rozruchu w otwartej pętli, Hz
uint32_t bldcStartOLfmax = 300;
//minimalne wypełnienie podczas rozruchu w otwartej pętli, 0-499
uint8_t bldcStartOLdmin = 50;
//maksymalne wypełnienie podczas rozruchu w otwartej pętli, 0-499
uint32_t bldcStartOLdmax = 150;
//czas przyrostu, ms
uint32_t bldcStartOLtime = 800;
uint32_t ZERO_PERIOD = 0;
uint32_t bldcStartOLfdiff = 0;
uint32_t bldcStartOLddiff;
/**************************************KONIEC*********************************/
/**********************************KONWERSJA ADC******************************/
//USTAWIENIA
enum {
	//długoś buforu DMA do detekcji zera
	ADC_BUFFER_LENGTH = 7,
	ADC_CHANNELS = 3,
	//numery kanałów w buforze DMA
	BEMF_A = 0,
	BEMF_B = 1,
	BEMF_C = 2,
	//detekcja przy narastającym czy opadającym BEMF
	BEMF_UP = 0,
	BEMF_DOWN = 1,
	ZERO_DETECTION_OK = 1,
	//próg napięcia do detekcji zera
	ZEROTHRESHOLD = 1,
	NROFTHRESHOLDS = 2,
};
//bufor do zapisu wyników pomiarów
uint16_t g_ADCBuffer[ADC_BUFFER_LENGTH];
//flaga wystawiana po wykryciu zera
volatile uint16_t zeroDetected = 0;
//licznik/zmienna do filtracji pomiarów
uint8_t ADCcnt = 0;

/**********************DETEKCJA ZERA/PRACA BEZCZUJNIKOWEJ*********************/
//flaga wykorzystywana po wykryciu pierwszego zera
uint8_t zeroFlag = 0;
//stan licznika odpowiadający 1/6 cyklu
uint32_t actualPeriod = 0;
//czas (w liczniku) odpowiadający
uint32_t lastRest = 0;

/*************************RAMPA/KONTROLA PRĄDU*******************************/
//WYPEŁNIENIE -> zmieniane w zakresie 0-499
uint16_t wypelnienie = 150;
uint16_t zadaneWypelnienie = 150;
//rampa
uint16_t rampaDCounter = 0;
uint16_t rampaDCounterMax = 0;
//nastawa rampy
uint8_t rampaScale = 3;
//kierunek rampy (góra/dół)
int8_t rampaSign = 0;
/**************************************KONIEC*********************************/
/******************************INNE PARAMETRY*******************************/
//zadane wypelnienie
uint8_t usartTransmitDataRequest = 0;
/**************************************KONIEC*********************************/

/**
 *
 * @brief  Inicjalizacja sterowania
 * @param  None
 * @retval None
 */

void bldcInit() {
	//inicjalizacja przetwornika ADC z DMA
	HAL_ADC_Start_DMA(&hadc1, g_ADCBuffer, ADC_BUFFER_LENGTH);

	//wartość początkowa referencyjnego prądu
	TIM16->CCR1 = 998;

	//start PWM
	HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1);
	TIM2->ARR = 0;
	wypelnienie = 0;

}
/**
 *
 * @brief  Załącz silnik
 * @param  None
 * @retval None
 */
void bldcStart() {
	//reset flag silnika
	bldcMotorState = 0;
	bldcMotorLastStp = 0;
	positioningStepsCounter = 0;
	zeroFlag = 0;

	//commUARTSent(CMD2_START);

	//zmiana flagi stanu silnika na rozruch
	bldcMotorState = 1;

	//początkowe wypełnienie na wyjściach IN1-3
	TIM1->CCR1 = 0;
	TIM1->CCR2 = 0;
	TIM1->CCR3 = 0;
	//uruchomienie PWM na wyjściach IN 1-3
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);

	//sygnalizacja startu
	commLedSet(0);

}
/**
 *
 * @brief  Wyłącz silnik
 * @param  None
 * @retval None
 */

void bldcStop() {

	//wyłączanie timera 2 odpowiedzialnego za przełączanie portów EN0
	HAL_TIM_Base_Stop_IT(&htim2);

	//wyłączenie pinów chip enable - EN 1-3
	HAL_GPIO_WritePin(EN1_GPIO_Port, EN1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(EN2_GPIO_Port, EN2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(EN3_GPIO_Port, EN3_Pin, GPIO_PIN_RESET);

	//wyłączenie wyjść PWM - IN 1-3
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);

	//wyłączenie komunikacji
	commLedReset();

	//zerowanie flagi
	bldcMotorState = 0;


	ZERO_PERIOD = 0;
	TIM2->ARR = 0;
	wypelnienie = 0;
}
/**
 *
 * @brief  Praca w pętli
 * @param  None
 * @retval None
 */
uint16_t temp2 = 0;
uint8_t cnt = 0;
uint8_t Data[40];

void bldcLoop() {

	//przełączenie na pracę w układzie zamkniętym
	if (bldcMotorState == 3) {
		//PRZEŁĄCZENIA NA PĘTLE ZAMKNIĘTĄ
		bldcMotorState = 4;
		actualPeriod = TIM2->ARR;
		setDutyCycle(zadaneWypelnienie);
	}

	//PRACA BEZCZUJNIKOWA
	if (bldcMotorState == 4 && transmissionSettled == 0) {
		temp2++;
		if (temp2 == 9000) {
			setDutyCycle(	(uint16_t) ((uint32_t) (g_ADCBuffer[6] * 400) / 4095) + 80);

			temp2 = 0;
		}
	}

	//wysłanie pomiaru prędkości i wypełnienia
	if (usartTransmitDataRequest == 1) {


		commUARTdata(CMD2_POMIARY_PRED,  ZERO_PERIOD /*(uint16_t) (bldcMotorState == 4) ? ZERO_PERIOD : TIM2->ARR * 2*/,
				(uint16_t) wypelnienie);

		usartTransmitDataRequest = 0;
	}

}

/**
 *
 * @brief  Przerwanie od SYSTICK co 1ms (1kHz)
 * @param  None
 * @retval None
 */
uint8_t cnt2 = 0;
void bldcSysTick() {
	//jeżeli stan zerowaniea
	if (bldcMotorState == 1)
		initiallPositioning();
	//jeżeli rozruch w otwartej pętli
	else if (bldcMotorState == 2)
		bldcStartOLSetTimer();

	//Żądanie wysłania pomiarów
	cnt++;
	if (cnt == KOMUNIKACJA_CO_MS) {
		usartTransmitDataRequest = 1;
		cnt = 0;
	}

	//rampa narastania D
	if (bldcMotorState > 2)
		dutyCycleLoop();
}

/**
 *
 * @brief  Przerwanie od timera TIM2
 * 			-zmiana sterowania tranzystorów
 * @param  None
 * @retval None
 */

void bldcIT(TIM_HandleTypeDef *htim) {

	if (htim->Instance == TIM2) {

		//ustawienie pinów EN
		HAL_GPIO_WritePin(EN1_GPIO_Port, EN1_Pin,
				DB1EN1States[bldcMotorLastStp]);
		HAL_GPIO_WritePin(EN2_GPIO_Port, EN2_Pin,
				DB1EN2States[bldcMotorLastStp]);
		HAL_GPIO_WritePin(EN3_GPIO_Port, EN3_Pin,
				DB1EN3States[bldcMotorLastStp]);

		if (bldcMotorLastStp == 0) {
			TIM1->CCR1 = wypelnienie;
			TIM1->CCR2 = 0;
			TIM1->CCR3 = 0;
		} else if (bldcMotorLastStp == 2) {
			TIM1->CCR1 = 0;
			TIM1->CCR2 = wypelnienie;
			TIM1->CCR3 = 0;
		} else if (bldcMotorLastStp == 4) {
			TIM1->CCR1 = 0;
			TIM1->CCR2 = 0;
			TIM1->CCR3 = wypelnienie;
		}
		//inkrementacja flagi położenia sterowania
		bldcMotorLastStp++;
		if (bldcMotorLastStp == 6)
			bldcMotorLastStp = 0;

		//wyzerowanie flag
		zeroDetected = 0;
		ADCcnt = 0;

		if(zeroFlag > 1 && zeroFlag < 10)
			zeroFlag++;

		//jeżeli pracujemy bezczujnikowo
		if (bldcMotorState == 4) {
			//ustawinie okresu kolejnego przełączenia (przy opadającej BEMF)
			TIM2->ARR = actualPeriod;
		}

	}

}

void initiallPositioning() {
	if (positioningStepsCounter == 0) {
		TIM1->CCR1 = positioningDutyCycle;
		TIM1->CCR2 = 0;
		TIM1->CCR3 = 0;
		wypelnienie = positioningDutyCycle;
		HAL_GPIO_WritePin(EN1_GPIO_Port, EN2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(EN2_GPIO_Port, EN2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(EN3_GPIO_Port, EN3_Pin, GPIO_PIN_SET);

	} else if (positioningStepsCounter == positioningTime) {
		TIM1->CCR1 = 0;
		TIM1->CCR2 = 0;
		TIM1->CCR3 = 0;

		HAL_GPIO_WritePin(EN1_GPIO_Port, EN2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(EN2_GPIO_Port, EN2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(EN3_GPIO_Port, EN3_Pin, GPIO_PIN_RESET);

		//Inicjalizacja rozruchu w pętli otwartej
		bldcStartOLInit();

		//przejdź do rozruchu w pętli otwartej
		bldcMotorState = 2;
		TIM2->ARR = 0;
		//uruchamianie timera 2 odpowiedzialnego za przełączanie portów EN0
		HAL_TIM_Base_Start_IT(&htim2);

	}

	positioningStepsCounter++;
}

/**
 *
 * @brief  Inicjalizacja i uruchomienie rozruchu w pętli otwartej
 * @param  None
 * @retval None
 *
 */

void bldcStartOLInit() {

	//wyzerowanie stanu licznika
	bldcStartOLcounter = 0;

	bldcStartOLfdiff = bldcStartOLfmax-bldcStartOLfmin;

	bldcStartOLddiff = bldcStartOLdmax - bldcStartOLdmin;
	wypelnienie = bldcStartOLdmin;
	//wartość początkowa licznika TIM2 odpowiadająca częstotliwości początkowej
	TIM2->ARR = 1600000 / bldcStartOLfmin;
}
/**
 *
 * @brief  Ustawianie częstotliwości podczas rozruchu w pętli otwartej
 * @param  None
 * @retval None
 *
 */
void bldcStartOLSetTimer() {
	//zmiana częstotliwości
	TIM2->ARR = 1600000 / ( (bldcStartOLcounter * bldcStartOLfdiff )/ bldcStartOLtime + bldcStartOLfmin);
	//jeżeli przekroczono wartość max. licznika to zeruj
	if ((TIM2->ARR - 6) <= TIM2->CNT) {
		TIM2->CNT = 0;
	}
	//liczba dokonanych inkrementacji
	bldcStartOLcounter++;

	//zmiana wypełnienia
	wypelnienie = ((uint32_t) (bldcStartOLcounter * bldcStartOLddiff))
					/ bldcStartOLtime + bldcStartOLdmin;

	if (bldcMotorLastStp == 1)
		TIM1->CCR1 = wypelnienie;
	else if (bldcMotorLastStp == 3)
		TIM1->CCR2 = wypelnienie;
	else if (bldcMotorLastStp == 5)
		TIM1->CCR3 = wypelnienie;

	//sprawdzenie czy nie jest to koniec rozruchu
	if (bldcStartOLcounter == bldcStartOLtime) {
		//zapisanie stanu licznika TIM2 odpowiadającemu aktualnej
		//częstotliwości komutacji w sterowaniu otwartym
		actualPeriod = TIM2->ARR;
		//przejście do pracy w pętli otwartej
		bldcMotorState = 3;
		return;
	}

}

/**
 *
 * @brief  Przerwanie wykonane po zakończeniu przetarzania ADC
 * @param  None
 * @retval None
 *
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* AdcHandle) {

	//jeżeli wykryto już zero i jesteśmy w pracy bezczujnikowej, nie rób nic
	if (zeroDetected == 1 || bldcMotorState != 4)
		return;

	//obliczanie średniej z 3 pomiarów w serii
	uint16_t mean = 0;
	if (bldcMotorLastStp == 0 || bldcMotorLastStp == 3)
		mean = (g_ADCBuffer[BEMF_A] + g_ADCBuffer[BEMF_A + ADC_CHANNELS]) / 2;
	else if (bldcMotorLastStp == 2 || bldcMotorLastStp == 5)
		mean = (g_ADCBuffer[BEMF_B] + g_ADCBuffer[BEMF_B + ADC_CHANNELS]) / 2;
	else if (bldcMotorLastStp == 4 || bldcMotorLastStp == 1)
		mean = (g_ADCBuffer[BEMF_C] + g_ADCBuffer[BEMF_C + ADC_CHANNELS]) / 2;

	uint8_t dir = BEMF_UP;
	//wyznaczanie kierunku przechodzenia przez zero
	if (bldcMotorLastStp % 2 == 1)
		dir = BEMF_DOWN;

	//sprawdzenie czy uśredniona wartość BEMF spełnia zadane warunki
	if (dir == BEMF_UP && mean < 1000 && mean >= ZEROTHRESHOLD)
		ADCcnt++;
	else
		ADCcnt = 0;


	if (ADCcnt >= NROFTHRESHOLDS) {
		HAL_GPIO_TogglePin(TEST2_GPIO_Port, TEST2_Pin);

		//ustaw flagę po zidentyfikowaniu zera
		zeroDetected = 1;
		//określenie miejsca komutacji
		estimateCommutation();
		//zerowanie licznika
		ADCcnt = 0;
	}

}
void ADC_IRQHandler() {
	HAL_ADC_IRQHandler(&hadc1);
}
/**
 *
 * @brief  Estymowanie nowego momentu komutacji na podstawie zmierzonych przejść BEMF przez zero
 * @param  None
 * @retval None
 *
 */
void estimateCommutation() {

	//wartość w liczniku TIM2 odpowiadająca czasowi od poprzedniej komutacji do momentu wykrycia przejścia przez zero
	uint32_t TCNT = TIM2->CNT;
	if(zeroFlag == 4)
		zeroFlag = 1;

	//zadania wykonywane, jeżeli jest to dopiero pierwsza detekcja zera w całym cyklu pracy
	if (zeroFlag == 0 || zeroFlag > 4) {
		zeroFlag = 2;
		//wartość w liczniku TIM2 odpowiadająca czasowi od wykrytej detekcji zera do komutacji
		lastRest = TIM2->ARR - TCNT;
		return;
	}

	if(zeroFlag != 1)
		return;
	//estymowana wartość okresu pomiędzy dwoma wykryciami zera komutacji
	ZERO_PERIOD = (lastRest + actualPeriod + TCNT) / 2;
	//wartość w liczniku TIM2 odpowiadająca czasowi od wykrytej detekcji zera do komutacji
	lastRest = ZERO_PERIOD / 2;
	//ustawienie czasu następnej komutacji
	TIM2->ARR = TCNT + lastRest;
	//wartość czasu komutacji w przedziale opadającej BEMF, gdzie nie są dokonywane pomiary
	actualPeriod = ZERO_PERIOD;
}
/*************************RAMPA/KONTROLA PRĄDU*******************************/
/**
 *
 * @brief  Zmień wypełnienie z rampą
 * @param  None
 * @retval None
 *
 */
void setDutyCycle(uint16_t value) {
	if (bldcMotorState < 3)
		return;
	//żądanie nowej rampy
	if (rampaDCounterMax != 0) {
		rampaDCounterMax = 0;
		rampaDCounter = 0;

	}

	//saturation - obcięcie wartości większych od maksymalnych dopuszczalnych
	if (value > D_MAX)
		value = D_MAX;
	else if (value < 100)
		value = 100;

	uint16_t diff = 0;
	//obliczenie liczby kroków
	if (value > wypelnienie) {
		rampaSign = 1;
		diff = (value - wypelnienie);
	} else {
		rampaSign = -1;
		diff = (wypelnienie - value);
	}


	//jeżeli różnica mniejsz od jednego kroku, zareaguj natychmiast
	if (diff <= rampaScale) {
		wypelnienie = value;
		return;
	}
	//max wartość
	rampaDCounterMax = diff * rampaScale;
	//startuj
	rampaDCounter = 0;
}
/**
 *
 * @brief  Aktualizuj wypełnienie
 * @param  None
 * @retval None
 *
 */
void dutyCycleLoop() {
	if (rampaDCounterMax == 0 || bldcMotorState != 4)
		return;
	rampaDCounter++;

	if (rampaDCounter % rampaScale == 0) {
		//aktualizacja wypełnienia
		wypelnienie = wypelnienie + rampaSign;
		//zakończenie zliczania
		if (rampaDCounterMax == rampaDCounter) {
			rampaDCounterMax = 0;
			rampaDCounter = 0;
		}
	}
}
