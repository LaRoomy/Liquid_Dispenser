#include <stm32f4xx.h>
#include <stm32f4xx_tools.h>
#include <ext_int.h>
#include <common.h>
#include <delay.h>
#include "nfc07a1.h"
#include <i2c.h>

#define		DEVICE_SELECT_CODE_USER_MEMORY			(0xA6>>1)
#define		DEVICE_SELECT_CODE_SYSTEM_MEMORY		(0xAE>>1)

// system memory addresses
#define		GPO1_R				((uint16_t)0x0000)
#define		GPO2_R				((uint16_t)0x0001)
#define		FTM_R				((uint16_t)0x000D)

#define		GPO_CTRL_DYN_R		((uint16_t)0x2000)
#define		IT_STS_DYN_R		((uint16_t)0x2005)
#define		I2C_SSO_DYN_R 		((uint16_t)0x2004)
#define		MB_CTRL_DYN_R		((uint16_t)0x2006)
#define		MB_LEN_DYN_R		((uint16_t)0x2007)
#define		FTM_BOX_START_ADDR	((uint16_t)0x2008)

/* fast transfer mode mailbox
   start address: 0x2008 (byte   0)
   end address:   0x2107 (byte 255) */

// GPO1_R bits
#define		GPO_EN			(1U<<0)
#define		RF_PUT_MSG_EN	(1U<<5)
#define		RF_GET_MSG_EN	(1U<<6)
#define		FIELD_CHANGE_EN (1U<<4)
#define		RF_WRITE_EN		(1U<<7)

// FTM_R bits
#define		MB_MODE			(1U<<0)

// MB_CTRL_DYN_R bits
#define		MB_EN			(1U<<0)

// IT_STS_DYN_R bits
#define		RF_PUT_MSG		(1U<<5)

static BOOL nfc_module_initialized = FALSE;
static void (*nfc_data_rec_cb)(void) = NULL;

static nfc07a1_result_t nfc07a1_open_security_session(NFC07A1Object * self);
static nfc07a1_result_t nfc07a1_prepare_ftm(NFC07A1Object * self);
static nfc07a1_result_t nfc07a1_config_gpo1_reg(NFC07A1Object * self);
static nfc07a1_result_t nfc07a1_config_gpo2_reg(NFC07A1Object * self);
static nfc07a1_result_t nfc07a1_config_ftm_reg(NFC07A1Object * self);
static nfc07a1_result_t nfc07a1_initiate_ftm(NFC07A1Object * self);
static nfc07a1_result_t nfc07a1_enable_gpo(NFC07A1Object * self);
static nfc07a1_result_t nfc07a1_enable_ftm(NFC07A1Object * self);
static nfc07a1_result_t nfc07a1_write_single_reg_value(NFC07A1Object * self, uint8_t slaveAddr, uint16_t memAddr, uint8_t regValue);
static nfc07a1_result_t i2cObjectFromNFC07A1Object(NFC07A1Object * self, I2CObject * i2cObj);
static void gpo_interrupt_callback();

nfc07a1_result_t nfc07a1_verify_mailbox(NFC07A1Object * self, uint8_t length);
uint32_t nfc07a1_read_ftm_data_length();

static nfc07a1_result_t validate_nfc07a1_object(NFC07A1Object * self)
{
	if(self == NULL)
	{
		return NFC07A1_ERR_INVALID_ARG;
	}
	if(self->gpoInterruptPort != NULL)
	{
		if(is_valid_port_pin(self->gpoInterruptPort, self->gpoInterruptPin) == FALSE)
		{
			return NFC07A1_ERR_INVALID_ARG;
		}
	}
	if(self->led1Port != NULL)
	{
		if(is_valid_port_pin(self->led1Port, self->led1Pin) == FALSE)
		{
			return NFC07A1_ERR_INVALID_ARG;
		}
	}
	if(self->led2Port != NULL)
	{
		if(is_valid_port_pin(self->led2Port, self->led2Pin) == FALSE)
		{
			return NFC07A1_ERR_INVALID_ARG;
		}
	}
	if(self->led3Port != NULL)
	{
		if(is_valid_port_pin(self->led3Port, self->led3Pin) == FALSE)
		{
			return NFC07A1_ERR_INVALID_ARG;
		}
	}
	return NFC07A1_OK;
}

