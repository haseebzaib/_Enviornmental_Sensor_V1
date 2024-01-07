/*
 * sensirion_arch_config.h
 *
 *  Created on: Dec 23, 2023
 *      Author: hzaib
 */

#ifndef SENSIRION_ARCH_CONFIG_H_
#define SENSIRION_ARCH_CONFIG_H_

/**
 * If your platform does not provide the library stdint.h you have to
 * define the integral types yourself (see below).
 */
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Typedef section for types commonly defined in <stdint.h>
 * If your system does not provide stdint headers, please define them
 * accordingly.
 */
/* typedef unsigned long long int uint64_t;
 * typedef long long int int64_t;
 * typedef long int32_t;
 * typedef unsigned long uint32_t;
 * typedef short int16_t;
 * typedef unsigned short uint16_t;
 * typedef char int8_t;
 * typedef unsigned char uint8_t; */

#ifndef NULL
#define NULL ((void*)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* SENSIRION_ARCH_CONFIG_H_ */
