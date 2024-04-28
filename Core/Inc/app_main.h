/*
 * app_main.h
 *
 *  Created on: Dec 23, 2023
 *      Author: hzaib
 */

#ifndef INC_APP_MAIN_H_
#define INC_APP_MAIN_H_







#include "main.h"
#include "csv_json_handling.h"
#include <File_Handling.h>


/*uncomment which one ever you want to use and comment the other one*/
//#define use_scd40x 1
#define use_scd30  1


#define BLUE_LED_TOGGLE() HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin);

#define disable_5v()  HAL_GPIO_WritePin(EN_5V_GPIO_Port, EN_5V_Pin, GPIO_PIN_RESET);
#define enable_5v()   HAL_GPIO_WritePin(EN_5V_GPIO_Port, EN_5V_Pin, GPIO_PIN_SET);

#define disable_motion()   HAL_GPIO_WritePin(EN_MOTION_GPIO_Port, EN_MOTION_Pin, GPIO_PIN_SET);
#define enable_motion()   HAL_GPIO_WritePin(EN_MOTION_GPIO_Port, EN_MOTION_Pin, GPIO_PIN_RESET);

#define ID_DBGMCU_IDCODE		0xE0042000
#define _ID_GetSignature()    ((*(uint16_t *) (ID_DBGMCU_IDCODE)) & 0x0FFF)
#define Address_Mem 0x08060000 //we will be using last sector as an memory to store our packet
                               //not a good practice as last sector is 128kb but for now it will work
typedef struct PACKED{
  char File_Name[100];
  char File_Format[100];
  uint32_t Time_Interval; //its in minutes
  uint8_t Calibration_Status;
  char id[50];
  char location[50];
  char name[50];
  char group[50];
  char filename_with_format[100];
  char filename_ver_date[100];
  uint16_t co2_samples;
  uint16_t valid_pattern; //this is to verify if flash data is valid
} Flash_Packet;

extern Flash_Packet _Flash_Packet;


typedef struct
{

#ifdef use_scd40x
	uint16_t co2;
    int32_t temperature;
    int32_t humidity;
#elif use_scd30
	float co2;
    float temperature;
    float humidity;
#endif
    uint8_t motion_detection;
    float pm1_0;
    float pm2_5;
    float pm4_0;
    float pm10_0;
    float battery_voltage;
    uint8_t showPrompt;
    uint8_t filename_changed;
    uint8_t fileformat_changed;
    uint8_t fileformat_selection;
    uint8_t prev_day;
    uint16_t prev_year;
    uint8_t day_changed;
    uint8_t year_changed;
    uint8_t sps30_uart_error;
    uint8_t console_uart_error;
    uint8_t scd4x_i2c_error;
    uint8_t usb_detection;
    uint8_t usb_active_disable_pm;
    uint8_t usb_first_start;
    uint8_t usb_start_timer;
    int8_t sd_file_creation;
    int8_t sd_card_disk_write_error;
    uint8_t PM_calibration;
    uint8_t CO2_calibration;
    uint8_t temp_offset;
    uint8_t altitude_comp;
    uint16_t _target_co2_concentration;
    uint16_t _tempOffset_co2_concentration;
    uint16_t _altitudeComp_co2_concentration;
    uint16_t _frc_correction;
    uint8_t pwr_off_det;
    uint8_t day;
    uint8_t month;
    uint8_t year;

}RunTime_Packet;

extern RunTime_Packet _RunTime_Packet;



extern uint8_t debug_scd_pm;
extern uint8_t save_param;
extern uint8_t start_timer;
extern uint32_t time_keep;
extern uint32_t usb_time_keep ;
extern uint8_t rx_Wait;

#define set_param_flags() save_param=1;\
	                    start_timer=1;\


extern uint8_t pir_motion_flag;
extern uint8_t RX_Data[522];

extern uint32_t sleep_time;
extern uint32_t prev_sleep_time;

extern uint32_t prev_usb_time_;
extern uint8_t stop_measurement;

extern uint8_t RetainState  ;


extern void blink_red();
#ifdef use_scd40x
extern  void get_sps30_measurement();
extern void get_scd4x_measurement();
#elif use_scd30
extern void get_scd30_measurement();
extern void get_scd30_measurement_();
#endif
extern void toggle_blue_led();
extern void power_off_detect();
int16_t sensirion_uart_receive(uint8_t* data);
extern void pir_interrupt();
extern void timer_interrupt();
extern char *ver_GetUid();
extern void run_console_from_scd30();


extern void app_main();


#endif /* INC_APP_MAIN_H_ */
