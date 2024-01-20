/*
 * app_main.c
 *
 *  Created on: Dec 23, 2023
 *      Author: hzaib
 */

/**
 * TODO
 * mass storage speed up
 * mass storage malfunction fix needed
 * power off detection led need to be added
 */

#include "main.h"
#include "app_main.h"
#include "console.h"
#include "Uhr.h"
#include "stdio.h"
#include "string.h"
#include "scd4x_i2c.h"
#include "sensirion_common.h"
#include "sensirion_i2c_hal.h"
#include "sensirion_uart.h"
#include "sps30.h"
#include "fatfs.h"
#include "csv_json_handling.h"
#include "usb_device.h"
#include "internal_adc.h"
#include "Utils.h"
#include "File_Handling.h"

char buf_sdcard[] =
		"sdcard error, this can cause issue in mass storage also\r\n";

#define battery_Full 3.27f
#define battery_Low  2.43f
#define battery_very_low 2.25f

#define lengthSoftPWMbuffer 1000
uint32_t dataA[lengthSoftPWMbuffer];
static void setSoftPWM(uint16_t pin, uint32_t duty, uint32_t *softpwmbuffer) {
	for (uint32_t i = 0; i < lengthSoftPWMbuffer; ++i) {
		if (i < duty) { //set pin
			softpwmbuffer[i] &= (uint32_t) ~(pin << 16);
			softpwmbuffer[i] |= (uint32_t) pin;
		} else { //reset pin
			softpwmbuffer[i] &= (uint32_t) ~(pin);
			softpwmbuffer[i] |= (uint32_t) pin << 16;
		}
	}

}

#define disable_led 1000
#define GREEN_LED_PWM(x) TIM3->CCR1=x;
#define RED_LED_PWM(x)   TIM3->CCR2=x;
#define BLUE_LED_PWM(x)  setSoftPWM(BLUE_LED_Pin, x, (uint32_t*)&dataA);

uint8_t green_led_pwm_flag = 0;
uint8_t red_led_pwm_flag = 0;
uint8_t blue_led_pwm_flag = 0;
int16_t green_led_pwm_val = 0;
int16_t red_led_pwm_val = 0;
int16_t blue_led_pwm_val = 0;

Flash_Packet _Flash_Packet;
RunTime_Packet _RunTime_Packet;
const Flash_Packet m_Flash_Packet = { "Enviornment_Sensor", ".CSV", 15, 0,
		"0000000000000000", "default", "default", "default", 0x1840, };

uint8_t debug_scd_pm = 0;
uint8_t save_param = 0;
uint8_t start_timer = 0;
uint32_t time_keep = 0;
uint8_t start_usb_timer = 0;
uint32_t usb_time_keep = 0;
uint8_t pir_motion_flag = 0;
uint8_t debug_pir_motion = 0;
uint8_t RX_Data[522] = { 0 };
struct sps30_measurement m;

uint32_t sleep_time = 20000;
uint32_t prev_sleep_time = 0;

uint32_t sleep_time_pm_co2 = 12000;
uint32_t prev_sleep_time_pm_co2 = 0;

uint32_t usb_time_ = 50000;
uint32_t prev_usb_time_ = 0;

void toggle_blue_led()
{
		if(blue_led_pwm_val != 0)
    		{
			blue_led_pwm_val = 0;
    		}
    		else
    		{
    			blue_led_pwm_val = 1000;
    		}
    		BLUE_LED_PWM(blue_led_pwm_val); //we toggle red led
}

//##############Interrupts###############
void pir_interrupt() {
	debug_pir_motion = 1;
	if (HAL_GPIO_ReadPin(OUT_MOTION_GPIO_Port, OUT_MOTION_Pin)) {
		pir_motion_flag = 1;
	} else {
		pir_motion_flag = 0;
	}
}

void timer_interrupt() {
	time_keep = time_keep + 1;
	if (time_keep > 65535) {
		time_keep = 0;
	}

//	if(start_usb_timer)
//	{
//		usb_time_keep = usb_time_keep + 1;
//		if (usb_time_keep > 65535) {
//			usb_time_keep = 0;
//		}
//	}
//
//
//	if(!save_param && !start_usb_timer)//if both are off and for some reason timer is still on we shut it down
//	{
//		HAL_TIM_Base_Stop_IT(&htim2);
//	}
	prev_sleep_time = HAL_GetTick();
}

