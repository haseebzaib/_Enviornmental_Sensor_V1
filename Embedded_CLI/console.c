/*
 * console.c
 *
 *  Created on: Dec 23, 2023
 *      Author: hzaib
 */

/************************************************************************/
/* Program:    Enviornmental_Sensor    Date: Dec 23, 2023               */
/* Version:     V1                                                      */
/* Autor: Haseeb Zaib                                                   */
/* This file handles all the functions related to Console	            */
/************************************************************************/

/**
 * Esc[A (key up) and Esc[B (key down) navigates through history
 * Tab moves cursor to the end of autocompleted command
 * backspace removes last typed character
 */

#include "embedded_cli.h"
#include "main.h"
#include "app_main.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "stddef.h"
#include <ctype.h>
#include "Uhr.h"
#include "Utils.h"
#include "sensirion_common.h"
#include "sensirion_i2c_hal.h"
#include "sensirion_uart.h"
#include "sps30.h"
#include "internal_adc.h"
#include "scd4x_i2c.h"
#include "csv_json_handling.h"
#include "scd30_i2c.h"

static EmbeddedCli *cli;
// Definitions for CLI UART peripheral
#define UART_CLI_PERIPH &huart1
#define UART_RX_BUFF_SIZE 1
#define CLI_BUFFER_SIZE 2048
#define CLI_RX_BUFFER_SIZE 16
#define CLI_CMD_BUFFER_SIZE 32
#define CLI_HISTORY_SIZE 32
#define CLI_MAX_BINDING_COUNT 32
uint32_t max_wait_time = 300000; //5min wait time
uint32_t prev_max_wait_time = 0;

uint32_t max_wait_time_motion = 10000;
uint32_t prev_max_wait_time_motion = 0;
extern struct sps30_measurement m;
/**
 * Definition of the cli_printf() buffer size.
 * Can make smaller to decrease RAM usage,
 * make larger to be able to print longer strings.
 */
#define CLI_PRINT_BUFFER_SIZE 512
// UART buffers
uint8_t UART_CLI_rxBuffer[UART_RX_BUFF_SIZE] = { 0 };

//static CLI_UINT cliBuffer[BYTES_TO_CLI_UINTS(CLI_BUFFER_SIZE)];

bool cliIsReady = false;
uint8_t cli_other = 0;
uint8_t flag_cli = 0;
uint8_t invoke_time = 0;
uint8_t invoke_date = 0;

// STM32 UART callback function, to pass received characters to the embedded-cli
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart == UART_CLI_PERIPH && cliIsReady) {
		HAL_UART_Receive_IT(UART_CLI_PERIPH, UART_CLI_rxBuffer,
		UART_RX_BUFF_SIZE);

		if (!cli_other) {
			char c = UART_CLI_rxBuffer[0];
			embeddedCliReceiveChar(cli, c);
		} else {
			flag_cli = 1;
		}

		prev_sleep_time = HAL_GetTick();
	}

//	if(huart == &huart2)
//	{
//		sensirion_uart_interrupt();
//	}
}

/**
 static void onCommand(EmbeddedCli *embeddedCli, CliCommand *command)
 {
 embeddedCliTokenizeArgs(command->args);
 for (int i = 1; i <= embeddedCliGetTokenCount(command->args); ++i)
 {
 const char *token = embeddedCliGetToken(command->args, i);
 size_t tokenSize = strlen(token);
 // UART2_write(uart2Handle, (const void *)token, tokenSize, NULL);
 HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t *)token, tokenSize, 1000);
 }
 }
 **/

static void writeCharToCli(EmbeddedCli *embeddedCli, char c) {
	uint8_t c_to_send = c;
	HAL_UART_Transmit(UART_CLI_PERIPH, &c_to_send, 1, 100);
}

// Function to encapsulate the 'embeddedCliPrint()' call with print formatting arguments (act like printf(), but keeps cursor at correct location).
// The 'embeddedCliPrint()' function does already add a linebreak ('\r\n') to the end of the print statement, so no need to add it yourself.
void cli_printf(EmbeddedCli *cli, const char *format, ...) {
	// Create a buffer to store the formatted string
	char buffer[CLI_PRINT_BUFFER_SIZE];

	// Format the string using snprintf
	va_list args;
	va_start(args, format);
	int length = vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	// Check if string fitted in buffer else print error to stderr
	if (length < 0) {
		fprintf(stderr, "Error formatting the string\r\n");
		return;
	}

	// Call embeddedCliPrint with the formatted string
	embeddedCliPrint(cli, buffer);
}

void receiveString(EmbeddedCli *cli, char *buffer, size_t bufferSize) {
	size_t index = 0;
	int i, j;
	prev_max_wait_time = HAL_GetTick();
	while (1) {
		if (flag_cli) {
			if (UART_CLI_rxBuffer[0] == '\x0d' || index == bufferSize - 1) {
				// Exit the loop when Enter key is pressed or buffer is full
				buffer[index] = '\0'; // Null-terminate the string
				flag_cli = 0;
				// Convert the entered string to lowercase
//				for (size_t i = 0; buffer[i]; i++) {
//					buffer[i] = tolower(buffer[i]);
//				}
				//Iterating each character and removing non alphabetical characters.
				for (i = 0; buffer[i] != '\0'; ++i) {
					while (!((buffer[i] >= 'a' && buffer[i] <= 'z')
							|| (buffer[i] >= 'A' && buffer[i] <= 'Z')
							|| buffer[i] == '_' || buffer[i] == ':'
							|| (buffer[i] >= 0x30 && buffer[i] <= 0x39)
							|| buffer[i] == '\0')) {
						for (j = i; buffer[j] != '\0'; ++j) {
							buffer[j] = buffer[j + 1];
						}
						buffer[j] = '\0';
					}
				}
				break;
			} else {
				// Check if the received character is an alphabet character
				if ((UART_CLI_rxBuffer[0] == '\b'
						|| UART_CLI_rxBuffer[0] == 0x7F)) {
					cli->writeChar(cli, '\b');
					cli->writeChar(cli, ' ');
					cli->writeChar(cli, '\b');
					--index;
					buffer[index] = '\0';
				} else {
					buffer[index++] = UART_CLI_rxBuffer[0];
					cli->writeChar(cli, UART_CLI_rxBuffer[0]);
				}
			}
			flag_cli = 0;
		}

		//incase no usb then also break it
		if ((HAL_GetTick() - prev_max_wait_time > max_wait_time) || !HAL_GPIO_ReadPin(USB_DETECT_GPIO_Port, USB_DETECT_Pin)) {
			//we break the loop if time goes above 5min
			break;
		}

	}
}

void onClearCLI(EmbeddedCli *cli, char *args, void *context) {
	cli_printf(cli, "\33[2J");
}

void debug_scd4x_PM25(EmbeddedCli *cli, char *args, void *context) {
	const char *newLine = "\r\n";
	cli_printf(cli, "Do you want to enable debug for scd4x and PM y/n?");

	cli_other = 1;
	flag_cli = 0;
	char buffer[50];
	receiveString(cli, buffer, sizeof(buffer));
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
			1000);

	if (strstr(buffer, "y")) {
		debug_scd_pm = 1;
		cli_printf(cli, "Debug enabled");
	} else if (strstr(buffer, "n")) {
		debug_scd_pm = 0;
		cli_printf(cli, "Debug disabled");
	}

	flag_cli = 0;
	cli_other = 0;
}

