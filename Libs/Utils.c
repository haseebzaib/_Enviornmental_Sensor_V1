/*
 * Utils.c
 *
 *  Created on: Dec 23, 2023
 *      Author: hzaib
 */


/************************************************************************/
/* Program:    Enviornmental_Sensor    Date: Dec 23, 2023               */
/* Version:     V1                                                      */
/* Autor: Haseeb Zaib                                                   */
/* This file handles all the ultility functions it is platform independent		    */
/************************************************************************/
#include "main.h"
#include "app_main.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "string.h"
#include "Uhr.h"

const char kaHexAsc[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

/**
 * Our date format is simple
 * Day:month:year
 * therefore delimiter is ':'
 * we will extract date and save them in there respective variable
 * return 1 = buffer empty 0 = success
 */
uint8_t Parse_Date(char *Datebuffer,uint8_t *date)
{
	uint16_t i =0;
	int16_t val = 0;
	char *ptr = strtok(Datebuffer, ":");

	if(ptr == NULL)
		{
			return 1;
		}
	else
	{

	while(ptr != NULL)
	{


			val = atoi(ptr);
			if(i == 0 && (val > 31 || val <=0))
			{
				val= 1;
			}
			else if(i == 1 && (val >  12 || val <=0))
			{
				val= 1;
			}
			else if(i == 2 && val > 1000)
			{
				val=val%100;
			}
			else if(i > 2)
			{
				return 1;
			}


		   date[i] =val;
		//printf("'%s'\n", ptr);
		ptr = strtok(NULL, ":");
		i++;
	}

	}

	return 0;

}
/**
 * Our time format is simple
 * hour:minute:second
 * therefore delimiter is ':'
 * we will extract time and save them in there respective variable
 * return 1 = buffer empty 0 = success
 */
uint8_t Parse_Time(char *Timebuffer,uint8_t *time)
{
	uint16_t i =0;
		int16_t val = 0;
		char *ptr = strtok(Timebuffer, ":");

		if(ptr == NULL)
			{
				return 1;
			}
		else
		{

		while(ptr != NULL)
		{


				val = atoi(ptr);
				if(i == 0 && (val > 24|| val <0))
				{
					val= 24;
				}
				else if(i == 1 && (val >  59 || val <0))
				{
					val= 59;
				}
				else if(i == 2 && (val > 59 || val <0))
				{
					val=59;
				}
				else if(i > 2)
				{
					return 1;
				}


				time[i] =val;
			//printf("'%s'\n", ptr);
			ptr = strtok(NULL, ":");
			i++;
		}

		}

		return 0;
}


 void CharToHexasc(uint8_t *pPos, uint8_t Wert)
{
      *pPos++ = kaHexAsc[Wert >> 4];
      *pPos = kaHexAsc[Wert & 0x0f];
}
