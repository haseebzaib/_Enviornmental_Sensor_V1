/*
 * csv_json_handling.h
 *
 *  Created on: Dec 24, 2023
 *      Author: hzaib
 */

#ifndef CSV_JSON_HANDLING_H_
#define CSV_JSON_HANDLING_H_

#include "main.h"


extern uint8_t file_already_exist;

extern void json_update();
extern void csv_update();
extern void csv_header();
extern void filesaving_process();
extern int8_t createfile(char  *filename,char *fileformat);



#endif /* CSV_JSON_HANDLING_H_ */