nfc07a1_result_t nfc07a1_init(NFC07A1Object * self)
{
	if(nfc_module_initialized == TRUE)
	{
		return NFC07A1_ERR_ALREADY_INIT;
	}
	if(validate_nfc07a1_object(self) != NFC07A1_OK)
	{
		return NFC07A1_ERR_INVALID_ARG;
	}
	if(self->led1Port != NULL)
	{
		gpio_config_port_pin(self->led1Port, self->led1Pin, GPIO_MODE_OUTPUT);
	}
	if(self->led2Port != NULL)
	{
		gpio_config_port_pin(self->led2Port, self->led2Pin, GPIO_MODE_OUTPUT);
	}
	if(self->led3Port != NULL)
	{
		gpio_config_port_pin(self->led3Port, self->led3Pin, GPIO_MODE_OUTPUT);
	}
	if(self->gpoInterruptPort != NULL)
	{
		ExternalInterruptObject extIntObj =
		{
			.extIntPin = self->gpoInterruptPin,
			.extIntPort = self->gpoInterruptPort,
			.triggerType = EXT_INT_TRIGGER_TYPE_FALLING_EDGE,
			.ext_int_callback = gpo_interrupt_callback
		};
		if(external_interrupt_init(&extIntObj) != EXT_INT_OK)
		{
			return NFC07A1_ERR_INVALID_ARG;
		}
	}
	I2CObject i2cObj =
	{
		.apbClockFrequency = self->apbClockFrequency,
		.i2cNumber = self->i2cNumber,
		.memoryAddressFormat = I2C_MEM_ADDR_FORMAT_16BIT
	};
	if(I2C_init(&i2cObj) != I2C_OK)
	{
		return NFC07A1_ERR_INVALID_ARG;
	}

	nfc_data_rec_cb = self->nfc_data_received_irq;

	uint32_t result = nfc07a1_prepare_ftm(self);

	if(result == NFC07A1_OK)
	{
		result = nfc07a1_initiate_ftm(self);
	}
	nfc_module_initialized = TRUE;
	return result;
}

nfc07a1_result_t nfc07a1_deinit(NFC07A1Object * self)
{
	if(validate_nfc07a1_object(self) != NFC07A1_OK)
	{
		return NFC07A1_ERR_INVALID_ARG;
	}

	if(self->led1Port != NULL)
	{
		gpio_config_port_pin(self->led1Port, self->led1Pin, GPIO_MODE_RESET_STATE);
	}
	if(self->led2Port != NULL)
	{
		gpio_config_port_pin(self->led2Port, self->led2Pin, GPIO_MODE_RESET_STATE);
	}
	if(self->led3Port != NULL)
	{
		gpio_config_port_pin(self->led3Port, self->led3Pin, GPIO_MODE_RESET_STATE);
	}
	if(self->gpoInterruptPort != NULL)
	{
		ExternalInterruptObject extIntObj =
		{
			.extIntPin = self->gpoInterruptPin,
			.extIntPort = self->gpoInterruptPort,
			.triggerType = EXT_INT_TRIGGER_TYPE_FALLING_EDGE,
			.ext_int_callback = gpo_interrupt_callback
		};
		external_interrupt_deinit(&extIntObj);
	}
	I2CObject i2cObj =
	{
		.apbClockFrequency = self->apbClockFrequency,
		.i2cNumber = self->i2cNumber,
		.memoryAddressFormat = I2C_MEM_ADDR_FORMAT_16BIT
	};
	I2C_deinit(&i2cObj);

	nfc_data_rec_cb = NULL;
	nfc_module_initialized = FALSE;
	return NFC07A1_OK;
}

