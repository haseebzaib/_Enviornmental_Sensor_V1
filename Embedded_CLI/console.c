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
#include "string.h"
#include <ctype.h>
#include "Uhr.h"
#include "Utils.h"
#include "sensirion_common.h"
#include "sensirion_i2c_hal.h"
#include "sensirion_uart.h"
#include "sps30.h"
#include "internal_adc.h"
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

uint32_t max_wait_time_motion = 20000;
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
uint8_t invoke_time =0;
uint8_t invoke_date =0;

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

	prev_max_wait_time = HAL_GetTick();
	while (1) {
		if (flag_cli) {
			if (UART_CLI_rxBuffer[0] == '\x0d' || index == bufferSize - 1) {
				// Exit the loop when Enter key is pressed or buffer is full
				buffer[index] = '\0'; // Null-terminate the string
				flag_cli = 0;
				// Convert the entered string to lowercase
				for (size_t i = 0; buffer[i]; i++) {
					buffer[i] = tolower(buffer[i]);
				}
				break;
			} else {
				// Check if the received character is an alphabet character
				buffer[index++] = UART_CLI_rxBuffer[0];
				cli->writeChar(cli, UART_CLI_rxBuffer[0]);
			}
			flag_cli = 0;
		}

		if(HAL_GetTick() - prev_max_wait_time > max_wait_time)
		{
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
	cli_printf(cli,"Do you want to enable debug for scd4x and PM y/n?");

	cli_other = 1;
		flag_cli = 0;
		char buffer[50];
		receiveString(cli,buffer,sizeof(buffer));
		HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t *)newLine,  strlen(newLine), 1000);


		if(strstr(buffer, "y"))
		{
			debug_scd_pm=1;
			cli_printf(cli,"Debug enabled");
		}
		else if(strstr(buffer, "n"))
		{
			debug_scd_pm=0;
			cli_printf(cli,"Debug disabled");
		}


		flag_cli = 0;
		cli_other = 0;
}


void DeviceSignature(EmbeddedCli *cli, char *args, void *context) {

//	cli_printf(cli,"Device ID/Signature: TF241_0x%04X.",_ID_GetSignature());

	cli_printf(cli,"Device ID/Signature: TF241_%s.",	ver_GetUid());
}

void SetID(EmbeddedCli *cli, char *args, void *context) {
	uint16_t len;
    const char *newLine = "\r\n";
	cli_printf(cli,"Set the ID. Keep ID below 30 characters");
	cli_other = 1;
	flag_cli = 0;
	char buffer[50];
	receiveString(cli,buffer,sizeof(buffer));
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t *)newLine,  strlen(newLine), 1000);

	len = strlen(buffer);
	if(len < 30)
	{
	   if(buffer[0] != '\0' )
		{if(buffer != NULL)
		{
			strcpy(_Flash_Packet.id,buffer);
			set_param_flags();
			cli_printf(cli,"ID is set to: %s",_Flash_Packet.id);
		}
		}
	}
	else
	{
		cli_printf(cli,"ID is too long");
	}

	flag_cli = 0;
	cli_other = 0;

}

void SetLocation(EmbeddedCli *cli, char *args, void *context) {
	uint16_t len;
    const char *newLine = "\r\n";
	cli_printf(cli,"Set the Location. Keep Location below 15 characters");
	cli_other = 1;
	flag_cli = 0;
	char buffer[50];
	receiveString(cli,buffer,sizeof(buffer));
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t *)newLine,  strlen(newLine), 1000);

	len = strlen(buffer);
	if(len < 15)
	{
		if(buffer[0] != '\0' )
		   {
		if(buffer != NULL)
		{
			strcpy(_Flash_Packet.location,buffer);
			set_param_flags();
			cli_printf(cli,"Location is set to: %s",_Flash_Packet.location);
		}
		   }
	}
	else
	{
		cli_printf(cli,"Location is too long");
	}

	flag_cli = 0;
	cli_other = 0;

}

