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

char filename_with_format[100];
uint8_t file_already_exist = 0;
char buffer[255];

void json_update() {
	RTC_DateTypeDef sDate;
	HAL_RTC_GetDate(RTC_Handle, &sDate, RTC_FORMAT_BIN);
	RTC_TimeTypeDef sTime;
	HAL_RTC_GetTime(RTC_Handle, &sTime, RTC_FORMAT_BIN);
	fresult1 = Mount_SD("/");

	if (fresult1 == FR_OK) {
		//CAN Bus
		sprintf(buffer, "{\n");
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "\"devEUI\":\"TF241_%s\",\n", ver_GetUid());
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "\"id\":\"%s\",\n", _Flash_Packet.id);
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "\"timestamp\":\"%02d:%02d:%02d\",\n", sTime.Hours,
				sTime.Minutes, sTime.Seconds);
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "\"location\":\"%s\",\n", _Flash_Packet.location);
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "\"name\":\"%s\",\n", _Flash_Packet.name);
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "\"group\":\"%s\",\n", _Flash_Packet.group);
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "\"co2\":\"%d\",\n", _RunTime_Packet.co2);
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "\"temperature\":\"%ld\",\n",
				_RunTime_Packet.temperature);
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "\"humidity\":\"%ld\",\n", _RunTime_Packet.humidity);
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "\"pir\":\"%s\",\n",
				_RunTime_Packet.motion_detection ? "TRUE" : "FALSE");
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "\"pm2.5\":\"%.02f\",\n", _RunTime_Packet.pm2_5);
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "\"port\":\"none\",\n");
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "\"battery\":\"%.02f\",\n",
				_RunTime_Packet.battery_voltage);
		Update_File(filename_with_format, buffer);

		//we end it
		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "}\n\n");
		fresult1 = Update_File(filename_with_format, buffer);

		Unmount_SD("/");
	}
	_RunTime_Packet.sd_card_disk_write_error = fresult1;
}
void csv_update() {
	RTC_DateTypeDef sDate;
	HAL_RTC_GetDate(RTC_Handle, &sDate, RTC_FORMAT_BIN);
	RTC_TimeTypeDef sTime;
	HAL_RTC_GetTime(RTC_Handle, &sTime, RTC_FORMAT_BIN);
	fresult1 = Mount_SD("/");

	if (fresult1 == FR_OK) {
//		sprintf(buffer, " TF241_%s , %s , %02d:%02d:%02d ,", ver_GetUid(),
//				_Flash_Packet.id, sTime.Hours, sTime.Minutes, sTime.Seconds);
//		Update_File(filename_with_format, buffer);
//		sprintf(buffer, " %s , %s , %s ,", _Flash_Packet.location,
//				_Flash_Packet.name, _Flash_Packet.group);
//		Update_File(filename_with_format, buffer);
//		sprintf(buffer, " %d , %ld , %ld ,", _RunTime_Packet.co2,
//				_RunTime_Packet.temperature, _RunTime_Packet.humidity);
//		Update_File(filename_with_format, buffer);
//		sprintf(buffer, " %s , %.02f , none ,%.02f ,",
//				_RunTime_Packet.motion_detection ? "TRUE" : "FALSE",
//				_RunTime_Packet.pm2_5, _RunTime_Packet.battery_voltage);
//		Update_File(filename_with_format, buffer);
//		sprintf(buffer, "\n");
//		Update_File(filename_with_format, buffer);



		sprintf(buffer, "TF241_%s ,", ver_GetUid());
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "%s ,", _Flash_Packet.id);
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "%02d:%02d:%02d ,", sTime.Hours, sTime.Minutes, sTime.Seconds);
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "%s ,", _Flash_Packet.location);
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "%s ,", _Flash_Packet.name);
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "%s ,", _Flash_Packet.group);
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "%d ,", _RunTime_Packet.co2);
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "%ld ,", _RunTime_Packet.temperature);
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "%ld ,",  _RunTime_Packet.humidity);
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "%s ,", _RunTime_Packet.motion_detection == 1 ? "TRUE" : "FALSE");
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "%.02f ,", _RunTime_Packet.pm2_5);
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "NONE ,");
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "%.02f ,", _RunTime_Packet.battery_voltage);
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "\n");
		Update_File(filename_with_format, buffer);

		Unmount_SD("/");
	}
	_RunTime_Packet.sd_card_disk_write_error = fresult1;
}
void csv_header() {

	fresult1 = Mount_SD("/");

	if (fresult1 == FR_OK) {
		//CAN Bus
//		sprintf(buffer, " devEUI , id , timestamp ,");
//		Update_File(filename_with_format, buffer);
//		sprintf(buffer, " location , name , group ,");
//		Update_File(filename_with_format, buffer);
//		sprintf(buffer, " co2 , temperature , humidity ,");
//		Update_File(filename_with_format, buffer);
//		sprintf(buffer, " pir , pm2.5 , port ,battery\n\n");
//		Update_File(filename_with_format, buffer);

		sprintf(buffer, "devEUI ,");
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "id ,");
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "timestamp ,");
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "location ,");
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "name ,");
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "group ,");
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "co2 ,");
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "temperature ,");
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "humidity ,");
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "pir ,");
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "pm2.5 ,");
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "port ,");
		Update_File(filename_with_format, buffer);

		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "battery \n\n");
		Update_File(filename_with_format, buffer);

		Unmount_SD("/");
	}
	_RunTime_Packet.sd_card_disk_write_error = fresult1;


}
void filesaving_process() {
//	__disable_irq();
	memset(buffer,0,sizeof(buffer));
	if (_RunTime_Packet.fileformat_selection) {
		csv_update();
	} else {
		json_update();
	}

	if(_RunTime_Packet.sd_card_disk_write_error != 0)//means there is error
	{
        //blink red so user can know
		blink_red();
	}
//	__enable_irq();
}
int8_t createfile(char *filename, char *fileformat) {
	//__disable_irq();
	if (Mount_SD("/") == FR_OK) {
	} else {
	//	__enable_irq();
		return -1;
	}
	Unmount_SD("/");

	RTC_DateTypeDef sDate;
	HAL_RTC_GetDate(RTC_Handle, &sDate, RTC_FORMAT_BIN);
	RTC_TimeTypeDef sTime;
	HAL_RTC_GetTime(RTC_Handle, &sTime, RTC_FORMAT_BIN);

	fresult1 = Mount_SD("/");
	if (fresult1 == FR_OK) {

		sprintf(filename_with_format, "%s_%02d-%02d-%02d%s", filename,
				sDate.Year, sDate.Month, sDate.Date, fileformat);

		fresult1 = Create_File(filename_with_format);

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
