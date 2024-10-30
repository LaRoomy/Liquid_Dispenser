#ifndef I2C_16_H_
#define I2C_16_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stm32f4xx.h>

#define		I2C_MEM_ADDR_FORMAT_8BIT		0
#define		I2C_MEM_ADDR_FORMAT_16BIT		1

#define		I2C_OK							0
#define		I2C_ERROR_INVALID_ARGUMENT		(-22)

typedef struct
{
	uint32_t i2cNumber;
	uint32_t apbClockFrequency;
	uint32_t memoryAddressFormat;

}I2CObject;

typedef int i2c_result_t;

typedef struct
{
	uint8_t slaveAddress;
	uint16_t memoryAddress;
	uint32_t lengthOfData;
	uint8_t * data;

}I2CData;

i2c_result_t I2C_init(I2CObject * self);
i2c_result_t I2C_deinit(I2CObject * self);
i2c_result_t I2C_read(I2CObject * self, I2CData * transmissionData);
i2c_result_t I2C_write(I2CObject * self, I2CData * transmissionData);
/*
 * I2C pin definitions
 * -------------------------------
 * | I2C Number | SCL  | SDA  | Alternate function
 * ---------------------------------------------
 * | I2C1       | PB8  | PB9  | AF4
 * | I2C2       | PB10 | PB11 | AF4
 * | I2C3       | PA8  | PC9  | AF4
 *
 * */
#ifdef __cplusplus
 } // extern "C"
#endif

#endif /* I2C_16_H_ */