void Setname(EmbeddedCli *cli, char *args, void *context) {
	uint16_t len;
    const char *newLine = "\r\n";
	cli_printf(cli,"Set the name. Keep name below 15 characters");
	cli_other = 1;
	flag_cli = 0;
	char buffer[50];
	receiveString(cli,buffer,sizeof(buffer));
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t *)newLine,  strlen(newLine), 1000);

	len = strlen(buffer);
	if(len < 15)
	{
		   if(buffer[0] != '\0' )
		   {
		if(buffer != NULL)
		{
			strcpy(_Flash_Packet.name,buffer);
			set_param_flags();
			cli_printf(cli,"name is set to: %s",_Flash_Packet.name);
		}
		   }
	}
	else
	{
		cli_printf(cli,"name is too long");
	}

	flag_cli = 0;
	cli_other = 0;

}

void Setgroup(EmbeddedCli *cli, char *args, void *context) {
	uint16_t len;
    const char *newLine = "\r\n";
	cli_printf(cli,"Set the group. Keep name below 15 characters");
	cli_other = 1;
	flag_cli = 0;
	char buffer[50];
	receiveString(cli,buffer,sizeof(buffer));
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t *)newLine,  strlen(newLine), 1000);

	len = strlen(buffer);
	if(len < 15)
	{
		   if(buffer[0] != '\0' )
		   {
		if(buffer != NULL)
		{
			strcpy(_Flash_Packet.group,buffer);
			set_param_flags();
			cli_printf(cli,"group is set to: %s",_Flash_Packet.group);
		}
		   }
	}
	else
	{
		cli_printf(cli,"group is too long");
	}

	flag_cli = 0;
	cli_other = 0;

}



void SetDate(EmbeddedCli *cli, char *args, void *context) {
    const char *newLine = "\r\n";
	cli_printf(cli,"Set date in following format: day:month:year eg:(15:1:23).");
	cli_other = 1;
	flag_cli = 0;
	char buffer[50];
	receiveString(cli,buffer,sizeof(buffer));
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t *)newLine,  strlen(newLine), 1000);
	if(buffer[0] != '\0' )
	{
	Set_Date(buffer);
	}
	flag_cli = 0;
	cli_other = 0;

	RTC_DateTypeDef gDate;
	HAL_RTC_GetDate(RTC_Handle, &gDate, RTC_FORMAT_BIN);
	RTC_TimeTypeDef gTime;
	HAL_RTC_GetTime(RTC_Handle, &gTime, RTC_FORMAT_BIN);
	cli_printf(cli,"Set Date: %02d.%02d.%02d",gDate.Date,gDate.Month,gDate.Year);

}

void SetTime(EmbeddedCli *cli, char *args, void *context) {
    const char *newLine = "\r\n";
	cli_printf(cli,"Set time in following format: hours:minutes:seconds eg:(11:15:01).");
	cli_other = 1;
	flag_cli = 0;
	char buffer[50];
	receiveString(cli,buffer,sizeof(buffer));
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t *)newLine,  strlen(newLine), 1000);
	if(buffer[0] != '\0' )
	{
	Set_Time(buffer);
    }
	flag_cli = 0;
	cli_other = 0;
	set_alarm_Time = 1;

	RTC_DateTypeDef gDate;
	HAL_RTC_GetDate(RTC_Handle, &gDate, RTC_FORMAT_BIN);
	RTC_TimeTypeDef gTime;
	HAL_RTC_GetTime(RTC_Handle, &gTime, RTC_FORMAT_BIN);
	cli_printf(cli,"Set Time: %02d:%02d:%02d",gTime.Hours,gTime.Minutes,gTime.Seconds);

}

void GetDate(EmbeddedCli *cli, char *args, void *context) {



	RTC_DateTypeDef gDate;
	HAL_RTC_GetDate(RTC_Handle, &gDate, RTC_FORMAT_BIN);
	RTC_TimeTypeDef gTime;
	HAL_RTC_GetTime(RTC_Handle, &gTime, RTC_FORMAT_BIN);
	cli_printf(cli,"%02d.%02d.%02d",gDate.Date,gDate.Month,gDate.Year);
	HAL_Delay(100);
}

//for some reason if you dont read date the time does not update