void power_off_detect() {
	if (!HAL_GPIO_ReadPin(SW_DET_GPIO_Port, SW_DET_Pin)) {
		//_RunTime_Packet.pwr_off_det = 1;
	}
}

//######################################

static void MX_DMA_Init(void) {
	/* DMA controller clock enable */
	__HAL_RCC_DMA2_CLK_ENABLE();

	/* DMA interrupt init */
	/* DMA2_Stream5_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA2_Stream5_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);

}

static void pwr_off_detected() {
	if (_RunTime_Packet.pwr_off_det) {
		_RunTime_Packet.pwr_off_det = 0;
		BLUE_LED_PWM(900);
		HAL_Delay(5000);

	}
}
/*
 * setting clock speed to 250khz
 */
static void clock_speed_slow() {

//	uint16_t timeout;
//
//	/* Enable HSI clock */
//	RCC->CR |= RCC_CR_HSION;
//
//	/* Wait till HSI is ready */
//	timeout = 0xFFFF;
//	while (!(RCC->CR & RCC_CR_HSIRDY) && timeout--);
//
//	/* Select HSI clock as main clock */
//	RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_SW)) | RCC_CFGR_SW_HSI;
//
//	/* Disable PLL */
//	RCC->CR &= ~RCC_CR_PLLON;

//we will set clock speed to 250khz
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI
			| RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV64;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
		Error_Handler();
	}

}

/**
 * setting clock speed to 84Mhz
 */
static void clock_speed_high() {
	//we will set clock speed to 84Mhz
	uint16_t timeout;

	/* Enable HSI clock */
	RCC->CR |= RCC_CR_HSION;

	/* Wait till HSI is ready */
	timeout = 0xFFFF;
	while (!(RCC->CR & RCC_CR_HSIRDY) && timeout--)
		;

	/* Select HSI clock as main clock */
	RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_SW)) | RCC_CFGR_SW_HSI;

	/* Disable PLL */
	RCC->CR &= ~RCC_CR_PLLON;

	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE
			| RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 25;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}

}

/*
 * we check if there is anything stored inside internal flash
 * if nothing in available then we store our default values
 * else we take the values stroed inside the flash
 */
static void load_param() {

	Flash_Packet *ipFlaPar;
	ipFlaPar = (Flash_Packet*) Address_Mem;

	if (ipFlaPar->valid_pattern == m_Flash_Packet.valid_pattern) // 1. Sentence, pattern valid ?
			{
		memcpy(&_Flash_Packet, ipFlaPar, sizeof(Flash_Packet));
	}

	else //load default param
	{
		memcpy(&_Flash_Packet, &m_Flash_Packet, sizeof(Flash_Packet));
	}

	if (strstr(_Flash_Packet.File_Format, ".CSV")) {
		_RunTime_Packet.fileformat_selection = 1;
	} else if (strstr(_Flash_Packet.File_Format, ".JSON")) {
		_RunTime_Packet.fileformat_selection = 0;
	}

}

/*
 * we store the whole structure inside the internal flash
 */
static uint32_t store_param() {
	__disable_irq();

	uint32_t ZielAdr;
	uint32_t WrAdr;
	uint32_t QuellAdr = (uint32_t) &_Flash_Packet;
	static FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t SECTORError;
	/* Unlock the Flash to enable the flash control register access *************/
	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP);
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPERR);
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_WRPERR);
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_PGAERR);
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_PGPERR);
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_PGSERR);
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_RDERR);

	EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.Sector = FLASH_SECTOR_7;
	EraseInitStruct.NbSectors = 1;

	/* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
	 you have to make sure that these data are rewritten before they are accessed during code
	 execution. If this cannot be done safely, it is recommended to flush the caches by setting the
	 DCRST and ICRST bits in the FLASH_CR register. */
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError) != HAL_OK) {
		return HAL_FLASH_GetError();
	}

	ZielAdr = Address_Mem + sizeof(Flash_Packet); /* - FLASH_DOUBLE_SIZE))*/
	WrAdr = Address_Mem; // Pointer on 1st entry

	while (WrAdr < ZielAdr) {

		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, WrAdr,
				*(uint64_t*) QuellAdr) == HAL_OK)

		{
			WrAdr += 4;
			QuellAdr += 4;

		} else {

			HAL_FLASH_Lock();
			return HAL_FLASH_GetError();

		}

	}
	HAL_FLASH_Lock();

	__enable_irq();

