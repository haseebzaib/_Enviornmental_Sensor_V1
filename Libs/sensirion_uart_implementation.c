/*
 * Copyright (c) 2018, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "sensirion_arch_config.h"
#include "sensirion_uart.h"
#include "main.h"
#include "app_main.h"
#include "stdio.h"
#include "string.h"
uint16_t rx_len=0;
uint16_t rx_incre = 0;
uint8_t rx_Wait = 0;
/*
 * INSTRUCTIONS
 * ============
 *
 * Implement all functions where they are marked with TODO: implement
 * Follow the function specification in the comments.
 */

/**
 * sensirion_uart_select_port() - select the UART port index to use
 *                                THE IMPLEMENTATION IS OPTIONAL ON SINGLE-PORT
 *                                SETUPS (only one SPS30)
 *
 * Return:      0 on success, an error code otherwise
 */
int16_t sensirion_uart_select_port(uint8_t port) {
    return 0;
}

uint8_t rx_Data;
uint16_t len = 0;

void sensirion_uart_interrupt()
{
	if(rx_incre == 0 && rx_Data == 0xFE)
	{
		RX_Data[rx_incre] = rx_Data;
	}
	else
	{


		RX_Data[rx_incre] = rx_Data;
		if(rx_Data == 0xFE)
		{
			rx_len = rx_incre;
			rx_Wait = 1;
		}

	}

	rx_incre++;
	HAL_UART_Receive_IT(&huart2, &rx_Data, 1);
}
/**
 * sensirion_uart_open() - initialize UART
 *
 * Return:      0 on success, an error code otherwise
 */
int16_t sensirion_uart_open() {
	HAL_UART_DeInit(&huart2);
	  huart2.Instance = USART2;
	  huart2.Init.BaudRate = 115200;
	  huart2.Init.WordLength = UART_WORDLENGTH_8B;
	  huart2.Init.StopBits = UART_STOPBITS_1;
	  huart2.Init.Parity = UART_PARITY_NONE;
	  huart2.Init.Mode = UART_MODE_TX_RX;
	  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	  if (HAL_UART_Init(&huart2) != HAL_OK)
	  {
	    Error_Handler();
	  }

		HAL_UARTEx_ReceiveToIdle_IT(&huart2, RX_Data, 522);
		//HAL_UART_Receive_IT(&huart2, &rx_Data, 1);
		 rx_len=0;
		rx_incre = 0;
		//HAL_UARTEx_ReceiveToIdle_DMA(&huart2, RX_Data, 522);
    return 0;
}

/**
 * sensirion_uart_close() - release UART resources
 *
 * Return:      0 on success, an error code otherwise
 */
int16_t sensirion_uart_close() {
    // TODO: implement
	HAL_UART_DeInit(&huart2);
	HAL_UART_MspDeInit(&huart2);
    return 0;
}

/**
 * sensirion_uart_tx() - transmit data over UART
 *
 * @data_len:   number of bytes to send
 * @data:       data to send
 * Return:      Number of bytes sent or a negative error code
 */
int16_t sensirion_uart_tx(uint16_t data_len, const uint8_t* data) {
    // TODO: implement
	if(HAL_UART_Transmit(&huart2, data, data_len, HAL_MAX_DELAY) == HAL_OK)
		{
			return data_len;
		}
		else
		{
			return -1;

		}

	return data_len;
}
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{

	rx_len = Size;
	rx_Wait = 1;
	HAL_UARTEx_ReceiveToIdle_IT(&huart2,RX_Data, 522);
}

/**
 * sensirion_uart_rx() - receive data over UART
 *
 * @data_len:   max number of bytes to receive
 * @data:       Memory where received data is stored
 * Return:      Number of bytes received or a negative error code
 */
int16_t sensirion_uart_rx(uint16_t max_data_len, uint8_t* data) {
    // TODO: implement
uint32_t counter = 0;

//we give 200ms of delay
		  while(!rx_Wait && counter <= 2)
		  {
			  counter++;
			  HAL_Delay(10);

		  }
		    rx_Wait = 0;

//		    if(counter > 3)
//		    {
//		    	sensirion_uart_open();
//		    }
		    len = 0;
		    rx_incre = 0;
	    return rx_len;
	//return i;
}

/**
 * Sleep for a given number of microseconds. The function should delay the
 * execution for at least the given time, but may also sleep longer.
 *
 * Despite the unit, a <10 millisecond precision is sufficient.
 *
 * @param useconds the sleep time in microseconds
 */
void sensirion_sleep_usec(uint32_t useconds) {
    // TODO: implement
	useconds = (useconds/1000);
	HAL_Delay(useconds);
}