void GetTime(EmbeddedCli *cli, char *args, void *context) {


	RTC_DateTypeDef gDate;
	HAL_RTC_GetDate(RTC_Handle, &gDate, RTC_FORMAT_BIN);
	RTC_TimeTypeDef gTime;
	HAL_RTC_GetTime(RTC_Handle, &gTime, RTC_FORMAT_BIN);
	cli_printf(cli,"%02d:%02d:%02d",gTime.Hours,gTime.Minutes,gTime.Seconds);
	HAL_Delay(100);
}

void SetInterval(EmbeddedCli *cli, char *args, void *context) {
	  const char *newLine = "\r\n";
	cli_printf(cli,"Enter 1 for 5min.");
	cli_printf(cli,"Enter 2 for 15min.");
	cli_printf(cli,"Enter 3 for 60min.");
	cli_printf(cli,"Disclaimer: The battery life will be reduced if interval is reduced.");
	cli_other = 1;
	flag_cli = 0;
	char buffer[50];
	receiveString(cli,buffer,sizeof(buffer));
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t *)newLine,  strlen(newLine), 1000);


	if(strstr(buffer, "1"))
	{
        _Flash_Packet.Time_Interval = 5;
        set_param_flags();
	}
	else if(strstr(buffer, "2"))
	{
		_Flash_Packet.Time_Interval = 15;
		set_param_flags();
	}
	else if(strstr(buffer, "3"))
	{
		_Flash_Packet.Time_Interval = 60;
		set_param_flags();
	}

	flag_cli = 0;
	cli_other = 0;
	cli_printf(cli,"Interval is set to: %ldmin",_Flash_Packet.Time_Interval);
	set_alarm_Time = 1;
}

void GetInterval(EmbeddedCli *cli, char *args, void *context) {

	cli_printf(cli,"Interval is Set to: %ldmin",_Flash_Packet.Time_Interval);

}

void SetFilename(EmbeddedCli *cli, char *args, void *context) {
uint16_t len;
	  const char *newLine = "\r\n";
	cli_printf(cli,"To set file name dont use special characters or space, '_' can be used and please keep the name below 18characters.");

	cli_other = 1;
	flag_cli = 0;
	char buffer[50];
	receiveString(cli,buffer,sizeof(buffer));
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t *)newLine,  strlen(newLine), 1000);

len = strlen(buffer);
if(len < 20)
{
	   if(buffer[0] != '\0' )
	   {
	if(buffer != NULL)
	{
		strcpy(_Flash_Packet.File_Name,buffer);
		set_param_flags();
		 _RunTime_Packet.filename_changed = 1;
			cli_printf(cli,"Filename is set to: %s",_Flash_Packet.File_Name);
	}
	   }
}
else
{
	cli_printf(cli,"Filename is too long");
}


	flag_cli = 0;
	cli_other = 0;


}

void GetFilename(EmbeddedCli *cli, char *args, void *context) {

	cli_printf(cli,"Filename is set to: %s",_Flash_Packet.File_Name);

}

void SetFileformat(EmbeddedCli *cli, char *args, void *context) {

	  const char *newLine = "\r\n";
	cli_printf(cli,"Following file formats are allowed:");
	cli_printf(cli,"1. (.csv)");
	cli_printf(cli,"2. (.json)");
	cli_printf(cli,"Enter the number to select your desired format");

	cli_other = 1;
	flag_cli = 0;
	char buffer[50];
	receiveString(cli,buffer,sizeof(buffer));
	HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t *)newLine,  strlen(newLine), 1000);

	if(strstr(buffer, "1"))
	{
		strcpy(_Flash_Packet.File_Format,".CSV");
		set_param_flags();
		 _RunTime_Packet.fileformat_changed = 1;
			_RunTime_Packet.fileformat_selection = 1;
	}
	else if(strstr(buffer, "2"))
	{
		strcpy(_Flash_Packet.File_Format,".JSON");
		set_param_flags();
		 _RunTime_Packet.fileformat_changed = 1;
			_RunTime_Packet.fileformat_selection = 0;
	}


	flag_cli = 0;
	cli_other = 0;
	cli_printf(cli,"File format is set to: %s",_Flash_Packet.File_Format);

}