//	if(start_usb_timer)
//	{
//		HAL_TIM_Base_Start_IT(&htim2);
//	}

	return 0;

}
/*
 * This is to save data in internal flash. when user change something that needs to be saved in flash for future use
 * we wait for 30seconds, so in this time if user also changes something else we write to flash at once then
 */
static void save_data() {
	if (save_param) {
		if (start_timer) {
			HAL_TIM_Base_Stop_IT(&htim2);
			HAL_TIM_Base_Start_IT(&htim2);
			time_keep = 0;
			start_timer = 0;
		}

		if (time_keep >= 300) //save after 30secs when data occur happens
				{
			//if(!start_usb_timer) //if usb timer is on dont stop timer as usb will stop itself when its operation will end
			//{
			HAL_TIM_Base_Stop_IT(&htim2);
			//}
			store_param();
			time_keep = 0;
			save_param = 0;
			start_timer = 0;
		}
	}

}
static void init_scd4x_i2c() {

	sensirion_i2c_hal_init();
	scd4x_wake_up();
	scd4x_stop_periodic_measurement();
	scd4x_reinit();
	uint16_t serial_0;
	uint16_t serial_1;
	uint16_t serial_2;
	scd4x_get_serial_number(&serial_0, &serial_1, &serial_2);

}
static void get_scd4x_measurement() {
	int16_t error = 0;

	error = scd4x_start_periodic_measurement();
	sensirion_i2c_hal_sleep_usec(100000);
	bool data_ready_flag = false;
	error = scd4x_get_data_ready_flag(&data_ready_flag);

	if (data_ready_flag) {
		error = scd4x_read_measurement(&_RunTime_Packet.co2,
				&_RunTime_Packet.temperature, &_RunTime_Packet.humidity);
		_RunTime_Packet.temperature = _RunTime_Packet.temperature / 1000;
		_RunTime_Packet.humidity = _RunTime_Packet.humidity / 1000;

		if (debug_scd_pm) {
			char buf[50];
			if (error) {
				sprintf(buf, "Error executing scd4x_read_measurement(): %i\n",
						error);
				HAL_UART_Transmit(&huart1, (uint8_t*) buf, strlen(buf), 1000);
			} else if (_RunTime_Packet.co2 == 0) {
				sprintf(buf, "Invalid sample detected, skipping.\n");
				HAL_UART_Transmit(&huart1, (uint8_t*) buf, strlen(buf), 1000);
			} else {

				sprintf(buf, "Co2: %d, Temperature: %ld C, Humidity: %ld \r\n",
						_RunTime_Packet.co2, _RunTime_Packet.temperature,
						_RunTime_Packet.humidity);
				HAL_UART_Transmit(&huart1, (uint8_t*) buf, strlen(buf), 1000);
			}
		}
	}
	//error = scd4x_stop_periodic_measurement();
}
static void init_sps30() {
	char serial[SPS30_MAX_SERIAL_LEN];

	int16_t ret;
	sensirion_sleep_usec(1000000); /* sleep for 1s */

	sensirion_uart_open();

	//printf("SPS30 sensor probing successful\r\n");

	struct sps30_version_information version_information;
	ret = sps30_read_version(&version_information);
	if (ret) {
		//printf("error %d reading version information\r\n", ret);
	} else {
//		printf("FW: %u.%u HW: %u, SHDLC: %u.%u\r\n",
//				version_information.firmware_major,
//				version_information.firmware_minor,
//				version_information.hardware_revision,
//				version_information.shdlc_major,
//				version_information.shdlc_minor);
	}

	ret = sps30_get_serial(serial);
	if (ret) {
		//	printf("error %d reading serial\r\n", ret);
	} else {
		//printf("SPS30 Serial: %s\r\n", serial);
	}

//	ret = sps30_set_fan_auto_cleaning_interval_days(AUTO_CLEAN_DAYS);
//	if (ret)
//	{
//		//printf("error %d setting the auto-clean interval\r\n", ret);
//	}

}
static void get_sps30_measurement() {

	int16_t ret;
	char buffer[100];
	ret = sps30_start_measurement();
	if (ret < 0) {

//			sprintf(buffer,"error starting measurement\r\n");
//			HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 1000);

	}

	ret = sps30_read_measurement(&m);
	_RunTime_Packet.pm1_0 = m.mc_1p0;
	_RunTime_Packet.pm2_5 = m.mc_2p5;
	_RunTime_Packet.pm4_0 = m.mc_4p0;
	_RunTime_Packet.pm10_0 = m.mc_10p0;

	if (ret < 0) {
//				if (debug_scd_pm) {
//				printf("error reading measurement\r\n");
//				HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 1000);
//				}

	} else {
		if (debug_scd_pm) {
			if (SPS30_IS_ERR_STATE(ret)) {
				sprintf(buffer,
						"Chip state: %u - measurements may not be accurate\r\n",
						SPS30_GET_ERR_STATE(ret));
				HAL_UART_Transmit(&huart1, (uint8_t*) buffer, strlen(buffer),
						1000);
			}
			sprintf(buffer,
					"%0.2f pm1.0\r\n%0.2f pm2.5\r\n%0.2f pm4.0\r\n%0.2f pm10.0\r\n",
					m.mc_1p0, m.mc_2p5, m.mc_4p0, m.mc_10p0);

			HAL_UART_Transmit(&huart1, (uint8_t*) buffer, strlen(buffer), 1000);
		}
	}

	//	ret =	sps30_stop_measurement();

}
static void check_peripheral_error() {
	if (_RunTime_Packet.console_uart_error) {
		_RunTime_Packet.console_uart_error = 0; //we clear it and reset the peripherals
		console_uart_init();
	}

	if (_RunTime_Packet.sps30_uart_error) {
		_RunTime_Packet.sps30_uart_error = 0;
		disable_5v();
		HAL_Delay(500);
		enable_5v();
		init_sps30();
	}

	if (HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t) (0x62 << 1), 5, 100)
			!= HAL_OK) {

		_RunTime_Packet.scd4x_i2c_error = 1;
	}

}
static void sensor_calibration() {

	if (_RunTime_Packet.PM_calibration && !_RunTime_Packet.usb_detection) {
		_RunTime_Packet.PM_calibration = 0;
		sps30_start_manual_fan_cleaning();
	}

	if (_RunTime_Packet.CO2_calibration) {
		_RunTime_Packet.CO2_calibration = 0;
		scd4x_perform_forced_recalibration(
				_RunTime_Packet._target_co2_concentration,
				&_RunTime_Packet._frc_correction);
	}

}
static void sleep() {

//save stuff to sd card right here then go to sleep
	GREEN_LED_PWM(disable_led);
	RED_LED_PWM(disable_led);
	BLUE_LED_PWM(disable_led);

	HAL_ADC_DeInit(&hadc1);
	HAL_UART_Transmit(&huart1, (uint8_t*) "sleepTime\r\n", 11, 1000);
	disable_5v();
	disable_motion();
	scd4x_stop_periodic_measurement();
	scd4x_power_down();
	console_uart_deinit();
	sensirion_i2c_hal_free();
	sensirion_uart_close();
	MX_USB_DEVICE_DeInit();

	HAL_TIM_Base_DeInit(&htim1);
	HAL_TIM_Base_DeInit(&htim2);
	HAL_TIM_Base_DeInit(&htim3);
	HAL_DMA_DeInit(&hdma_tim1_up);

	clock_speed_slow();

	HAL_SuspendTick();
	/*## Enter Stop Mode #######################################################*/
	HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

}
static void wakeup() {

	//also add usb stuff

	HAL_ResumeTick();
	clock_speed_high();
	enable_5v();
	enable_motion();
	console_uart_init();

	MX_DMA_Init();
	MX_TIM1_Init();
	MX_TIM2_Init();
	MX_TIM3_Init();

	init_scd4x_i2c();
	MX_USB_DEVICE_Init(); //initialize usb anyways
	if (!HAL_GPIO_ReadPin(USB_DETECT_GPIO_Port, USB_DETECT_Pin)) {
		init_sps30();
	} else {
		_RunTime_Packet.usb_detection = 1;
		_RunTime_Packet.usb_start_timer = 1;
		usb_time_keep = 0;
		prev_usb_time_ = HAL_GetTick();
	}

	HAL_TIM_Base_Start(&htim1);
	HAL_DMA_Start(&hdma_tim1_up, (uint32_t) &(dataA[0]),
			(uint32_t) &(BLUE_LED_GPIO_Port->BSRR),
			sizeof(dataA) / sizeof(dataA[0]));
	__HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_UPDATE);
	BLUE_LED_PWM(disable_led);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); //for green led
	GREEN_LED_PWM(disable_led);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2); //for red led
	RED_LED_PWM(disable_led);

	pwr_off_detected();

	HAL_UART_Transmit(&huart1, (uint8_t*) "wakeTime\r\n", 10, 1000);
}

