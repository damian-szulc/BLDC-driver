/*
 * driver_bldc1.h
 *
 *  Created on: 25 paź 2016
 *      Author: root
 */

#ifndef DRIVER_BLDC1_H_
#define DRIVER_BLDC1_H_
extern uint16_t debugonly;
/*****************************GENERALNE PROC STERUJĄCE***********************/

//deklatacja driver bldc1 motor state - stan w jakim znajduje się silnik
extern uint8_t bldcMotorState;
extern uint16_t zadWypelnienie;
//przechowuje informacje o sterowaniu w jednej z 6 części
extern volatile uint8_t bldcMotorLastStp;
extern uint16_t zadaneWypelnienie;
extern uint32_t actualPeriod;
/**************************************KONIEC*********************************/
/**********************************ZEROWANIE************************************/
extern uint16_t positioningTime;
extern uint16_t positioningDutyCycle;
/**************************************KONIEC*********************************/
/*******************PARAMETRY ROZRUCHU W OTWARTEJ PĘTLI***********************/
//minimalna częstotliwość podczas rozruchu w otwartej pętli, Hz
extern uint32_t bldcStartOLfmin;
//maksymalna częstotliwość podczas rozruchu w otwartej pętli, Hz
extern uint32_t bldcStartOLfmax;
//minimalne wypełnienie podczas rozruchu w otwartej pętli, 0-499
extern uint8_t bldcStartOLdmin ;
//maksymalne wypełnienie podczas rozruchu w otwartej pętli, 0-499
extern uint32_t bldcStartOLdmax;
//czas przyrostu, ms
extern uint32_t bldcStartOLtime;

extern uint8_t rampaScale;

uint32_t bldcStartOLddiff;

/**************************************KONIEC*********************************/

void bldcInit();


void bldcStart();


void bldcStop();


void bldcIT(TIM_HandleTypeDef *htim);

void bldcSysTick();

void bldcLoop();

/********************************ZEROWANIE*********************************/
void initiallPositioning();

/************************ROZRUCH W OTWARTEJ PĘTLI**************************/

//liczba wykonanych zmian częstotliwości przy rozruchu
uint16_t bldcStartOLcounter;
uint32_t bldcStartOLstep;

void bldcStartOLInit();

void bldcStartOLSetTimer();
/**************************ESTYMACJA KOMUTACJI***********************************/
void estimateCommutation();
/**************************RAMPA/KONTROLA PRĄDU**********************************/
void setDutyCycle(uint16_t value);
void dutyCycleLoop();

#endif /* DRIVER_BLDC1_H_ */