void GetFileformat(EmbeddedCli *cli, char *args, void *context) {

	cli_printf(cli,"File format is set to: %s",_Flash_Packet.File_Format);

}

void Devicecalibrate(EmbeddedCli *cli, char *args, void *context) {
	  const char *newLine = "\r\n";
	  uint32_t val;
	  int counter = 0;
	  const char *dot = ".";
	  uint8_t do_Calibration = 3;
	cli_printf(cli,"Do you want to Calibrate Particle sensor y/n?");

	cli_other = 1;
		flag_cli = 0;
		char buffer[50];
		receiveString(cli,buffer,sizeof(buffer));
		HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t *)newLine,  strlen(newLine), 1000);


        if(strstr(buffer, "n"))
		{
			_Flash_Packet.Calibration_Status=0;
			cli_printf(cli,"Particle sensor Calibration aborted.");
			//set_param_flags();
			do_Calibration &= ~1;
		}
        else if(strstr(buffer, "y"))
        {
        	do_Calibration |= 1;
        }

		memset(buffer,'\0',sizeof(buffer));
		HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t *)newLine,  strlen(newLine), 1000);
		cli_printf(cli,"To calibrate Co2 sensor, please provide Co2 concentration value.");
		cli_printf(cli,"If Co2 concentration is unknown then just press enter to cancel the calibration.");

		receiveString(cli,buffer,sizeof(buffer));
		HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t *)newLine,  strlen(newLine), 1000);

		   if(buffer[0] != '\0' )
			{
			   _RunTime_Packet.CO2_calibration = 1;

                val = atoi(buffer);

                if(val < 65535)
                {
                   _RunTime_Packet._target_co2_concentration = val;
                }
            	cli_printf(cli,"Co2 sensor is going to be Calibrated now. Co2 concentration value provided : %d",_RunTime_Packet._target_co2_concentration);
            	do_Calibration |= 2;
			 }
		   else
		   {
				cli_printf(cli,"Co2 sensor Calibration aborted.");
				do_Calibration &= ~2;
		   }

		   if(do_Calibration > 0)
		   {
				HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t *)newLine,  strlen(newLine), 1000);
				cli_printf(cli,"Sensors are calibrating, please wait.");
		   while(counter < 20 )
		   {
				HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t *)dot,  strlen(dot), 1000);
                HAL_Delay(20);
                counter++;
		   }

			HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t *)newLine,  strlen(newLine), 1000);
			cli_printf(cli,"Sensor calibration done.");
		   }

		flag_cli = 0;
		cli_other = 0;
}

void SystemRestart(EmbeddedCli *cli, char *args, void *context) {
	  const char *newLine = "\r\n";
	cli_printf(cli,"Do you want to restart the device y/n?");

	cli_other = 1;
		flag_cli = 0;
		char buffer[50];
		receiveString(cli,buffer,sizeof(buffer));
		HAL_UART_Transmit(UART_CLI_PERIPH, (uint8_t *)newLine,  strlen(newLine), 1000);


		if(strstr(buffer, "y"))
		{
			cli_printf(cli,"Device is going to restart.");
			HAL_NVIC_SystemReset();
		}
		else if(strstr(buffer, "n"))
		{
			cli_printf(cli,"Device restart aborted.");
		}


		flag_cli = 0;
		cli_other = 0;
}

void Co2Level(EmbeddedCli *cli, char *args, void *context) {

	cli_printf(cli,"Co2 Value: %d",_RunTime_Packet.co2);
}

void TempLevel(EmbeddedCli *cli, char *args, void *context) {

	cli_printf(cli,"Temperature Value: %dC ",_RunTime_Packet.temperature);
}

void HumidLevel(EmbeddedCli *cli, char *args, void *context) {

	cli_printf(cli,"Humidity Value: %d ",_RunTime_Packet.humidity);
}

uint8_t prev_motion;