static void green_led_blink() {

	GREEN_LED_PWM(0);
	HAL_Delay(800);

	GREEN_LED_PWM(disable_led);
	HAL_Delay(800);

	GREEN_LED_PWM(0);
	HAL_Delay(800);

	GREEN_LED_PWM(disable_led);
	HAL_Delay(800);

	GREEN_LED_PWM(0);

}

static void led_awake_routine() {

	if (_RunTime_Packet.sd_card_disk_write_error == 0) //no errors in sdcard
			{
		if (HAL_GPIO_ReadPin(USB_DETECT_GPIO_Port, USB_DETECT_Pin)) //if usb is detected, then just turn the blue led on
				{
			GREEN_LED_PWM(disable_led);
			RED_LED_PWM(disable_led);
			BLUE_LED_PWM(0);

		} else {
			BLUE_LED_PWM(disable_led); //we turn off the blue led incase it was on before
			if (_RunTime_Packet.battery_voltage >= battery_Full) {
				GREEN_LED_PWM(0);
				RED_LED_PWM(disable_led);
				BLUE_LED_PWM(disable_led);

			} else {
				if (_RunTime_Packet.battery_voltage > battery_Low
						&& _RunTime_Packet.battery_voltage <= battery_Full) {
					RED_LED_PWM(disable_led);
					BLUE_LED_PWM(disable_led);

					if (!green_led_pwm_flag) {
						green_led_pwm_val += 5;
					} else {
						green_led_pwm_val -= 5;
					}

					if (green_led_pwm_val > 1000) {
						green_led_pwm_flag = 1;
					} else if (green_led_pwm_val <= 0) {
						green_led_pwm_flag = 0;
					}

					GREEN_LED_PWM(green_led_pwm_val); //we breathe the greem led
				} else if (_RunTime_Packet.battery_voltage > battery_very_low
						&& _RunTime_Packet.battery_voltage <= battery_Low) {

					if(green_led_pwm_flag != red_led_pwm_flag) //for this we need flags to be sync with each other
					{
						green_led_pwm_flag = red_led_pwm_flag;
					}

					BLUE_LED_PWM(disable_led);

					if (!green_led_pwm_flag) {
						green_led_pwm_val += 5;
					} else {
						green_led_pwm_val -= 5;
					}

					if (green_led_pwm_val > 1000) {
						green_led_pwm_flag = 1;
					} else if (green_led_pwm_val <= 0) {
						green_led_pwm_flag = 0;
					}

					if (!red_led_pwm_flag) {
						red_led_pwm_val += 5;
					} else {
						red_led_pwm_val -= 5;
					}

					if (red_led_pwm_val > 1000) {
						red_led_pwm_flag = 1;
					} else if (red_led_pwm_val <= 0) {
						red_led_pwm_flag = 0;
					}

					GREEN_LED_PWM(green_led_pwm_val); //we breathe the greem led
					RED_LED_PWM(red_led_pwm_val); //we breathe red led
				} else //we now know battery is very low
				{
					GREEN_LED_PWM(disable_led);
					BLUE_LED_PWM(disable_led);
					if (!red_led_pwm_flag) {
						red_led_pwm_val += 5;
					} else {
						red_led_pwm_val -= 5;
					}

					if (red_led_pwm_val > 1000) {
						red_led_pwm_flag = 1;
					} else if (red_led_pwm_val <= 0) {
						red_led_pwm_flag = 0;
					}

					RED_LED_PWM(red_led_pwm_val); //we breathe red led
				}

			}

		}
	} else {
		GREEN_LED_PWM(disable_led);
		BLUE_LED_PWM(disable_led);
		if(red_led_pwm_val != disable_led)
		{
			red_led_pwm_val = disable_led;
		}
		else
		{
			red_led_pwm_val =0;
		}
		RED_LED_PWM(red_led_pwm_val); //we toggle red led
		HAL_Delay(300);
	}
}