void DeviceSignature(EmbeddedCli *cli, char *args, void *context) {

//	cli_printf(cli,"Device ID/Signature: TF241_0x%04X.",_ID_GetSignature());

	cli_printf(cli, "Device ID/Signature: %s.", ver_GetUid());
}

void SetID(EmbeddedCli *cli, char *args, void *context) {
	uint16_t len;
	const char *newLine = "\r\n";
	cli_printf(cli, "Disclaimer: It takes 30second for any changes to save.");
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
			1000);
	cli_printf(cli, "Set the ID. Keep ID below 30 characters");
	cli_other = 1;
	flag_cli = 0;
	char buffer[50];
	receiveString(cli, buffer, sizeof(buffer));
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
			1000);

	len = strlen(buffer);
	if (len < 30) {
		if (buffer[0] != '\0') {
			if (buffer !=  NULL) {
				strcpy(_Flash_Packet.id, buffer);
				set_param_flags();
				cli_printf(cli, "ID is set to: %s", _Flash_Packet.id);
			}
		}
	} else {
		cli_printf(cli, "ID is too long");
	}

	flag_cli = 0;
	cli_other = 0;

}

void SetLocation(EmbeddedCli *cli, char *args, void *context) {
	uint16_t len;
	const char *newLine = "\r\n";
	cli_printf(cli, "Disclaimer: It takes 30second for any changes to save.");
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
			1000);
	cli_printf(cli, "Set the Location. Keep Location below 15 characters");
	cli_other = 1;
	flag_cli = 0;
	char buffer[50];
	receiveString(cli, buffer, sizeof(buffer));
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
			1000);

	len = strlen(buffer);
	if (len < 15) {
		if (buffer[0] != '\0') {
			if (buffer != NULL) {
				strcpy(_Flash_Packet.location, buffer);
				set_param_flags();
				cli_printf(cli, "Location is set to: %s",
						_Flash_Packet.location);
			}
		}
	} else {
		cli_printf(cli, "Location is too long");
	}

	flag_cli = 0;
	cli_other = 0;

}

void Setname(EmbeddedCli *cli, char *args, void *context) {
	uint16_t len;
	const char *newLine = "\r\n";
	cli_printf(cli, "Disclaimer: It takes 30second for any changes to save.");
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
			1000);
	cli_printf(cli, "Set the name. Keep name below 15 characters");
	cli_other = 1;
	flag_cli = 0;
	char buffer[50];
	receiveString(cli, buffer, sizeof(buffer));
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
			1000);

	len = strlen(buffer);
	if (len < 15) {
		if (buffer[0] != '\0') {
			if (buffer != NULL) {
				strcpy(_Flash_Packet.name, buffer);
				set_param_flags();
				cli_printf(cli, "name is set to: %s", _Flash_Packet.name);
			}
		}
	} else {
		cli_printf(cli, "name is too long");
	}

	flag_cli = 0;
	cli_other = 0;

}

void Setgroup(EmbeddedCli *cli, char *args, void *context) {
	uint16_t len;
	const char *newLine = "\r\n";
	cli_printf(cli, "Disclaimer: It takes 30second for any changes to save.");
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
			1000);
	cli_printf(cli, "Set the group. Keep name below 15 characters");
	cli_other = 1;
	flag_cli = 0;
	char buffer[50];
	receiveString(cli, buffer, sizeof(buffer));
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
			1000);

	len = strlen(buffer);
	if (len < 15) {
		if (buffer[0] != '\0') {
			if (buffer != NULL) {
				strcpy(_Flash_Packet.group, buffer);
				set_param_flags();
				cli_printf(cli, "group is set to: %s", _Flash_Packet.group);
			}
		}
	} else {
		cli_printf(cli, "group is too long");
	}

	flag_cli = 0;
	cli_other = 0;

}

void GetID(EmbeddedCli *cli, char *args, void *context) {
	cli_printf(cli, "ID: %s", _Flash_Packet.id);
}

void GetLocation(EmbeddedCli *cli, char *args, void *context) {
	cli_printf(cli, "Location: %s", _Flash_Packet.location);

}

void Getname(EmbeddedCli *cli, char *args, void *context) {
	cli_printf(cli, "Name: %s", _Flash_Packet.name);

}

void Getgroup(EmbeddedCli *cli, char *args, void *context) {
	cli_printf(cli, "Group: %s", _Flash_Packet.group);

}

void SetDate(EmbeddedCli *cli, char *args, void *context) {
	const char *newLine = "\r\n";
	cli_printf(cli,
			"Set date in following format: day:month:year eg:(15:1:23).");
	cli_other = 1;
	flag_cli = 0;
	char buffer[50];
	receiveString(cli, buffer, sizeof(buffer));
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
			1000);
	if (buffer[0] != '\0') {
		Set_Date(buffer);
	}
	flag_cli = 0;
	cli_other = 0;

	RTC_DateTypeDef gDate;
	HAL_RTC_GetDate(RTC_Handle, &gDate, RTC_FORMAT_BIN);
	RTC_TimeTypeDef gTime;
	HAL_RTC_GetTime(RTC_Handle, &gTime, RTC_FORMAT_BIN);
	cli_printf(cli, "Set Date: %02d.%02d.%02d", gDate.Date, gDate.Month,
			gDate.Year);

}

void SetTime(EmbeddedCli *cli, char *args, void *context) {
	const char *newLine = "\r\n";
	cli_printf(cli,
			"Set time in following format: hours:minutes:seconds eg:(11:15:01).");
	cli_other = 1;
	flag_cli = 0;
	char buffer[50];
	receiveString(cli, buffer, sizeof(buffer));
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
			1000);
	if (buffer[0] != '\0') {
		Set_Time(buffer);
	}
	flag_cli = 0;
	cli_other = 0;
	set_alarm_Time = 1;

	RTC_DateTypeDef gDate;
	HAL_RTC_GetDate(RTC_Handle, &gDate, RTC_FORMAT_BIN);
	RTC_TimeTypeDef gTime;
	HAL_RTC_GetTime(RTC_Handle, &gTime, RTC_FORMAT_BIN);
	cli_printf(cli, "Set Time: %02d:%02d:%02d", gTime.Hours, gTime.Minutes,
			gTime.Seconds);

}

void GetDate(EmbeddedCli *cli, char *args, void *context) {

	RTC_DateTypeDef gDate;
	HAL_RTC_GetDate(RTC_Handle, &gDate, RTC_FORMAT_BIN);
	RTC_TimeTypeDef gTime;
	HAL_RTC_GetTime(RTC_Handle, &gTime, RTC_FORMAT_BIN);
	cli_printf(cli, "%02d.%02d.%02d", gDate.Date, gDate.Month, gDate.Year);
	HAL_Delay(100);
}

//for some reason if you dont read date the time does not update

void GetTime(EmbeddedCli *cli, char *args, void *context) {

	RTC_DateTypeDef gDate;
	HAL_RTC_GetDate(RTC_Handle, &gDate, RTC_FORMAT_BIN);
	RTC_TimeTypeDef gTime;
	HAL_RTC_GetTime(RTC_Handle, &gTime, RTC_FORMAT_BIN);
	cli_printf(cli, "%02d:%02d:%02d", gTime.Hours, gTime.Minutes,
			gTime.Seconds);
	HAL_Delay(100);
}