void nfc07a1_switch_user_led_1(NFC07A1Object * self, uint32_t state)
{
	if(self != NULL)
	{
		gpio_control_pin(self->led1Port, self->led1Pin, state);
	}
}

void nfc07a1_switch_user_led_2(NFC07A1Object * self, uint32_t state)
{
	if(self != NULL)
	{
		gpio_control_pin(self->led2Port, self->led2Pin, state);
	}
}

void nfc07a1_switch_user_led_3(NFC07A1Object * self, uint32_t state)
{
	if(self != NULL)
	{
		gpio_control_pin(self->led3Port, self->led3Pin, state);
	}
}

static nfc07a1_result_t nfc07a1_open_security_session(NFC07A1Object * self)
{
	uint8_t st25_i2c_passwd_seq[17] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	uint8_t data = 0;

	I2CObject i2cObj;
	i2cObjectFromNFC07A1Object(self, &i2cObj);

	// open security session
	I2CData i2cData =
	{
		.data = st25_i2c_passwd_seq,
		.lengthOfData = 17,
		.memoryAddress = 0x0900,
		.slaveAddress = DEVICE_SELECT_CODE_SYSTEM_MEMORY
	};
	I2C_write(&i2cObj, &i2cData);

	// check if security session is open
	i2cData.data = &data;
	i2cData.lengthOfData = 1;
	i2cData.memoryAddress = I2C_SSO_DYN_R;
	i2cData.slaveAddress = DEVICE_SELECT_CODE_USER_MEMORY;
	I2C_read(&i2cObj, &i2cData);

	if(data & (1U<<0))
	{
		return NFC07A1_OK;
	}
	return NFC07A1_ERR;
}

nfc07a1_result_t nfc07a1_read_mailbox(NFC07A1Object * self)
{
	if(self == NULL)
	{
		return NFC07A1_ERR_INVALID_ARG;
	}
	uint8_t it_sys_dyn_r = 0;
	uint8_t data_len_r = 0;

	uint8_t buffer[256] = {0};

	//clear_mailbox_buffer
	for(uint32_t i = 0; i < 256; i++)
	{
		self->mailbox_buffer[i] = '\0';
	}

	I2CObject i2cObj;
	i2cObjectFromNFC07A1Object(self, &i2cObj);

	/* read interrupt event type */
	I2CData i2cData =
	{
		.data = &it_sys_dyn_r,
		.lengthOfData = 1,
		.memoryAddress = IT_STS_DYN_R,
		.slaveAddress = DEVICE_SELECT_CODE_USER_MEMORY
	};
	I2C_read(&i2cObj, &i2cData);

	if(it_sys_dyn_r & (RF_PUT_MSG))
	{
		/* read length of fast transfer message */
		i2cData.data = &data_len_r;
		i2cData.lengthOfData = 1;
		i2cData.memoryAddress = MB_LEN_DYN_R;
		i2cData.slaveAddress = DEVICE_SELECT_CODE_USER_MEMORY;
		I2C_read(&i2cObj, &i2cData);

		// read one byte more - this is important - otherwise the RF_PUT_MESSAGE flag is not cleared
		if(data_len_r < 255)
		{
			data_len_r += 1;
		}

		/* read message */
		i2cData.data = buffer;
		i2cData.lengthOfData = data_len_r;
		i2cData.memoryAddress = FTM_BOX_START_ADDR;
		i2cData.slaveAddress = DEVICE_SELECT_CODE_USER_MEMORY;
		I2C_read(&i2cObj, &i2cData);

		// copy buffer
		for(int i = 0; i < 256; i++)
		{
			if(buffer[i] == 0)
			{
				break;
			}
			self->mailbox_buffer[i] = (char)buffer[i];
		}
	}
	return NFC07A1_OK;
}