static uint8_t sUid[13];	//12-bit asci

//unique ID from MCU
char* ver_GetUid(void) {
	uint32_t iWafer;

	iWafer = HAL_GetUIDw1() & 0x00FFul;
	// 7 lot numbers are already in ASCII
	*(uint32_t*) (&sUid[0]) = HAL_GetUIDw1() >> 8;
	*(uint32_t*) (&sUid[3]) = HAL_GetUIDw2();

	// Convert wafer number to 1 ASCII character
	if (iWafer >= 36)
		sUid[7] = (iWafer >= 62) ? 'z' : ('a' + (iWafer - 36));
	// Display with 10 + 26 + 26 symbols
	else
		sUid[7] = ((iWafer >= 10) ? 'A' : '0') + iWafer;

	// Convert X and Y to 4 ASCII-HEX
	CharToHexasc(&sUid[8], (HAL_GetUIDw0() >> 16) & 0x00FFul);
	CharToHexasc(&sUid[10], HAL_GetUIDw0() & 0x00FFul);
	sUid[12] = 0;
	return ((char*) sUid);
}

void app_main() {

	uint8_t stop_measurement = 1;
	memset(&_RunTime_Packet, 0, sizeof(_RunTime_Packet));

	load_param();

	HAL_TIM_Base_Start(&htim1);
	HAL_DMA_Start(&hdma_tim1_up, (uint32_t) &(dataA[0]),
			(uint32_t) &(BLUE_LED_GPIO_Port->BSRR),
			sizeof(dataA) / sizeof(dataA[0]));
	__HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_UPDATE);
	BLUE_LED_PWM(disable_led);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); //for green led
	GREEN_LED_PWM(disable_led);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2); //for red led
	RED_LED_PWM(disable_led);

	MX_USB_DEVICE_Init(); //initialize usb anyways
	prev_usb_time_ = HAL_GetTick();