void SetInterval(EmbeddedCli *cli, char *args, void *context) {
	const char *newLine = "\r\n";
	cli_printf(cli, "Disclaimer: It takes 30second for any changes to save.");
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
			1000);
	cli_printf(cli, "Enter 1 for 5min.");
	cli_printf(cli, "Enter 2 for 15min.");
	cli_printf(cli, "Enter 3 for 60min.");
	cli_printf(cli,
			"Disclaimer: The battery life will be reduced if interval is reduced.");
	cli_other = 1;
	flag_cli = 0;
	char buffer[50];
	receiveString(cli, buffer, sizeof(buffer));
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
			1000);

	if (strstr(buffer, "1")) {
		_Flash_Packet.Time_Interval = 5;
		set_param_flags();
	} else if (strstr(buffer, "2")) {
		_Flash_Packet.Time_Interval = 15;
		set_param_flags();
	} else if (strstr(buffer, "3")) {
		_Flash_Packet.Time_Interval = 60;
		set_param_flags();
	}

	flag_cli = 0;
	cli_other = 0;
	cli_printf(cli, "Interval is set to: %ldmin", _Flash_Packet.Time_Interval);
	set_alarm_Time = 1;
}

void GetInterval(EmbeddedCli *cli, char *args, void *context) {

	cli_printf(cli, "Interval is Set to: %ldmin", _Flash_Packet.Time_Interval);

}

void SetFilename(EmbeddedCli *cli, char *args, void *context) {
	uint16_t len;
	const char *newLine = "\r\n";
	cli_printf(cli, "Disclaimer: It takes 30second for any changes to save.");
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
			1000);
	cli_printf(cli,
			"To set file name dont use special characters or space, '_' can be used and please keep the name below 18characters.");
	cli_printf(cli,
			"If any special characters they will be omitted automatically.");

	cli_other = 1;
	flag_cli = 0;
	char buffer[50];
	receiveString(cli, buffer, sizeof(buffer));
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
			1000);

	len = strlen(buffer);
	if (len < 20) {
		if (buffer[0] != '\0') {
			if (buffer != NULL) {
				strcpy(_Flash_Packet.File_Name, buffer);
				set_param_flags();
				_RunTime_Packet.filename_changed = 1;
				cli_printf(cli, "Filename is set to: %s",
						_Flash_Packet.File_Name);

				RTC_DateTypeDef sDate;
				HAL_RTC_GetDate(RTC_Handle, &sDate, RTC_FORMAT_BIN);
				RTC_TimeTypeDef sTime;
				HAL_RTC_GetTime(RTC_Handle, &sTime, RTC_FORMAT_BIN);


				sprintf(_Flash_Packet.filename_ver_date, "%s_%02d-%02d-%02d", _Flash_Packet.File_Name, sDate.Year,sDate.Month, sDate.Date);
			}
		}
	} else {
		cli_printf(cli, "Filename is too long");
	}

	flag_cli = 0;
	cli_other = 0;

}

void GetFilename(EmbeddedCli *cli, char *args, void *context) {

	cli_printf(cli, "Filename is set to: %s", _Flash_Packet.filename_ver_date);

}

void SetFileformat(EmbeddedCli *cli, char *args, void *context) {

	const char *newLine = "\r\n";
	cli_printf(cli, "Disclaimer: It takes 30second for any changes to save.");
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
			1000);
	cli_printf(cli, "Following file formats are allowed:");
	cli_printf(cli, "1. (.csv)");
	cli_printf(cli, "2. (.json)");
	cli_printf(cli, "Enter the number to select your desired format");

	cli_other = 1;
	flag_cli = 0;
	char buffer[50];
	receiveString(cli, buffer, sizeof(buffer));
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
			1000);

	if (strstr(buffer, "1")) {
		strcpy(_Flash_Packet.File_Format, ".CSV");
		set_param_flags();
		_RunTime_Packet.fileformat_changed = 1;
		_RunTime_Packet.fileformat_selection = 1;
	} else if (strstr(buffer, "2")) {
		strcpy(_Flash_Packet.File_Format, ".JSON");
		set_param_flags();
		_RunTime_Packet.fileformat_changed = 1;
		_RunTime_Packet.fileformat_selection = 0;
	}

	flag_cli = 0;
	cli_other = 0;
	cli_printf(cli, "File format is set to: %s", _Flash_Packet.File_Format);

}

void GetFileformat(EmbeddedCli *cli, char *args, void *context) {

	cli_printf(cli, "File format is set to: %s", _Flash_Packet.File_Format);

}

void fanclean(EmbeddedCli *cli, char *args, void *context) {

	const char *newLine = "\r\n";
	int counter = 0;
	const char *dot = "........";
	uint8_t do_Calibration = 0;
	cli_printf(cli,
			"Do you want to initiate particle sensor fan-cleaning? y/n?");

	cli_other = 1;
	flag_cli = 0;
	char buffer[50];
	receiveString(cli, buffer, sizeof(buffer));
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
			1000);

	if (strstr(buffer, "n")) {
		_Flash_Packet.Calibration_Status = 0;
		_RunTime_Packet.PM_calibration = 1;
		cli_printf(cli, "Particle sensor fan-cleaning aborted.");
		//set_param_flags();
		do_Calibration = 0;
	} else if (strstr(buffer, "y")) {
		_RunTime_Packet.PM_calibration = 0;
		do_Calibration = 1;
	}

	if (do_Calibration == 1) {

		HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
				1000);
		cli_printf(cli, "Particle sensor fan-cleaning initiated, please wait.");
		while (counter < 8) {
			HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) dot, strlen(dot),
					1000);
			HAL_Delay(800);
			toggle_blue_led();
			counter++;
		}

		HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
				1000);
		cli_printf(cli, "fan-cleaning done.");

	}

	flag_cli = 0;
	cli_other = 0;
}

void co2calibrate(EmbeddedCli *cli, char *args, void *context) {
	const char *newLine = "\r\n";
	uint32_t val;
	int counter = 0;
	const char *dot = "........";
	uint8_t do_Calibration = 0;

	cli_other = 1;
	flag_cli = 0;
	char buffer[50];
	memset(buffer, '\0', sizeof(buffer));
	cli_printf(cli,
			"To calibrate Co2 sensor, please provide Co2 concentration value.");
	cli_printf(cli,
			"If Co2 concentration is unknown then just press enter to cancel the calibration.");

	receiveString(cli, buffer, sizeof(buffer));
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
			1000);

	if (buffer[0] != '\0') {
		_RunTime_Packet.CO2_calibration = 1;

		val = atoi(buffer);

		if (val < 65535) {
			_RunTime_Packet._target_co2_concentration = val;
		}

		cli_printf(cli,
				"Co2 sensor is going to be Calibrated now. Co2 concentration value provided : %d",
				_RunTime_Packet._target_co2_concentration);
		do_Calibration = 1;
	} else {
		cli_printf(cli, "Co2 sensor Calibration aborted.");
		_RunTime_Packet.CO2_calibration = 0;
		do_Calibration = 0;
	}

	if (do_Calibration == 1) {
		HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
				1000);
		cli_printf(cli, "Co2 is calibrating, please wait.");
		if (!_RunTime_Packet.scd4x_i2c_error) {

#ifdef use_scd40x
			scd4x_perform_forced_recalibration(
					_RunTime_Packet._target_co2_concentration,
					&_RunTime_Packet._frc_correction);
#elif use_scd30
		scd30_force_recalibration(_RunTime_Packet._target_co2_concentration);
#endif
		}
		while (counter < 8) {
			HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) dot, strlen(dot),
					1000);
			HAL_Delay(1000);
			toggle_blue_led();
			counter++;
		}

		HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
				1000);
		cli_printf(cli, "Sensor calibration done.");
	}

	flag_cli = 0;
	cli_other = 0;
}