nfc07a1_result_t nfc07a1_write_mailbox(NFC07A1Object * self, const char* data, uint32_t length)
{
	if(length < 254)
	{
		uint8_t data_buffer[256] = {0};

		I2CObject i2cObj;
		i2cObjectFromNFC07A1Object(self, &i2cObj);

		I2CData i2cData =
		{
			.data = data_buffer,
			.lengthOfData = length,
			.memoryAddress = (FTM_BOX_START_ADDR),
			.slaveAddress = DEVICE_SELECT_CODE_USER_MEMORY
		};

		// convert buffer
		for(int i = 0; i < length; i++)
		{
			data_buffer[i] = (uint8_t)data[i];
		}
		I2C_write(&i2cObj, &i2cData);

		return NFC07A1_OK;
	}
	return NFC07A1_MBX_SIZE_EXCEEDED;
}

static nfc07a1_result_t nfc07a1_prepare_ftm(NFC07A1Object * self)
{
	uint32_t result;
	/*
		I2C security session must first be open, by presenting a valid I²C password, to grant write access to system
		configuration registers (this must only be done once since the system registers are non volatile)
	*/
	result = nfc07a1_open_security_session(self);
	if(result != NFC07A1_OK)
	{
		return NFC07A1_SECSESSION_FAIL;
	}

	result = nfc07a1_config_gpo1_reg(self);
	if(result != NFC07A1_OK)
	{
		return NFC07A1_GPO1_PG_FAIL;
	}

	result = nfc07a1_config_gpo2_reg(self);
	if(result != NFC07A1_OK)
	{
		return NFC07A1_GPO2_PG_FAIL;
	}

	result = nfc07a1_config_ftm_reg(self);
	if(result != NFC07A1_OK)
	{
		return NFC07A1_FTM_R_PG_FAIL;
	}

	return NFC07A1_OK;
}

static nfc07a1_result_t nfc07a1_config_gpo1_reg(NFC07A1Object * self)
{
	return nfc07a1_write_single_reg_value(self, DEVICE_SELECT_CODE_SYSTEM_MEMORY, GPO1_R, ((GPO_EN)|(RF_PUT_MSG_EN)));
}

static nfc07a1_result_t nfc07a1_config_gpo2_reg(NFC07A1Object * self)
{
	uint8_t regValue = 0x0C;// GPO interrupt duration 188µs
	return nfc07a1_write_single_reg_value(self, DEVICE_SELECT_CODE_SYSTEM_MEMORY, GPO2_R, regValue);
}

static nfc07a1_result_t nfc07a1_config_ftm_reg(NFC07A1Object * self)
{
	uint8_t regValue = ((MB_MODE)|((1U<<1)|(1U<<2)|(1U<<3)));// MB_MODE + 2sec watchdog duration
	return nfc07a1_write_single_reg_value(self, DEVICE_SELECT_CODE_SYSTEM_MEMORY, FTM_R, regValue);
}

static nfc07a1_result_t nfc07a1_initiate_ftm(NFC07A1Object * self)
{
	uint32_t result;

	result = nfc07a1_enable_gpo(self);
	if(result != NFC07A1_OK)
	{
		return NFC07A1_ENABLE_GPO_FAIL;
	}

	result = nfc07a1_enable_ftm(self);
	if(result != NFC07A1_OK)
	{
		return NFC07A1_ENABLE_FTM_FAIL;
	}

	/*
	 * NOTE: the verification of the data length register and the mailbox will fail if this
	 *       is a reset condition and the mailbox was written previously. Only in a power up
	 *       condition this operation makes sense. (Resetting the STM32F446 will not automatically reset the ST25DVxxKC)
	 * */

//	result = nfc07a1_read_ftm_data_length();
//	if(result != 0)
//	{
//		return NFC07A1_INVALID_DATA_LEN;
//	}

//	result = nfc07a1_verify_mailbox(8);
//	if(result != NFC07A1_OK)
//	{
//		return NFC07A1_INVALID_MB_VAL;
//	}

	return NFC07A1_OK;
}