//
//
	if (!HAL_GPIO_ReadPin(USB_DETECT_GPIO_Port, USB_DETECT_Pin)) {
		init_sps30();
	} else {
		_RunTime_Packet.usb_detection = 0;
		_RunTime_Packet.usb_first_start = 1;
		_RunTime_Packet.usb_active_disable_pm = 1;
		_RunTime_Packet.usb_start_timer = 1;
		usb_time_keep = 0;
	}

	//init_sps30();
	init_scd4x_i2c();
	console_init();
	Rtc_set_alarm();

	RTC_DateTypeDef sDate;
	HAL_RTC_GetDate(RTC_Handle, &sDate, RTC_FORMAT_BIN);
	_RunTime_Packet.prev_day = sDate.Date;

	//if this flag is -1 this means that file creation failed at the start of program because usb was connected
	//as accessing both usb and sdcard is not possible
	//therefore we will run file creation again when usb operation is completed. it takes about 30second
	//untill that 30second we wont even go to sleep as usb is connected so power is not the issue

    //if(!HAL_GPIO_ReadPin(USB_DETECT_GPIO_Port, USB_DETECT_Pin))
	//{
	  _RunTime_Packet.sd_file_creation = createfile(_Flash_Packet.File_Name,_Flash_Packet.File_Format);
	//}
   // else
   // {
   // 	 _RunTime_Packet.sd_file_creation = -1;
   // }



    green_led_blink();