void statco2calibrate(EmbeddedCli *cli, char *args, void *context)
{

	uint16_t calib_val;
	int16_t stat;
	char status[20];
	stat = scd30_get_force_recalibration_status(&calib_val);

	switch(stat)
	{

	case 0:
	{
		strcpy(status,"Success");
		break;
	}
	default:
	{
		strcpy(status,"Error  ");
		break;
	}
	}

	cli_printf(cli, "Calibration Status: %s",status);
	cli_printf(cli, "Calibration Value: %d",calib_val);



}


void co2_auto_calibration(EmbeddedCli *cli, char *args, void *context)
{

	uint16_t calib;
	const char *newLine = "\r\n";
		cli_printf(cli, "Enter \"y\" to enable auto calibration!");
		cli_printf(cli, "Enter \"n\" to disable auto calibration!");



		cli_other = 1;
		flag_cli = 0;
		char buffer[50];
		receiveString(cli, buffer, sizeof(buffer));
		HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
				1000);

		if (strstr(buffer, "y")) {
			calib = 1;

			cli_printf(cli, "Auto calibration enabled");
		} else if (strstr(buffer, "n")) {
			calib = 0;
			cli_printf(cli, "Auto calibration disabled");
		}

		flag_cli = 0;
		cli_other = 0;



	scd30_activate_auto_calibration(calib);
}


void get_auto_calibration(EmbeddedCli *cli, char *args, void *context)
{
	uint16_t calib_val;
	int16_t stat;
	char status[20];
	stat = scd30_get_auto_calibration_status(&calib_val);

	switch(stat)
	{

	case 0:
	{
		strcpy(status,"Success");
		break;
	}
	default:
	{
		strcpy(status,"Error  ");
		break;
	}
	}

	cli_printf(cli, "Calibration Status: %s",status);
	cli_printf(cli, "Auto calibration active status: %d",calib_val);
}

void set_co2_temp_offset(EmbeddedCli *cli, char *args, void *context)
{
	const char *newLine = "\r\n";
	uint32_t val;
	int counter = 0;
	const char *dot = "........";
	uint8_t do_Calibration = 0;
	cli_other = 1;
		flag_cli = 0;
		char buffer[50];
		memset(buffer, '\0', sizeof(buffer));
		cli_printf(cli,
				"To calibrate Co2 sensor Temperature offset, please provide value.");
		cli_printf(cli,
				"If temperature offset is unknown then just press enter to cancel.");

		receiveString(cli, buffer, sizeof(buffer));
		HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
				1000);

		if (buffer[0] != '\0') {
			_RunTime_Packet.temp_offset = 1;

			val = atoi(buffer);

			if (val < 65535) {
				_RunTime_Packet._tempOffset_co2_concentration = val;
			}

			cli_printf(cli,
					"Co2 sensor temperature offset is going to be Calibrated now : %d",
					_RunTime_Packet._tempOffset_co2_concentration);

			do_Calibration = 1;
		} else {
			cli_printf(cli, "Co2 sensor Temperature Offset aborted.");
			do_Calibration = 0;
		}

		if (do_Calibration == 1) {
			HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
					1000);
			cli_printf(cli, "Co2 is calibrating temperature offset, please wait.");
			if (!_RunTime_Packet.scd4x_i2c_error) {


				scd30_set_temperature_offset(_RunTime_Packet._tempOffset_co2_concentration);

			}
			while (counter < 8) {
				HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) dot, strlen(dot),
						1000);
				HAL_Delay(1000);
				toggle_blue_led();
				counter++;
			}

			HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
					1000);
			cli_printf(cli, "Sensor calibration done.");
		}

		flag_cli = 0;
		cli_other = 0;
}

void get_co2_temp_offset(EmbeddedCli *cli, char *args, void *context)
{
	uint16_t calib_val;
		int16_t stat;
		char status[20];
		stat = scd30_get_temperature_offset(&calib_val);

		switch(stat)
		{

		case 0:
		{
			strcpy(status,"Success");
			break;
		}
		default:
		{
			strcpy(status,"Error  ");
			break;
		}
		}

		cli_printf(cli, "Temperature Offset Status: %s",status);
		cli_printf(cli, "Temperature Offset Value: %d",calib_val);
}

void set_co2_altitude_compensation(EmbeddedCli *cli, char *args, void *context)
{
	const char *newLine = "\r\n";
	uint32_t val;
	int counter = 0;
	const char *dot = "........";
	uint8_t do_Calibration = 0;
	cli_other = 1;
		flag_cli = 0;
		char buffer[50];
		memset(buffer, '\0', sizeof(buffer));
		cli_printf(cli,
				"To calibrate Co2 sensor Altitude compensation, please provide value.");
		cli_printf(cli,
				"If Altitude compensation is unknown then just press enter to cancel.");

		receiveString(cli, buffer, sizeof(buffer));
		HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
				1000);

		if (buffer[0] != '\0') {
			_RunTime_Packet.temp_offset = 1;

			val = atoi(buffer);

			if (val < 65535) {
				_RunTime_Packet._tempOffset_co2_concentration = val;
			}

			cli_printf(cli,
					"Co2 sensor Altitude compensation is going to be Calibrated now : %d",
					_RunTime_Packet._tempOffset_co2_concentration);

			do_Calibration = 1;
		} else {
			cli_printf(cli, "Co2 sensor Altitude compensation aborted.");
			do_Calibration = 0;
		}

		if (do_Calibration == 1) {
			HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
					1000);
			cli_printf(cli, "Co2 is calibrating Altitude compensation, please wait.");
			if (!_RunTime_Packet.scd4x_i2c_error) {


				scd30_set_altitude_compensation(_RunTime_Packet._tempOffset_co2_concentration);

			}
			while (counter < 8) {
				HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) dot, strlen(dot),
						1000);
				HAL_Delay(1000);
				toggle_blue_led();
				counter++;
			}

			HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
					1000);
			cli_printf(cli, "Sensor calibration done.");
		}

		flag_cli = 0;
		cli_other = 0;
}

void get_co2_altitude_compensation(EmbeddedCli *cli, char *args, void *context)
{
	uint16_t calib_val;
		int16_t stat;
		char status[20];
		stat = scd30_get_altitude_compensation(&calib_val);

		switch(stat)
		{

		case 0:
		{
			strcpy(status,"Success");
			break;
		}
		default:
		{
			strcpy(status,"Error  ");
			break;
		}
		}

		cli_printf(cli, "Altitude_compensation Status: %s",status);
		cli_printf(cli, "Altitude_compensation Value: %d",calib_val);
}


