/*
 * Uhr.c
 *
 *  Created on: Dec 23, 2023
 *      Author: hzaib
 */

/************************************************************************/
/* Program:    Enviornmental_Sensor    Date: Dec 23, 2023               */
/* Version:     V1                                                      */
/* Autor: Haseeb Zaib                                                   */
/* This file handles all the functions related to Internal RTC		    */
/************************************************************************/
#include "main.h"
#include "app_main.h"
#include "Uhr.h"
#include "Utils.h"

uint8_t set_alarm_Time = 1;
char scheduled_packet[30] = "NULL";

uint16_t prev_min = 0;
void RtcInt()
{

	set_alarm_Time = 1;
	//Rtc_set_alarm();
}

void Rtc_Alarm_watchdog_disable()
{
  __HAL_RTC_ALARM_DISABLE_IT(&hrtc, RTC_IT_ALRB);
}

void Rtc_Alarm_watchdog()
{
	uint16_t seconds;
	RTC_TimeTypeDef gTime;
	HAL_RTC_GetTime(RTC_Handle, &gTime, RTC_FORMAT_BIN);
	RTC_DateTypeDef sDate;
    HAL_RTC_GetDate(RTC_Handle, &sDate, RTC_FORMAT_BIN);

    seconds = gTime.Seconds + 25;

    if(seconds > 59)
    {
    	seconds = 0;

    }

      RTC_AlarmTypeDef sAlarm = {0};
	  sAlarm.AlarmTime.Hours = 0;
	  sAlarm.AlarmTime.Minutes = 0;
	  sAlarm.AlarmTime.Seconds = seconds;
	  sAlarm.AlarmTime.SubSeconds = 0;
	  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
	  sAlarm.AlarmMask = RTC_ALARMMASK_MINUTES| RTC_ALARMMASK_HOURS | RTC_ALARMMASK_DATEWEEKDAY;
	  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
	  sAlarm.AlarmDateWeekDay = 1;
	  sAlarm.Alarm = RTC_ALARM_B;
	  HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN);

	  __HAL_RTC_ALARM_ENABLE_IT (&hrtc, RTC_IT_ALRB);
	  __HAL_RTC_ALARM_CLEAR_FLAG (&hrtc, RTC_IT_ALRB);

}

void Set_Date(char *Datebuffer) {

	RTC_DateTypeDef sDate;
	uint8_t date[10];
	uint8_t flag;
	flag = Parse_Date(Datebuffer, date);

	if (!flag) {
		sDate.Date = date[0];
		sDate.Month = date[1];
		sDate.Year = date[2];
		HAL_RTC_SetDate(RTC_Handle, &sDate, RTC_FORMAT_BIN);
	}
HAL_Delay(500);
}


/*
 * this sets alarm according to saved intervals
 */
void Rtc_set_alarm()
{
uint16_t minute;



if(set_alarm_Time)
{
	uint8_t hour = 0;

	RTC_TimeTypeDef gTime;
	HAL_RTC_GetTime(RTC_Handle, &gTime, RTC_FORMAT_BIN);
	RTC_DateTypeDef sDate;
    HAL_RTC_GetDate(RTC_Handle, &sDate, RTC_FORMAT_BIN);


    minute = gTime.Minutes + _Flash_Packet.Time_Interval;
    hour = gTime.Hours;
    if(minute > 59)
    {
    	minute = minute%_Flash_Packet.Time_Interval;
//    	if(minute == 0)
//    	{
//    		minute = 1;
//    	}

    	hour = hour + 1; //we go to next hour as our time is schedualed for next hour

    	if(hour > 23)
    	{
          hour = 0;
    	}


    }

    sprintf(scheduled_packet,"%02d:%02d:%02d",hour,minute,gTime.Seconds);

      RTC_AlarmTypeDef sAlarm = {0};
	  sAlarm.AlarmTime.Hours = 0;
	  sAlarm.AlarmTime.Minutes = minute;
	  sAlarm.AlarmTime.Seconds = 0;
	  sAlarm.AlarmTime.SubSeconds = 0;
	  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
	  sAlarm.AlarmMask = RTC_ALARMMASK_SECONDS| RTC_ALARMMASK_HOURS | RTC_ALARMMASK_DATEWEEKDAY;
	  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
	  sAlarm.AlarmDateWeekDay = 1;
	  sAlarm.Alarm = RTC_ALARM_A;
	  HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN);

	  __HAL_RTC_ALARM_ENABLE_IT (&hrtc, RTC_IT_ALRA);
	  __HAL_RTC_ALARM_CLEAR_FLAG (&hrtc, RTC_IT_ALRA);


	  set_alarm_Time = 0;


		//HAL_UART_Transmit(&huart1, (uint8_t*) "Interrupt\r\n",11, 1000);
}

}


