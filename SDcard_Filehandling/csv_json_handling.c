/*
 * csv_json_handling.c
 *
 *  Created on: Dec 24, 2023
 *      Author: hzaib
 */

#include "csv_json_handling.h"
#include "main.h"
#include "app_main.h"
#include "fatfs.h"
#include "File_Handling.h"
#include "string.h"
#include "stdio.h"
#include "Uhr.h"

FRESULT fresult1;

//char _Flash_Packet.filename_with_format[100];
//char filename_ver_date[150];
uint8_t file_already_exist = 0;
uint8_t fileWrite_day = 0;
uint8_t fileWrite_month = 0;
uint8_t fileWrite_year = 0;

uint8_t fileWrite_hour = 0;
uint8_t fileWrite_min = 0;
uint8_t fileWrite_sec = 0;

uint8_t datawritten = 0;
char buffer[255];

void json_update() {
	RTC_DateTypeDef sDate;
	HAL_RTC_GetDate(RTC_Handle, &sDate, RTC_FORMAT_BIN);
	RTC_TimeTypeDef sTime;
	HAL_RTC_GetTime(RTC_Handle, &sTime, RTC_FORMAT_BIN);
	fileWrite_day = sDate.Date;
	fileWrite_month = sDate.Month;
	fileWrite_year = sDate.Year;
	fileWrite_hour = sTime.Hours;
	fileWrite_min = sTime.Minutes;
	fileWrite_sec = sTime.Seconds;

	fresult1 = Mount_SD("/");

	if (fresult1 == FR_OK) {
		//CAN Bus
		sprintf(buffer, "{\n");
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "\"devEUI\":\"%s\",\n", ver_GetUid());
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "\"id\":\"%s\",\n", _Flash_Packet.id);
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "\"timestamp\":\"%02d:%02d:%02d\",\n", sTime.Hours,
				sTime.Minutes, sTime.Seconds);
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "\"location\":\"%s\",\n", _Flash_Packet.location);
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "\"Filename\":\"%s\",\n", _Flash_Packet.filename_ver_date);
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "\"group\":\"%s\",\n", _Flash_Packet.group);
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