void run_co2_calibration_routine(EmbeddedCli *cli, char *args, void *context)
{
	uint32_t maxTime = 120000;
	uint32_t prev_maxTime =  HAL_GetTick();
	uint32_t cur_tick = 0;
	const char *newLine = "\r\n";
	uint32_t val;
	int counter = 0;
	const char *dot = "........";
	char progressbar_buf[50] = "ProgressBar: ";
	uint8_t calib = 0;
	uint8_t do_Calibration = 0;
	cli_other = 1;
		flag_cli = 0;
		char buffer[50];
		memset(buffer, '\0', sizeof(buffer));

		     cli_printf(cli,"");
				cli_printf(cli,"It takes about 2-3min to run complete routine. ");
				cli_printf(cli,"once this routine is activated it cant be canceled. ");
				cli_printf(cli,"Do you want to run calibration routine y/n?. ");

		receiveString(cli, buffer, sizeof(buffer));
		HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
				1000);

		if (strstr(buffer, "y")) {
			calib = 1;

			cli_printf(cli, "Calibration routine running");
		} else if (strstr(buffer, "n")) {
			calib = 0;
			cli_printf(cli, "Calibration routine aborted");
		}


		if(calib)
		{
			if (!_RunTime_Packet.scd4x_i2c_error) {
				scd30_start_periodic_measurement(0);
			}
			cli_printf(cli,"");
			cli_printf(cli,"GuideLines");
			cli_printf(cli,"");
			cli_printf(cli,"1)Donot remove USB, otherwise calibrations will be canceled. ");
			cli_printf(cli,"2)Expose the sensor to a controlled environment with a known value of Co2. ");
			cli_printf(cli,"3)After 2min, apply known value of Co2. ");
			cli_printf(cli,"4)Calibration routine finished. ");
			memset(buffer, '\0', sizeof(buffer));

			 prev_maxTime =  HAL_GetTick();
				cur_tick = HAL_GetTick();
				cli_printf(cli,"");
				cli_printf(cli,"");
				HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) progressbar_buf, strlen(progressbar_buf),
									1000);
			 while (HAL_GPIO_ReadPin(USB_DETECT_GPIO_Port, USB_DETECT_Pin)) {

					if ((HAL_GetTick() - cur_tick > 5000)) {


							HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*)"##", 2,
									1000);
							cur_tick = HAL_GetTick();
						}

			        if ((HAL_GetTick() - prev_maxTime > maxTime)) {
					//we break the loop if time goes above 5min
					break;
				}

			}

			 cli_printf(cli,"");
			 cli_printf(cli,"");
			 cli_printf(cli,"");
			 cli_printf(cli,"Enter Known Co2 reference value!");

				receiveString(cli, buffer, sizeof(buffer));
				HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
						1000);

				if (buffer[0] != '\0') {
					_RunTime_Packet.CO2_calibration = 1;

					val = atoi(buffer);

					if (val < 65535) {
						_RunTime_Packet._target_co2_concentration = val;
					}

					cli_printf(cli,
							"Co2 sensor is going to be Calibrated now. Co2 concentration value provided : %d",
							_RunTime_Packet._target_co2_concentration);
					do_Calibration = 1;
				} else {
					cli_printf(cli, "Co2 sensor Calibration aborted. No value provided");
					_RunTime_Packet.CO2_calibration = 0;
					do_Calibration = 0;
				}

				if (do_Calibration == 1) {
					HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
							1000);
					cli_printf(cli, "Co2 is calibrating, please wait.");
					if (!_RunTime_Packet.scd4x_i2c_error) {

			#ifdef use_scd40x
						scd4x_perform_forced_recalibration(
								_RunTime_Packet._target_co2_concentration,
								&_RunTime_Packet._frc_correction);
			#elif use_scd30
					scd30_force_recalibration(_RunTime_Packet._target_co2_concentration);
			#endif
					}
					while (counter < 8) {
						HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) dot, strlen(dot),
								1000);
						HAL_Delay(1000);
						toggle_blue_led();
						counter++;
					}

					HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
							1000);
					cli_printf(cli, "Sensor calibration done.");
				}


		}




		flag_cli = 0;
			cli_other = 0;

}


void set_co2_samples(EmbeddedCli *cli, char *args, void *context)
{
	const char *newLine = "\r\n";
	cli_printf(cli, "Disclaimer: It takes 30second for any changes to save.");
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
			1000);
	cli_printf(cli, "Enter the number of Samples: ");

	uint16_t val;
	cli_other = 1;
	flag_cli = 0;
	char buffer[50];
	memset(buffer, '\0', sizeof(buffer));
	receiveString(cli, buffer, sizeof(buffer));
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
			1000);

	if (buffer[0] != '\0') {

		val = atoi(buffer);

			if (val < 65535) {
				_Flash_Packet.co2_samples = val;
			}

		cli_printf(cli, "Samples: %d", _Flash_Packet.co2_samples);
		set_param_flags();
	}
	else
	{
		cli_printf(cli, "No Value provided");
	}

	flag_cli = 0;
	cli_other = 0;

}

void get_co2_samples(EmbeddedCli *cli, char *args, void *context)
{
	cli_printf(cli, "Co2 Samples: %d", _Flash_Packet.co2_samples);
}


void SystemRestart(EmbeddedCli *cli, char *args, void *context) {
	const char *newLine = "\r\n";
	cli_printf(cli, "Do you want to restart the device y/n?");

	cli_other = 1;
	flag_cli = 0;
	char buffer[50];
	receiveString(cli, buffer, sizeof(buffer));
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t*) newLine, strlen(newLine),
			1000);

	if (strstr(buffer, "y")) {
		cli_printf(cli, "Device is going to restart.");
		HAL_NVIC_SystemReset();
	} else if (strstr(buffer, "n")) {
		cli_printf(cli, "Device restart aborted.");
	}

	flag_cli = 0;
	cli_other = 0;
}

void Co2Level(EmbeddedCli *cli, char *args, void *context) {

	if (!_RunTime_Packet.scd4x_i2c_error) {


#ifdef use_scd40x
		get_scd4x_measurement();
#elif use_scd30
	get_scd30_measurement_();
#endif
	}
#ifdef use_scd40x
	cli_printf(cli, "Co2 Value: %d", _RunTime_Packet.co2);
#elif use_scd30
	cli_printf(cli, "Co2 Value: %.2f", _RunTime_Packet.co2);
#endif
}

void TempLevel(EmbeddedCli *cli, char *args, void *context) {

	if (!_RunTime_Packet.scd4x_i2c_error) {
#ifdef use_scd40x
		get_scd4x_measurement();
#elif use_scd30
	get_scd30_measurement_();
#endif
	}


#ifdef use_scd40x
	cli_printf(cli, "Temperature Value: %dC ", _RunTime_Packet.temperature);
#elif use_scd30
	cli_printf(cli, "Temperature Value: %.2fC ", _RunTime_Packet.temperature);
#endif
}

void HumidLevel(EmbeddedCli *cli, char *args, void *context) {

	if (!_RunTime_Packet.scd4x_i2c_error) {
#ifdef use_scd40x
		get_scd4x_measurement();
#elif use_scd30
	get_scd30_measurement_();
#endif
	}



#ifdef use_scd40x
	cli_printf(cli, "Humidity Value: %d ", _RunTime_Packet.humidity);
#elif use_scd30
	cli_printf(cli, "Humidity Value: %.2f ", _RunTime_Packet.humidity);
#endif
}

uint8_t prev_motion;

