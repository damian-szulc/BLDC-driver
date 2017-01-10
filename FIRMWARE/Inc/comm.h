/*
 * comm.h
 *
 *  Created on: 25 paź 2016
 *      Author: root
 */

#ifndef COMM_H_
#define COMM_H_

/******************************************LED********************************************/

//flaga wystąpienia przerwania
volatile unsigned char ledAlarmState;


//aktualny stan szybkich mrugnięć
volatile uint8_t ledAlarmBlinkState;

/*****************************************UART*********************************************/
//długość ramki
enum {
	//długoś buforu DMA do detekcji zera
	COMMUARTTRANLEN = 5,

};
//zwrócone z komunikacji z uart
uint8_t commUARTRecived[ COMMUARTTRANLEN+1 ];



/*****************************************************************************************/
/**************************************PROTOTYPES*****************************************/
/*****************************************************************************************/


//funkcja inicjalizacyjna
void commInit();


/******************************************LED********************************************/
//włączanie sygnalizacji ledem
void commLedSet(uint8_t type );

//wyłącznie sygnalizacji
void commLedReset();
/*****************************************UART*********************************************/

//inicjalizacja
void commUARTInit();

//wysłanie wiadomości przez UART
void commUARTHandleRecivedMsg();

//wysłanie wiadomości przez UART
void commUARTSent(uint8_t msg);
void commUARTSent2(uint8_t msg, uint8_t msg2);
//wysłanie danych przez UART
void commUARTdata(uint8_t msg, uint16_t data, uint16_t data2);
extern uint8_t transmissionSettled;

/****************************************BUTTON*********************************************/

//naciśnięcie przycisku
void commButtonClicked();

//naciśnięcie przycisku
uint16_t getValueFromRcvData(char a, char b);
#endif /* COMM_H_ */
