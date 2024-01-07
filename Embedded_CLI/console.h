/*
 * console.h
 *
 *  Created on: Dec 23, 2023
 *      Author: hzaib
 */

#ifndef CONSOLE_H_
#define CONSOLE_H_

extern uint8_t invoke_time;
extern uint8_t invoke_date;


extern void console_process(void);
extern void console_init(void);
extern void console_uart_deinit();
extern void console_uart_init();


#endif /* CONSOLE_H_ */
