/*
 *  mpu.c
 *  MPU 6500 sensor initialization
 *  Created on: Jun 5, 2026
 *      Author: nate
 */

#include "mpu.h"

#define MPU_ADDR 0x68

void mpu_init(void)
{
	// wake up MPU from sleep
	i2c_write(MPU_ADDR, 0x6B, 0x00);

	// configure sensor registers
	i2c_write(MPU_ADDR, 0x1B, 0x00);	// gyro
	i2c_write(MPU_ADDR, 0x1C, 0x00);	// accel
}

uint8_t mpu_read(int16_t* data)
{
	// reads all 14 bytes for gyro, temp, and accel data
	uint8_t buffer[14];
	if (i2c_read_burst(MPU_ADDR, 0x3B, buffer, 14) == 0)
	{
		for(int i = 0; i < 14; i+=2)
		{
			data[i/2] = buffer[i+1] | buffer[i] << 8;
		}

		return I2C_OK;
	}
	else
		return I2C_ERROR;
}

void mpu_convert(int16_t* raw, MPU_Data* out)
{
	// converts MPU data into physical units
	out->accel_x = raw[0] / 16384.0f;
	out->accel_y = raw[1] / 16384.0f;
	out->accel_z = raw[2] / 16384.0f;
	out->temp_c = (raw[3] / 333.87f) + 21.0f;
	out->gyro_x = raw[4] / 131.0f;
	out->gyro_y = raw[5] / 131.0f;
	out->gyro_z = raw[6] / 131.0f;
}
