/*
 *  mpu.h
 *  Header for mpu.c file
 *  Created on: Jun 5, 2026
 *      Author: nate
 */

#ifndef MPU_H
#define MPU_H

#include "stm32f4xx.h"
#include "i2c.h"

typedef struct {
	float accel_x, accel_y, accel_z;
	float gyro_x, gyro_y, gyro_z;
	float temp_c;
} MPU_Data;

void mpu_init(void);
uint8_t mpu_read(int16_t* data);
void mpu_convert(int16_t* raw, MPU_Data* out);

#endif
