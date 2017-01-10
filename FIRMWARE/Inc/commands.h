/*
 * commands.h
 *
 *  Created on: 25 paź 2016
 *      Author: root
 */

#ifndef COMMANDS_H_
#define COMMANDS_H_

/*
 * RAMKA SKŁADA SIĘ Z 5 bajtów z czego
 * 1-polecenie
 * 2-5 - opcjonalne
 */

enum {
    LICZBA_PAR_BIEGUNOW = 4,
    KOMUNIKACJA_CO_MS = 1,
    ERR_SUM_MAX = 1000,
    //max wypełnienie - %
    WYPELNIENIE_MAX = 499,
    D_MAX = 300, //60%
    D_MIN = 100,
    //regulator prędkości
    STREFA_NIECZULOSCI = 20,

    //ROZPOCZĘCIE TRANSMISJI -> przesłanie komendy START
    CMD_HELLO = 'H', //s
    CMD_SPACER = ' ', //spacer

    //INSTRUKCJE STERUJĄCA
    CMD_STER = 's', //s
    CMD_START = 'z', //1
    CMD_STOP = 'w', //0

    //USTAWIANIE PARAMETRÓW
    CMD_PARAM = 'p', //

    CMD_PRACA = 'a', //zmiana prędkości, wypełnienia
    CMD_PRACA_WYP = 'c', //wypelnienie

    CMD_TRYB = 't', //tryb pracy w ukladzie otwartym/zamknietym
    CMD_TRYB_OL = 'o', //0

    CMD_ZER = 'z', //zerowanie
    CMD_ZER_T = 't', //czas
    CMD_ZER_D = 'd', //wypełnienie

    CMD_ROZR_OTW = 'r', //tryb rozpędzania w ukladzie otwartym/zamknietym
    CMD_ROZR_OTW_T = 'o',
    CMD_ROZR_OTW_Fmin = 'i',
    CMD_ROZR_OTW_Fmax = 'm',
    CMD_ROZR_OTW_Dmin = 'n',
    CMD_ROZR_OTW_Dmax = 'x',

    CMD_RAMPA = 'e', //zmiana parametrów rampy

    //KOMENDY ZWROTNE
    CMD2_START = 'z',
    CMD2_STOP = 'w',
    CMD2_POMIARY_PRED = 'p',
    CMD2_POMIARY_WYP = 'd',


};
#endif /* COMMANDS_H_ */