void MotionDetection(EmbeddedCli *cli, char *args, void *context) {

	cli_printf(cli, "For 10seconds system will poll here to detect motion.");
	HAL_Delay(2000);
	prev_max_wait_time_motion = HAL_GetTick();
	prev_motion = 0;
	while (HAL_GetTick() - prev_max_wait_time_motion < max_wait_time_motion) {

		if (HAL_GPIO_ReadPin(OUT_MOTION_GPIO_Port, OUT_MOTION_Pin)) {
			_RunTime_Packet.motion_detection = 1;
			//cli_printf(cli,"PIR Motion: %d\r\n",_RunTime_Packet.motion_detection);
		} else {
			_RunTime_Packet.motion_detection = 0;
		}

//		if(prev_motion != _RunTime_Packet.motion_detection )
//	         	{
//			prev_motion = _RunTime_Packet.motion_detection ;
		cli_printf(cli, "PIR Motion: %s\r\n",
				_RunTime_Packet.motion_detection == 1 ? "TRUE" : "FALSE");
//	         	}
		HAL_Delay(100);

//if(_RunTime_Packet.motion_detection == 1)
//{
//disable_motion();
//HAL_Delay(500);
//enable_motion();
//HAL_Delay(500);
//}
//

	}

	cli_printf(cli, "Session ended");

}

void AirQuality(EmbeddedCli *cli, char *args, void *context) {

	cli_printf(cli, "AirQuality Measured Values:");
//	if(_RunTime_Packet.pm1_0 == 0 && _RunTime_Packet.pm2_5 == 0 && _RunTime_Packet.pm4_0 == 0 && _RunTime_Packet.pm10_0 == 0)
//	{
//		cli_printf(cli,"Calculating, try again later");
//	}
//	else
//	{
//	cli_printf(cli,"%0.2f pm1.0", _RunTime_Packet.pm1_0);
//	cli_printf(cli,"%0.2f pm2.5 ",_RunTime_Packet.pm2_5);
//	cli_printf(cli,"%0.2f pm4.0 ",_RunTime_Packet.pm4_0);
//	cli_printf(cli,"%0.2f pm10.0",_RunTime_Packet.pm10_0);
//	}

	if (_RunTime_Packet.usb_detection || _RunTime_Packet.usb_first_start) {
		cli_printf(cli, "Calculating, try again later");
	} else {

		get_sps30_measurement();
		cli_printf(cli, "%0.2f pm1.0", _RunTime_Packet.pm1_0);
		cli_printf(cli, "%0.2f pm2.5 ", _RunTime_Packet.pm2_5);
		cli_printf(cli, "%0.2f pm4.0 ", _RunTime_Packet.pm4_0);
		cli_printf(cli, "%0.2f pm10.0", _RunTime_Packet.pm10_0);
	}

}

void BattVolt(EmbeddedCli *cli, char *args, void *context) {

	//float batteryVoltage;
	adc_Measure(&_RunTime_Packet.battery_voltage);
	cli_printf(cli, "Battery voltage: %0.2f", _RunTime_Packet.battery_voltage);

}

void port(EmbeddedCli *cli, char *args, void *context) {
	cli_printf(cli, "Auxiliary port value measured(inactive)");
}

void showall(EmbeddedCli *cli, char *args, void *context) {
	char buff_pm[20];
	char buff_filewritten[50];

	if (_RunTime_Packet.usb_detection || _RunTime_Packet.usb_first_start) {
		sprintf(buff_pm, "Calculating");
	} else {

		get_sps30_measurement();
		sprintf(buff_pm, "%0.2f", _RunTime_Packet.pm2_5);

	}
	adc_Measure(&_RunTime_Packet.battery_voltage);
	if (HAL_GPIO_ReadPin(OUT_MOTION_GPIO_Port, OUT_MOTION_Pin)) {
		_RunTime_Packet.motion_detection = 1;
	} else {
		_RunTime_Packet.motion_detection = 0;
	}

	char co2_Sensor_stat[30] = "ERROR";
	if (!_RunTime_Packet.scd4x_i2c_error) {
#ifdef use_scd40x
		get_scd4x_measurement();
#elif use_scd30
	//get_scd30_measurement();
#endif
		strcpy(co2_Sensor_stat,"ACTIVE");
	}


	RTC_DateTypeDef gDate;
	HAL_RTC_GetDate(RTC_Handle, &gDate, RTC_FORMAT_BIN);
	RTC_TimeTypeDef gTime;
	HAL_RTC_GetTime(RTC_Handle, &gTime, RTC_FORMAT_BIN);

	if (datawritten == 0) {

		sprintf(buff_filewritten, "Data not written yet");

	} else if (datawritten == 1) {
		sprintf(buff_filewritten, " **SUCCESSFUL** ");
	} else if (datawritten == 2) {

		sprintf(buff_filewritten, " **FAILED** ");
	}
	uint16_t calib_val = 0;
	uint16_t calib_stat = 0;
	uint16_t temp_offset = 0;
	uint16_t altitude_comp = 0;
	if (!_RunTime_Packet.scd4x_i2c_error) {
	scd30_get_force_recalibration_status(&calib_val);
	scd30_get_auto_calibration_status(&calib_stat);
	scd30_get_altitude_compensation(&altitude_comp);
	scd30_get_temperature_offset(&temp_offset);
	}


	cli_printf(cli, "");
	cli_printf(cli, "");
	cli_printf(cli, "");

	cli_printf(cli, " *devEUI           | %s                       ",
			ver_GetUid());
	cli_printf(cli, " *Id               | %s                       ",
			_Flash_Packet.id);
	cli_printf(cli, " *Timestamp        | %02d:%02d:%02d           ",
			gTime.Hours, gTime.Minutes, gTime.Seconds);
	cli_printf(cli, " *Date             | %02d.%02d.%02d           ",
			gDate.Date, gDate.Month, gDate.Year);
	cli_printf(cli, " *Location         | %s                       ",
			_Flash_Packet.location);
	cli_printf(cli, " *Filename         | %s                       ",
			_Flash_Packet.filename_ver_date);
	cli_printf(cli, " *Fileformat       | %s                       ",
			_Flash_Packet.File_Format);
	cli_printf(cli, " *Group            | %s                       ",
			_Flash_Packet.group);
	cli_printf(cli, " *Interval         | %dmin                       ",
			_Flash_Packet.Time_Interval);
	cli_printf(cli, " *Co2 sensor status| %s                       ",
			co2_Sensor_stat);
	cli_printf(cli, " *Calibration Value| %d                       ",
			calib_val);
	cli_printf(cli, " *Auto calibration | %d                       ",
			calib_stat);
	cli_printf(cli, " *Temp offset      | %d                       ",
			temp_offset);
	cli_printf(cli, " *Altitude Comp    | %d                       ",
			altitude_comp);
	cli_printf(cli, " *Co2 Samples      | %d                       ",
			_Flash_Packet.co2_samples);
#ifdef use_scd40x
	cli_printf(cli, " *Co2              | %d                       ",
			_RunTime_Packet.co2);
	cli_printf(cli, " *Temperature      | %dC                      ",
			_RunTime_Packet.temperature);
	cli_printf(cli, " *Humidity         | %d%%                     ",
			_RunTime_Packet.humidity);
#elif use_scd30
	cli_printf(cli, " *Co2              | %.2f                       ",
			_RunTime_Packet.co2);
	cli_printf(cli, " *Temperature      | %.2fC                      ",
			_RunTime_Packet.temperature);
	cli_printf(cli, " *Humidity         | %.2f%%                     ",
			_RunTime_Packet.humidity);
#endif

	cli_printf(cli, " *PIR              | %s                       ",
			_RunTime_Packet.motion_detection == 1 ? "TRUE" : "FALSE");
	cli_printf(cli, " *Pm2.5            | %s                       ", buff_pm);
	cli_printf(cli, " *Port             | Inactive                 ");
	cli_printf(cli, " *Battery          | %0.2f                    ",
			_RunTime_Packet.battery_voltage);
	cli_printf(cli, " *Last_Packet      | [Date:%02d.%02d.%02d] [Time: %02d:%02d:%02d] [Filename: %s] [Packet Status: %s]",
			fileWrite_day, fileWrite_month, fileWrite_year, fileWrite_hour,
			fileWrite_min, fileWrite_sec, _Flash_Packet.filename_ver_date,
			buff_filewritten);
	cli_printf(cli, " *Scheduled_Packet | [Time: %s]               ",
			scheduled_packet);

	cli_printf(cli, "");
	cli_printf(cli, "");
	cli_printf(cli, "");
}