static nfc07a1_result_t nfc07a1_enable_gpo(NFC07A1Object * self)
{
	return nfc07a1_write_single_reg_value(self, DEVICE_SELECT_CODE_USER_MEMORY, GPO_CTRL_DYN_R, (GPO_EN));
}

static nfc07a1_result_t nfc07a1_enable_ftm(NFC07A1Object * self)
{
	return nfc07a1_write_single_reg_value(self, DEVICE_SELECT_CODE_USER_MEMORY, MB_CTRL_DYN_R, (MB_EN));
}

static nfc07a1_result_t nfc07a1_write_single_reg_value(NFC07A1Object * self, uint8_t slaveAddr, uint16_t memAddr, uint8_t regValue)
{
	uint8_t reg = 0;

	I2CObject i2cObj;
	i2cObjectFromNFC07A1Object(self,  &i2cObj);

	I2CData i2cData =
	{
		.data = &reg,
		.lengthOfData = 1,
		.memoryAddress = memAddr,
		.slaveAddress = slaveAddr
	};
	I2C_read(&i2cObj, &i2cData);

	if(reg != regValue)
	{
		i2cData.data = &regValue;
		I2C_write(&i2cObj, &i2cData);

		/* wait for the internal write cycle of the std25dv */
		delay_ms(20, self->apbClockFrequency);

		i2cData.data = &reg;
		I2C_read(&i2cObj, &i2cData);

		if((reg & regValue) == 0)
		{
			return NFC07A1_ERR;
		}
	}
	return NFC07A1_OK;
}

uint32_t nfc07a1_read_ftm_data_length(NFC07A1Object * self)
{
	uint8_t data_len_r = 0;

	I2CObject i2cObj;
	i2cObjectFromNFC07A1Object(self,  &i2cObj);

	I2CData i2cData =
	{
		.data = &data_len_r,
		.lengthOfData = 1,
		.memoryAddress = MB_LEN_DYN_R,
		.slaveAddress = DEVICE_SELECT_CODE_USER_MEMORY
	};
	I2C_read(&i2cObj, &i2cData);

	return (data_len_r > 0) ? ((uint32_t)(data_len_r - 1)) : ((uint32_t)data_len_r);
}

nfc07a1_result_t nfc07a1_verify_mailbox(NFC07A1Object * self, uint8_t length)
{
	uint8_t mb_data = 0;

	I2CObject i2cObj;
	i2cObjectFromNFC07A1Object(self,  &i2cObj);

	I2CData i2cData;
	i2cData.data = &mb_data;
	i2cData.lengthOfData = 1;
	i2cData.slaveAddress = DEVICE_SELECT_CODE_USER_MEMORY;

	for(uint8_t i = 0; i < length; i++)
	{
		i2cData.memoryAddress = FTM_BOX_START_ADDR + i;

		I2C_read(&i2cObj, &i2cData);

		if(mb_data != 0xFF)
		{
			return NFC07A1_ERR;
		}
	}
	return NFC07A1_OK;
}

static nfc07a1_result_t i2cObjectFromNFC07A1Object(NFC07A1Object * self, I2CObject * i2cObj)
{
	if(self == NULL || i2cObj == NULL)
	{
		return NFC07A1_ERR_INVALID_ARG;
	}
	i2cObj->apbClockFrequency = 0; // only used in init function
	i2cObj->i2cNumber = self->i2cNumber;
	i2cObj->memoryAddressFormat = I2C_MEM_ADDR_FORMAT_16BIT;
	return NFC07A1_OK;
}

static void gpo_interrupt_callback()
{
	nfc_data_rec_cb();
}



