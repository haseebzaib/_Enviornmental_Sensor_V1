/*
 * Uhr.h
 *
 *  Created on: Dec 23, 2023
 *      Author: hzaib
 */

#ifndef UHR_H_
#define UHR_H_

#include "main.h"
#include "app_main.h"



#define RTC_Handle &hrtc
extern uint8_t set_alarm_Time ;
extern void RtcInt();
extern void Rtc_set_alarm();
extern void Set_Date(char *Datebuffer);
extern void Set_Time(char *Timebuffer);
extern void Get_Date();
extern void Get_Time();


#endif /* UHR_H_ */