void MotionDetection(EmbeddedCli *cli, char *args, void *context) {

	cli_printf(cli,"For 20seconds system will poll here to detect motion.");
	HAL_Delay(1500);
         prev_max_wait_time_motion = HAL_GetTick();
         prev_motion = 0;
	while(HAL_GetTick() - prev_max_wait_time_motion < max_wait_time_motion)
	{

		if (HAL_GPIO_ReadPin(OUT_MOTION_GPIO_Port, OUT_MOTION_Pin)) {
				_RunTime_Packet.motion_detection = 1;
				//cli_printf(cli,"PIR Motion: %d\r\n",_RunTime_Packet.motion_detection);
			}
			else {
				_RunTime_Packet.motion_detection = 0;
			}

//		if(prev_motion != _RunTime_Packet.motion_detection )
//	         	{
//			prev_motion = _RunTime_Packet.motion_detection ;
			cli_printf(cli,"PIR Motion: %s\r\n",_RunTime_Packet.motion_detection ? "TRUE" : "FALSE");
//	         	}
HAL_Delay(100);

	}

	cli_printf(cli,"Session ended");

}

void AirQuality(EmbeddedCli *cli, char *args, void *context) {

	cli_printf(cli,"AirQuality Measured Values:");
	cli_printf(cli,"%0.2f pm1.0", _RunTime_Packet.pm1_0);
	cli_printf(cli,"%0.2f pm2.5 ",_RunTime_Packet.pm2_5);
	cli_printf(cli,"%0.2f pm4.0 ",_RunTime_Packet.pm4_0);
	cli_printf(cli,"%0.2f pm10.0",_RunTime_Packet.pm10_0);


}