void systemversion(EmbeddedCli *cli, char *args, void *context) {


	cli_printf(cli, "");
		cli_printf(cli, "");
		cli_printf(cli, "");

		cli_printf(cli, " *Hardware | TF410 Version 1.3                       ");
		cli_printf(cli, " *Software | TF410 Version 1.5                       ");

		cli_printf(cli, "");
			cli_printf(cli, "");
			cli_printf(cli, "");

}

void aboutDev(EmbeddedCli *cli, char *args, void *context)
{
	  cli_printf(cli, "");
	cli_printf(cli, "");
	cli_printf(cli, "This product is powered by BuildUp! "); //TM 0x99
	cli_printf(cli, "");
	cli_printf(cli, "Thingsfactory Inc. All rights reserved "); //copyright 0xA9
	cli_printf(cli, "");
	cli_printf(cli, "Montreal, Canada");
	cli_printf(cli, "");
	cli_printf(cli, "https://thingsfactory.com");
	cli_printf(cli, "");
	cli_printf(cli, "");
}
/**
 * Initializes the Embedded CLI instance and sets up command bindings.
 */
void initializeEmbeddedCli() {
	cli = embeddedCliNewDefault();

	CliCommandBinding clear_binding = { .name = "clear", .help =
			"Clears the console", .tokenizeArgs = true, .context = NULL,
			.binding = onClearCLI };

	CliCommandBinding Debug_scd4x_PM25 = { .name = "debug", .help =
			"Enable/Disable debug messages for scd4x and PM sensor",
			.tokenizeArgs = true, .context = NULL, .binding = debug_scd4x_PM25 };

	CliCommandBinding Get_DeviceSignature = { .name = "get-device-signature",
			.help = "Unique device identifier(from STM32 electronic signature)",
			.tokenizeArgs = true, .context = NULL, .binding = DeviceSignature };

	CliCommandBinding Set_ID = { .name = "set-id", .help =
			"Asset number assigned to the device", .tokenizeArgs = true,
			.context = NULL, .binding = SetID };

	CliCommandBinding Set_Location = { .name = "set-location", .help =
			"Optional field to assign a physical location to the device",
			.tokenizeArgs = true, .context = NULL, .binding = SetLocation };

//	CliCommandBinding Set_name = { .name = "set-name", .help =
//				"Optional field to assign name to the device", .tokenizeArgs = true, .context = NULL,
//				.binding = Setname };

	CliCommandBinding Set_group = { .name = "set-group", .help =
			"Optional field to assign group to the device",
			.tokenizeArgs = true, .context = NULL, .binding = Setgroup };

	CliCommandBinding Get_ID = { .name = "get-id", .help = "gets ID",
			.tokenizeArgs = true, .context = NULL, .binding = GetID };

	CliCommandBinding Get_Location = { .name = "get-location", .help =
			"gets Location", .tokenizeArgs = true, .context = NULL, .binding =
			GetLocation };

//	CliCommandBinding Get_name = { .name = "get-name", .help =
//				"gets name", .tokenizeArgs = true, .context = NULL,
//				.binding = Getname };

	CliCommandBinding Get_group = { .name = "get-group", .help = "gets group",
			.tokenizeArgs = true, .context = NULL, .binding = Getgroup };

	CliCommandBinding Set_Date = { .name = "set-date", .help =
			"In day:month:year", .tokenizeArgs = true, .context = NULL,
			.binding = SetDate };

	CliCommandBinding Set_Time = { .name = "set-time", .help = "In hh:mm:ss",
			.tokenizeArgs = true, .context = NULL, .binding = SetTime };

	CliCommandBinding Get_Date = { .name = "get-date", .help =
			"Get Systems Date", .tokenizeArgs = true, .context = NULL,
			.binding = GetDate };

	CliCommandBinding Get_Time = { .name = "get-time", .help =
			"Get Systems Time", .tokenizeArgs = true, .context = NULL,
			.binding = GetTime };

	CliCommandBinding Set_Interval = { .name = "set-interval", .help =
			"Set Time Interval for fetching the data", .tokenizeArgs = true,
			.context = NULL, .binding = SetInterval };

	CliCommandBinding Get_Interval = { .name = "get-interval", .help =
			"Get Time Interval", .tokenizeArgs = true, .context = NULL,
			.binding = GetInterval };

	CliCommandBinding Set_Filename = { .name = "set-filename", .help =
			"Filename prefix as in \"value+date\" in file naming convention",
			.tokenizeArgs = true, .context = NULL, .binding = SetFilename };

	CliCommandBinding Get_Filename = { .name = "get-filename", .help =
			"Get filename", .tokenizeArgs = true, .context = NULL, .binding =
			GetFilename };

	CliCommandBinding Set_Fileformat = { .name = "set-fileformat", .help =
			"Set desired file format", .tokenizeArgs = true, .context = NULL,
			.binding = SetFileformat };

	CliCommandBinding Get_Fileformat = { .name = "get-fileformat", .help =
			"Get file format", .tokenizeArgs = true, .context = NULL, .binding =
			GetFileformat };

	CliCommandBinding Fan_Clean = { .name = "fan-clean", .help =
			"Particle sensor fan cleaning", .tokenizeArgs = true, .context =
			NULL, .binding = fanclean };

	CliCommandBinding Co2_Calibration = { .name = "co2-force-calibration", .help =
			"Force calibration of Co2 sensor", .tokenizeArgs = true, .context = NULL,
			.binding = co2calibrate };

	CliCommandBinding getCo2_Calibration = { .name = "get-co2-force-calibration-status", .help =
			"Get status of Force calibration of Co2 sensor", .tokenizeArgs = true, .context = NULL,
			.binding = statco2calibrate };


	CliCommandBinding Co2_AutoCalibration = { .name = "co2-auto-calibration", .help =
			"Enable/Disable auto calibration of Co2 sensor", .tokenizeArgs = true, .context = NULL,
			.binding = co2_auto_calibration };


	CliCommandBinding getCo2_AutoCalibration = { .name = "get-co2-auto-calibration", .help =
			"Get Co2 sensor auto calibration status ", .tokenizeArgs = true, .context = NULL,
			.binding = get_auto_calibration };

	CliCommandBinding Co2_Tempoffset = { .name = "set-co2-temp-offset", .help =
			"Set temperature offset of Co2 sensor", .tokenizeArgs = true, .context = NULL,
			.binding = set_co2_temp_offset };

	CliCommandBinding getCo2_Tempoffset = { .name = "get-co2-temp-offset", .help =
			"Get temperature offset of Co2 sensor", .tokenizeArgs = true, .context = NULL,
			.binding = get_co2_temp_offset };

	CliCommandBinding Co2_altitudecompsentation = { .name = "set-co2-altitude-comp", .help =
			"Set altitude compensation of Co2 sensor", .tokenizeArgs = true, .context = NULL,
			.binding = set_co2_altitude_compensation };

	CliCommandBinding getCo2_altitudecompsentation = { .name = "get-co2-altitude-comp", .help =
			"Get altitude compensation of Co2 sensor", .tokenizeArgs = true, .context = NULL,
			.binding = get_co2_altitude_compensation };

	CliCommandBinding Co2_Calibration_program = { .name = "run-co2-calib-prog", .help =
			"Run Co2 sensor calibration program", .tokenizeArgs = true, .context = NULL,
			.binding = run_co2_calibration_routine };

	CliCommandBinding set_Co2_samples= { .name = "set-co2-samples", .help =
			"Set how much samples you want Co2 sensor to take", .tokenizeArgs = true, .context = NULL,
			.binding = set_co2_samples };

	CliCommandBinding get_Co2_samples= { .name = "get-co2-samples", .help =
			"Get Co2 samples", .tokenizeArgs = true, .context = NULL,
			.binding = get_co2_samples };

	CliCommandBinding Systemreset = { .name = "system-restart", .help =
			"Restart the system", .tokenizeArgs = true, .context = NULL,
			.binding = SystemRestart };

	CliCommandBinding Co2_Level = { .name = "co2-level", .help =
			"Co2 measured in ppm", .tokenizeArgs = true, .context = NULL,
			.binding = Co2Level };

	CliCommandBinding Temp_Level = { .name = "temperature-level", .help =
			"Temperature measured in Celsius", .tokenizeArgs = true, .context =
			NULL, .binding = TempLevel };

	CliCommandBinding Humid_Level = { .name = "humidity-level", .help =
			"Humidity measured in % of relative", .tokenizeArgs = true,
			.context = NULL, .binding = HumidLevel };

	CliCommandBinding Motion_Detection = { .name = "motion-detection", .help =
			"Movement detected - yes or no", .tokenizeArgs = true, .context =
			NULL, .binding = MotionDetection };

	CliCommandBinding Air_Quality = { .name = "air-quality", .help =
			"Check air quality level", .tokenizeArgs = true, .context = NULL,
			.binding = AirQuality };

	CliCommandBinding Battery_Voltage = { .name = "battery-voltage", .help =
			"Measure voltage from 4.17v(Full Charge) to 3.0v(Battery Low)",
			.tokenizeArgs = true, .context = NULL, .binding = BattVolt };

	CliCommandBinding _port = { .name = "port", .help = "Inactive field",
			.tokenizeArgs = true, .context = NULL, .binding = port };

	CliCommandBinding _showall = { .name = "show-all", .help =
			"Shows all parameters", .tokenizeArgs = true, .context = NULL,
			.binding = showall };

	CliCommandBinding getversion = { .name = "get-version", .help =
				"Shows hardware and software version", .tokenizeArgs = true, .context = NULL,
				.binding = systemversion };

	CliCommandBinding aboutdev = { .name = "about", .help =
				"Information about device", .tokenizeArgs = true, .context = NULL,
				.binding = aboutDev };

	// EmbeddedCli *cli = getCliPointer;debug_scd4x_PM25
	embeddedCliAddBinding(cli, clear_binding);
	embeddedCliAddBinding(cli, Debug_scd4x_PM25);
	embeddedCliAddBinding(cli, Get_DeviceSignature);
	embeddedCliAddBinding(cli, Set_ID);
	embeddedCliAddBinding(cli, Set_Location);
//	embeddedCliAddBinding(cli, Set_name);
	embeddedCliAddBinding(cli, Set_group);
	embeddedCliAddBinding(cli, Get_ID);
	embeddedCliAddBinding(cli, Get_Location);
//	embeddedCliAddBinding(cli, Get_name);
	embeddedCliAddBinding(cli, Get_group);
	embeddedCliAddBinding(cli, Set_Date);
	embeddedCliAddBinding(cli, Set_Time);
	embeddedCliAddBinding(cli, Get_Date);
	embeddedCliAddBinding(cli, Get_Time);
	embeddedCliAddBinding(cli, Set_Interval);
	embeddedCliAddBinding(cli, Get_Interval);
	embeddedCliAddBinding(cli, Set_Filename);
	embeddedCliAddBinding(cli, Get_Filename);
	embeddedCliAddBinding(cli, Set_Fileformat);
	embeddedCliAddBinding(cli, Get_Fileformat);
	embeddedCliAddBinding(cli, Fan_Clean);
	embeddedCliAddBinding(cli, Co2_Calibration);
	embeddedCliAddBinding(cli, getCo2_Calibration);
	embeddedCliAddBinding(cli, Co2_AutoCalibration);
	embeddedCliAddBinding(cli, getCo2_AutoCalibration);
	embeddedCliAddBinding(cli, Co2_Tempoffset);
	embeddedCliAddBinding(cli, getCo2_Tempoffset);
	embeddedCliAddBinding(cli, Co2_altitudecompsentation);
	embeddedCliAddBinding(cli, getCo2_altitudecompsentation);
	embeddedCliAddBinding(cli, Co2_Calibration_program);
	embeddedCliAddBinding(cli, set_Co2_samples);
	embeddedCliAddBinding(cli, get_Co2_samples);
	embeddedCliAddBinding(cli, Systemreset);
	embeddedCliAddBinding(cli, Co2_Level);
	embeddedCliAddBinding(cli, Temp_Level);
	embeddedCliAddBinding(cli, Humid_Level);
	embeddedCliAddBinding(cli, Motion_Detection);
	embeddedCliAddBinding(cli, Air_Quality);
	embeddedCliAddBinding(cli, Battery_Voltage);
	embeddedCliAddBinding(cli, _port);
	embeddedCliAddBinding(cli, _showall);
	embeddedCliAddBinding(cli, getversion);
	embeddedCliAddBinding(cli, aboutdev);
	// Assign character write function
	cli->writeChar = writeCharToCli;
	// cli->onCommand = onCommand;
	// CLI has now been initialized, set bool to true to enable interrupts.
	cliIsReady = true;
	cli_other = 0;
}

void console_process(void) {
	embeddedCliProcess(cli);
}

void console_uart_deinit() {
	HAL_UART_DeInit(&huart1);
	HAL_UART_MspDeInit(&huart1);
}
void console_uart_init() {
	HAL_UART_DeInit(&huart1);
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart1) != HAL_OK) {
		Error_Handler();
	}
	HAL_UART_Receive_IT(UART_CLI_PERIPH, UART_CLI_rxBuffer, UART_RX_BUFF_SIZE);
}

void console_init(void) {
	console_uart_init();
	initializeEmbeddedCli();
	HAL_UART_Receive_IT(UART_CLI_PERIPH, UART_CLI_rxBuffer, UART_RX_BUFF_SIZE);
}