#ifdef use_scd40x
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "\"co2\":\"%d\",\n", _RunTime_Packet.co2);
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "\"temperature\":\"%ld\",\n",
				_RunTime_Packet.temperature);
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "\"humidity\":\"%ld\",\n", _RunTime_Packet.humidity);
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);
#elif use_scd30
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "\"co2\":\"%.2f\",\n", _RunTime_Packet.co2);
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "\"temperature\":\"%.2f\",\n",
				_RunTime_Packet.temperature);
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "\"humidity\":\"%.2f\",\n", _RunTime_Packet.humidity);
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);
#endif
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "\"pir\":\"%s\",\n",
				_RunTime_Packet.motion_detection ? "TRUE" : "FALSE");
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "\"pm2.5\":\"%.02f\",\n", _RunTime_Packet.pm2_5);
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "\"port\":\"none\",\n");
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "\"battery\":\"%.02f\",\n",
				_RunTime_Packet.battery_voltage);
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

		//we end it
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "}\n\n");
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

		Unmount_SD("/");
	}
	_RunTime_Packet.sd_card_disk_write_error = fresult1;
}
void csv_update() {
	RTC_DateTypeDef sDate;
	HAL_RTC_GetDate(RTC_Handle, &sDate, RTC_FORMAT_BIN);
	RTC_TimeTypeDef sTime;
	HAL_RTC_GetTime(RTC_Handle, &sTime, RTC_FORMAT_BIN);
	fileWrite_day = sDate.Date;
	fileWrite_month = sDate.Month;
	fileWrite_year = sDate.Year;
	fileWrite_hour = sTime.Hours;
	fileWrite_min = sTime.Minutes;
	fileWrite_sec = sTime.Seconds;
	fresult1 = Mount_SD("/");

	if (fresult1 == FR_OK) {


		sprintf(buffer, "%s ,", ver_GetUid());
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%s ,", _Flash_Packet.id);
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%02d:%02d:%02d ,", sTime.Hours, sTime.Minutes,
				sTime.Seconds);
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%s ,", _Flash_Packet.location);
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%s ,", _Flash_Packet.filename_ver_date);
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%s ,", _Flash_Packet.group);
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);
#ifdef use_scd40x
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%d ,", _RunTime_Packet.co2);
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%ld ,", _RunTime_Packet.temperature);
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%ld ,", _RunTime_Packet.humidity);
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);
#elif use_scd30
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%.2f ,", _RunTime_Packet.co2);
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%.2f ,", _RunTime_Packet.temperature);
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%.2f ,", _RunTime_Packet.humidity);
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);
#endif
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%s ,",
				_RunTime_Packet.motion_detection == 1 ? "TRUE" : "FALSE");
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%.02f ,", _RunTime_Packet.pm2_5);
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "NONE ,");
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%.02f ,", _RunTime_Packet.battery_voltage);
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "\n");
		fresult1 = Update_File(_Flash_Packet.filename_with_format, buffer);

		Unmount_SD("/");
	}
	_RunTime_Packet.sd_card_disk_write_error = fresult1;
}
void csv_header() {

	fresult1 = Mount_SD("/");

	if (fresult1 == FR_OK) {


		sprintf(buffer, "devEUI ,");
		Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "id ,");
		Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "timestamp ,");
		Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "location ,");
		Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "Filename ,");
		Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "group ,");
		Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "co2 ,");
		Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "temperature ,");
		Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "humidity ,");
		Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "pir ,");
		Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "pm2.5 ,");
		Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "port ,");
		Update_File(_Flash_Packet.filename_with_format, buffer);

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "battery \n\n");
		Update_File(_Flash_Packet.filename_with_format, buffer);

		Unmount_SD("/");
	}
	_RunTime_Packet.sd_card_disk_write_error = fresult1;

}
uint8_t filesaving_process() {
//	__disable_irq();

	 Mount_SD("/");
	 if(check_file_exists(_Flash_Packet.filename_with_format)) //if file does not exists then create it
	 {
		Unmount_SD("/");
		_RunTime_Packet.sd_file_creation = createfile(_Flash_Packet.File_Name,_Flash_Packet.File_Format);
	 }
	 else
	 {
		Unmount_SD("/");
	 }




	memset(buffer, 0, sizeof(buffer));
	if (_RunTime_Packet.fileformat_selection) {
		csv_update();
	} else {
		json_update();
	}

	if (_RunTime_Packet.sd_card_disk_write_error != 0) //means there is error
			{
		//blink red so user can know
		blink_red();
		datawritten = 2;
		return 1; //not working
	} else //packet written succesfully
	{
		datawritten = 1;
	}

	return 0; //all good
//	__enable_irq();
}
int8_t createfile(char *filename, char *fileformat) {

	RTC_DateTypeDef sDate;
	HAL_RTC_GetDate(RTC_Handle, &sDate, RTC_FORMAT_BIN);
	RTC_TimeTypeDef sTime;
	HAL_RTC_GetTime(RTC_Handle, &sTime, RTC_FORMAT_BIN);


	//if rtc backup register has nothing or our filename with foirmat is not same as filename then create a file otheriwse just go forward
	if((HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) != 0xBEBE) || !strstr(_Flash_Packet.filename_with_format,_Flash_Packet.File_Name)
			|| _RunTime_Packet.year_changed || _RunTime_Packet.filename_changed || _RunTime_Packet.fileformat_changed || (_RunTime_Packet.sd_file_creation == -1))
	{
	  sprintf(_Flash_Packet.filename_with_format, "%s_%02d-%02d-%02d%s", filename,sDate.Year, sDate.Month, sDate.Date, fileformat);
	  sprintf(_Flash_Packet.filename_ver_date, "%s_%02d-%02d-%02d", filename, sDate.Year,sDate.Month, sDate.Date);
	  set_param_flags();
	}
	//__disable_irq();
	if (Mount_SD("/") == FR_OK) {
	} else {
		//	__enable_irq();
		return -1;
	}
	Unmount_SD("/");



	fresult1 = Mount_SD("/");
	if (fresult1 == FR_OK) {





		fresult1 = Create_File(_Flash_Packet.filename_with_format);

		Unmount_SD("/");

		if (_RunTime_Packet.fileformat_selection) {
			if (file_already_exist) {
				file_already_exist = 0;
			} else {
				csv_header();  //json does not require an header only csv
			}
		}

	} else {
		//	__enable_irq();
		return -1;
	}
//	__enable_irq();
	return 0;
}