void BattVolt(EmbeddedCli *cli, char *args, void *context) {

	//float batteryVoltage;
	adc_Measure(&_RunTime_Packet.battery_voltage);
	cli_printf(cli,"Battery voltage: %0.2f",_RunTime_Packet.battery_voltage);


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
			"Enable/Disable debug messages for scd4x and PM sensor", .tokenizeArgs = true, .context = NULL,
			.binding = debug_scd4x_PM25 };

	CliCommandBinding Get_DeviceSignature = { .name = "get-Device-Signature", .help =
				"Device Signature-Electronic ID", .tokenizeArgs = true, .context = NULL,
				.binding = DeviceSignature };

	CliCommandBinding Set_ID = { .name = "set-id", .help =
				"Sets ID", .tokenizeArgs = true, .context = NULL,
				.binding = SetID };

	CliCommandBinding Set_Location = { .name = "set-location", .help =
				"Sets Location", .tokenizeArgs = true, .context = NULL,
				.binding = SetLocation };

	CliCommandBinding Set_name = { .name = "set-name", .help =
				"Sets name", .tokenizeArgs = true, .context = NULL,
				.binding = Setname };

	CliCommandBinding Set_group = { .name = "set-group", .help =
				"Sets group", .tokenizeArgs = true, .context = NULL,
				.binding = Setgroup };

	CliCommandBinding Set_Date = { .name = "set-date", .help =
			"Set Systems Date", .tokenizeArgs = true, .context = NULL,
			.binding = SetDate };

	CliCommandBinding Set_Time = { .name = "set-time", .help =
			"Set Systems Time", .tokenizeArgs = true, .context = NULL,
			.binding = SetTime };

	CliCommandBinding Get_Date = { .name = "get-date", .help =
			"Get Systems Date", .tokenizeArgs = true, .context = NULL,
			.binding = GetDate };

	CliCommandBinding Get_Time = { .name = "get-time", .help =
			"Get Systems Time", .tokenizeArgs = true, .context = NULL,
			.binding = GetTime };

	CliCommandBinding Set_Interval = { .name = "set-interval", .help =
			"Set Time Interval for fetching the data", .tokenizeArgs = true, .context = NULL,
			.binding = SetInterval };

	CliCommandBinding Get_Interval = { .name = "get-interval", .help =
			"Get Time Interval", .tokenizeArgs = true, .context = NULL,
			.binding = GetInterval };


	CliCommandBinding Set_Filename = { .name = "set-filename", .help =
			"Set desired filename", .tokenizeArgs = true, .context = NULL,
			.binding = SetFilename };

	CliCommandBinding Get_Filename = { .name = "get-filename", .help =
			"Get filename", .tokenizeArgs = true, .context = NULL,
			.binding = GetFilename };


	CliCommandBinding Set_Fileformat = { .name = "set-fileformat", .help =
			"Set desired file format", .tokenizeArgs = true, .context = NULL,
			.binding = SetFileformat };

	CliCommandBinding Get_Fileformat = { .name = "get-fileformat", .help =
			"Get file format", .tokenizeArgs = true, .context = NULL,
			.binding = GetFileformat };

	CliCommandBinding DeviceCalibration = { .name = "device-calibration", .help =
			"Calibrate the device", .tokenizeArgs = true, .context = NULL,
			.binding = Devicecalibrate };

	CliCommandBinding Systemreset = { .name = "system-restart", .help =
			"Restart the system", .tokenizeArgs = true, .context = NULL,
			.binding = SystemRestart };

	CliCommandBinding Co2_Level = { .name = "Co2-level", .help =
			"Check Co2 level", .tokenizeArgs = true, .context = NULL,
			.binding = Co2Level };

	CliCommandBinding Temp_Level = { .name = "temperature-level", .help =
			"Check Temperature level", .tokenizeArgs = true, .context = NULL,
			.binding = TempLevel };

	CliCommandBinding Humid_Level = { .name = "humidity-level", .help =
			"Check Humidity level", .tokenizeArgs = true, .context = NULL,
			.binding = HumidLevel };

	CliCommandBinding Motion_Detection = { .name = "motion-detection", .help =
			"Detect Motion", .tokenizeArgs = true, .context = NULL,
			.binding = MotionDetection };

	CliCommandBinding Air_Quality = { .name = "air-quality", .help =
				"Check air quality level", .tokenizeArgs = true, .context = NULL,
				.binding = AirQuality };

	CliCommandBinding Battery_Voltage = { .name = "battery-volt", .help =
				"Check battery voltage", .tokenizeArgs = true, .context = NULL,
				.binding = BattVolt };



	// EmbeddedCli *cli = getCliPointer;debug_scd4x_PM25
	embeddedCliAddBinding(cli, clear_binding);
	embeddedCliAddBinding(cli, Debug_scd4x_PM25);
	embeddedCliAddBinding(cli, Get_DeviceSignature);
	embeddedCliAddBinding(cli, Set_ID);
	embeddedCliAddBinding(cli, Set_Location);
	embeddedCliAddBinding(cli, Set_name);
	embeddedCliAddBinding(cli, Set_group);
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
	embeddedCliAddBinding(cli, DeviceCalibration);
	embeddedCliAddBinding(cli, Systemreset);
	embeddedCliAddBinding(cli, Co2_Level);
	embeddedCliAddBinding(cli, Temp_Level);
	embeddedCliAddBinding(cli, Humid_Level);
	embeddedCliAddBinding(cli, Motion_Detection);
	embeddedCliAddBinding(cli, Air_Quality);
	embeddedCliAddBinding(cli, Battery_Voltage);
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



void console_uart_deinit()
{
	HAL_UART_DeInit(&huart1);
	HAL_UART_MspDeInit(&huart1);
}
void console_uart_init()
{
	HAL_UART_DeInit(&huart1);
	  huart1.Instance = USART1;
	  huart1.Init.BaudRate = 115200;
	  huart1.Init.WordLength = UART_WORDLENGTH_8B;
	  huart1.Init.StopBits = UART_STOPBITS_1;
	  huart1.Init.Parity = UART_PARITY_NONE;
	  huart1.Init.Mode = UART_MODE_TX_RX;
	  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	  if (HAL_UART_Init(&huart1) != HAL_OK)
	  {
	    Error_Handler();
	  }
		HAL_UART_Receive_IT(UART_CLI_PERIPH, UART_CLI_rxBuffer, UART_RX_BUFF_SIZE);
}

void console_init(void) {
	console_uart_init();
	initializeEmbeddedCli();
	HAL_UART_Receive_IT(UART_CLI_PERIPH, UART_CLI_rxBuffer, UART_RX_BUFF_SIZE);
}
