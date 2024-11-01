/*
 * adxl345.h
 *
 *  Created on: Apr 18, 2024
 *      Author: Hans Zimmermann
 */

#ifndef ADXL345_H_
#define ADXL345_H_

#include "spi.h"
#include <stdint.h>

#define		DEVID_R			(0x00)
#define		DEVICE_ADDR		(0x53)
#define		POWER_CTRL_R	(0x2D)
#define		DATA_FORMAT_R	(0x31)
#define		DATA_START_ADDR	(0x32)

#define		FOUR_G			(0x01)
#define		RESET			(0x00)
#define		SET_MEASURE_B	(0x08)

void adxl_init();
void adxl_read(uint8_t address, uint8_t* rxData, uint8_t size);

#endif /* ADXL345_H_ */