//MX_USB_DEVICE_DeInit();
	HAL_UART_Transmit(&huart1, (uint8_t*) "System Has Started \r\n", 21, 200);

	while (1) {

		prev_sleep_time = HAL_GetTick();
		prev_sleep_time_pm_co2 = HAL_GetTick();
		stop_measurement = 1;
		while (HAL_GetTick() - prev_sleep_time <= sleep_time) //stay awake for only 1min and then sleep
		{

//			if(hsd.ErrorCode != 0)
//			{
//
//
//				HAL_UART_Transmit(&huart1, (uint8_t*)buf_sdcard, strlen(buf_sdcard), 1000);
//
//			}

			if (HAL_GPIO_ReadPin(USB_DETECT_GPIO_Port, USB_DETECT_Pin)) //if usb is active dont go to sleep
					{
				prev_sleep_time = HAL_GetTick();
			}

			/**
			 * This is some kind of bug in this rtc that if time is not read continously with ddate inside the loop it does not update
			 * sometimes it does but this is the solution to solve it.

			 */

			RTC_TimeTypeDef gTime;
			HAL_RTC_GetTime(RTC_Handle, &gTime, RTC_FORMAT_BIN);
			RTC_DateTypeDef sDate;
			HAL_RTC_GetDate(RTC_Handle, &sDate, RTC_FORMAT_BIN);

			_RunTime_Packet.day = sDate.Date;
			_RunTime_Packet.month = sDate.Month;
			_RunTime_Packet.year = sDate.Year;

			if (_RunTime_Packet.prev_day != sDate.Date) {
				_RunTime_Packet.day_changed = 1;
				_RunTime_Packet.prev_day = sDate.Date;
			}

			check_peripheral_error();

//this routine is for when usb is detected
//the time calculated right now for mass storage to do its operation is about 30second but time is kept about 50sec to be on safe side
//this routine is specifically made for sps30 as for some reason sps30 uart stops working when usb is connected
			if (_RunTime_Packet.usb_detection
					|| _RunTime_Packet.usb_first_start) {

				if (_RunTime_Packet.usb_first_start) //part of old implementation not being used
				{

					start_usb_timer = 1;
				} else {
					_RunTime_Packet.usb_active_disable_pm = 1; //old implementation not being used
				}

				if ((HAL_GetTick() - prev_usb_time_ >= usb_time_)) //50sec has passed our usb must have initialized now
				{
					if (_RunTime_Packet.sd_file_creation == -1
							&& _RunTime_Packet.usb_first_start) {
						_RunTime_Packet.sd_file_creation = createfile(
								_Flash_Packet.File_Name,
								_Flash_Packet.File_Format);
					}

					_RunTime_Packet.usb_first_start = 0;
					_RunTime_Packet.usb_detection = 0;
					_RunTime_Packet.usb_active_disable_pm = 0;
					start_usb_timer = 0;

					init_sps30();
					//now as after connection of usb the 50seconds have passed now we can enable PM uart to access it

					prev_sleep_time_pm_co2 = HAL_GetTick(); //as we need to take measurements now
				}

				prev_sleep_time = HAL_GetTick();
			}

			console_process();

			//we wont take measurement until usb_detection flag disables. this flag means usb is in operation right now.
			//reason of doing this is when usb is in operation sps30 does not work so we wait for usb operation to end and then take the readings
			while ((HAL_GetTick() - prev_sleep_time_pm_co2 <= sleep_time_pm_co2)
					&& !_RunTime_Packet.usb_detection
					&& !_RunTime_Packet.usb_first_start)
			//if(!_RunTime_Packet.usb_detection && !_RunTime_Packet.usb_first_start)
			{

				console_process(); //run console here also so user can still access it
				if (!_RunTime_Packet.scd4x_i2c_error) {
					get_scd4x_measurement();
				}

				get_sps30_measurement();

				adc_Measure(&_RunTime_Packet.battery_voltage);
				led_awake_routine();
				pwr_off_detected();
				HAL_Delay(20);
			}

			/*
			 * if user performs calibration
			 */
			sensor_calibration();

			//dont stop measurement if USB is detected, we keep taking measurement after interval as power is not an issue if usb is active
			if (stop_measurement && !_RunTime_Packet.usb_detection
					&& !_RunTime_Packet.usb_first_start
					&& !HAL_GPIO_ReadPin(USB_DETECT_GPIO_Port,
					USB_DETECT_Pin)) {
				scd4x_stop_periodic_measurement();
				sps30_stop_measurement();
				stop_measurement = 0;
			}

//if USB is active and time interval is passed then we still save into the file, but make sure usb is not working at that time otherwise sdcard wont take data
//thats why usb_Detection flag is used
//this routine is specifically to table the issue, if user keeps the usb plugged in for somereason
//because the mostly the system will save data before going to sleep. if usb cable is plugged in it wont go to sleep, but with routine we still save the data after set intervals
			if (HAL_GPIO_ReadPin(USB_DETECT_GPIO_Port, USB_DETECT_Pin)
					&& set_alarm_Time && !_RunTime_Packet.usb_detection) {
				//if day changes create new file
				//if user change filename or fileformat then also create new file with that format or name
				if (_RunTime_Packet.day_changed
						|| _RunTime_Packet.filename_changed
						|| _RunTime_Packet.fileformat_changed
						|| _RunTime_Packet.sd_file_creation == -1) {
					_RunTime_Packet.day_changed = 0;
					_RunTime_Packet.filename_changed = 0;
					_RunTime_Packet.fileformat_changed = 0;
					_RunTime_Packet.sd_file_creation = createfile(
							_Flash_Packet.File_Name, _Flash_Packet.File_Format);
				}
				filesaving_process();

				prev_sleep_time_pm_co2 = HAL_GetTick(); //we also enable sensors to take reading then
			}

			Rtc_set_alarm();

			adc_Measure(&_RunTime_Packet.battery_voltage);
			led_awake_routine();
			save_data();

			if (HAL_GPIO_ReadPin(OUT_MOTION_GPIO_Port, OUT_MOTION_Pin)) {
				_RunTime_Packet.motion_detection = 1;
				debug_pir_motion = 1;
			} else {
				_RunTime_Packet.motion_detection = 0;
				debug_pir_motion = 1;

			}
			pwr_off_detected();
		}

		//if day changes create new file
		//if user change filename or fileformat then also create new file with that format or name
		if (_RunTime_Packet.day_changed || _RunTime_Packet.filename_changed
				|| _RunTime_Packet.fileformat_changed
				|| _RunTime_Packet.sd_file_creation == -1) {
			_RunTime_Packet.day_changed = 0;
			_RunTime_Packet.filename_changed = 0;
			_RunTime_Packet.fileformat_changed = 0;
			_RunTime_Packet.sd_file_creation = createfile(
					_Flash_Packet.File_Name, _Flash_Packet.File_Format);
		}
		filesaving_process();
		prev_sleep_time = HAL_GetTick();

		if (!HAL_GPIO_ReadPin(USB_DETECT_GPIO_Port, USB_DETECT_Pin)) {

			sleep();
			wakeup();
		}

	}

}

/**
 ret = sps30_stop_measurement();
 if (ret) {
 printf("Stopping measurement failed\r\n");
 }

 if (version_information.firmware_major >= 2) {
 ret = sps30_sleep();
 if (ret) {
 printf("Entering sleep failed\r\n");
 }
 }

 printf("No measurements for 1 minute\r\n");
 sensirion_sleep_usec(1000000 * 60);

 if (version_information.firmware_major >= 2) {
 ret = sps30_wake_up();
 if (ret) {
 printf("Error %i waking up sensor\r\n", ret);
 }
 }
 */