void Rtc_manual_alarm()
{
	RTC_TimeTypeDef gTime;
	HAL_RTC_GetTime(RTC_Handle, &gTime, RTC_FORMAT_BIN);
	RTC_DateTypeDef sDate;
    HAL_RTC_GetDate(RTC_Handle, &sDate, RTC_FORMAT_BIN);


    uint16_t minute =0;
	uint8_t hour = 0;

	   hour = gTime.Hours;


    if((gTime.Minutes % _Flash_Packet.Time_Interval) == 0 && (prev_min != gTime.Minutes))
    {
    	set_alarm_Time = 1;
    	prev_min = gTime.Minutes;
    	minute = gTime.Minutes +  _Flash_Packet.Time_Interval;
    	   if(minute > 59)
    	    {
    	    	hour = hour + 1; //we go to next hour as our time is schedualed for next hour
    	    	minute = 0;
    	    	if(hour > 23)
    	    	{
    	          hour = 0;
    	    	}
    	    }

    	   sprintf(scheduled_packet,"%02d:%02d:%02d",hour,minute,gTime.Seconds);
    }
    else if(set_alarm_Time)
    {
    	 set_alarm_Time = 0;
    }


}

void Rtc_manual_alarm_secondary()
{
	RTC_TimeTypeDef gTime;
	HAL_RTC_GetTime(RTC_Handle, &gTime, RTC_FORMAT_BIN);
	RTC_DateTypeDef sDate;
    HAL_RTC_GetDate(RTC_Handle, &sDate, RTC_FORMAT_BIN);


    uint16_t minute =0;
	uint8_t hour = 0;

	   hour = gTime.Hours;


    if((gTime.Minutes % _Flash_Packet.Time_Interval) == 0 && (prev_min != gTime.Minutes))
    {
    	set_alarm_Time = 1;
    	prev_min = gTime.Minutes;
    	minute = gTime.Minutes +  _Flash_Packet.Time_Interval;
    	   if(minute > 59)
    	    {
    	    	hour = hour + 1; //we go to next hour as our time is schedualed for next hour
    	    	minute = 0;
    	    	if(hour > 23)
    	    	{
    	          hour = 0;
    	    	}
    	    }

    	   sprintf(scheduled_packet,"%02d:%02d:%02d",hour,minute,gTime.Seconds);
    }
}


void Rtc_sleep_alarm()
{

}
void Set_Time(char *Timebuffer) {
	RTC_TimeTypeDef sTime;
	uint8_t time[10];
	uint8_t flag;
	flag = Parse_Time(Timebuffer, time);
	HAL_StatusTypeDef status;
	if (!flag) {
		sTime.Hours = time[0];
		sTime.Minutes = time[1];
		sTime.Seconds = time[2];
		sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
		sTime.StoreOperation = RTC_STOREOPERATION_RESET;
		status = HAL_RTC_SetTime(RTC_Handle, &sTime, RTC_FORMAT_BIN);
	}

	HAL_Delay(500);

}
void Get_Date() {
	RTC_DateTypeDef sDate;
	HAL_RTC_GetDate(RTC_Handle, &sDate, RTC_FORMAT_BIN);

}
void Get_Time() {
	RTC_TimeTypeDef sTime;
	HAL_RTC_GetTime(RTC_Handle, &sTime, RTC_FORMAT_BIN);
}
