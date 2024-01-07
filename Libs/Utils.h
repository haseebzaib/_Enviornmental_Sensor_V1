/*
 * Utils.h
 *
 *  Created on: Dec 23, 2023
 *      Author: hzaib
 */

#ifndef UTILS_H_
#define UTILS_H_
#include "main.h"



extern uint8_t Parse_Date(char *Datebuffer,uint8_t *date);
extern uint8_t Parse_Time(char *Timebuffer,uint8_t *time);
extern void CharToHexasc(uint8_t *pPos, uint8_t Wert);


#endif /* UTILS_H_ */
