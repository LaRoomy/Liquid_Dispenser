#include "main.h"

#define		DEVICE_SELECT_CODE_USER_MEMORY			(0xA6>>1)
#define		DEVICE_SELECT_CODE_SYSTEM_MEMORY		(0xAE>>1)

#define		I2C_SSO_DYN_R 		((uint16_t)0x2004)

/*
 * Test 4 - I2C
 *
 * The I2C is tested along with the NFC07A1 module. The tests are executed
 * when the user push button is pressed and the results are printed through the uart interface.
 *
 * Test cases:
 *
 * - write and read user memory
 * - burst write and read user memory
 * - enable I2C security session
 * - write and read to system memory
 *
 * */

I2CObject i2cObj =
{
	.apbClockFrequency = SYS_FRQ,
	.i2cNumber = 1,
	.memoryAddressFormat = I2C_MEM_ADDR_FORMAT_16BIT
};

uint32_t execute_tests = 0;

void test_write_read_user_memory();
void test_burst_write_read_user_memory();
void test_open_security_session();
void test_write_read_system_memory();

int main(void)
{
	CPU_Clock_Config();

	init_board_led();
	init_board_switch();

	I2C_init(&i2cObj);

	init_test_tools(SYS_FRQ);

	while(1)
	{
		if(get_board_switch_status_with_debounce() == PRESSED)
		{
			execute_tests = 1;
			control_board_led(ON);
		}
		if(execute_tests)
		{
			test_write_read_user_memory();

			test_burst_write_read_user_memory();

			test_open_security_session();

			test_write_read_system_memory();

			control_board_led(OFF);
			execute_tests = 0;
		}
	}
}

void test_write_read_user_memory()
{
	uint8_t data_out = 0, data_in = 0;
	I2CData transmissionData_out, transmissionData_in;

	printf("Test: write and read user memory of nfc07a1:\r\n\r\n");

	data_out = 5;
	// write user memory address 0x0000
	transmissionData_out.data = &data_out;
	transmissionData_out.lengthOfData = 1;
	transmissionData_out.slaveAddress = DEVICE_SELECT_CODE_USER_MEMORY;
	transmissionData_out.memoryAddress = 0x0000;
	I2C_write(&i2cObj, &transmissionData_out);

	delay_ms(50, SYS_FRQ); // wait for the internal write cycle of the nfc07a1

	data_out = 129;
	// write user memory address 0x0010
	transmissionData_out.data = &data_out;
	transmissionData_out.lengthOfData = 1;
	transmissionData_out.slaveAddress = DEVICE_SELECT_CODE_USER_MEMORY;
	transmissionData_out.memoryAddress = 0x0010;
	I2C_write(&i2cObj, &transmissionData_out);

	delay_ms(50, SYS_FRQ); // wait for the internal write cycle of the nfc07a1

	// read 0x0000
	transmissionData_in.data = &data_in;
	transmissionData_in.lengthOfData = 1;
	transmissionData_in.memoryAddress = 0x0000;
	transmissionData_in.slaveAddress = DEVICE_SELECT_CODE_USER_MEMORY;
	I2C_read(&i2cObj, &transmissionData_in);

	assert_uint_are_equal(5, (uint32_t)data_in, "Read data at address 0x0000");

	// read 0x0010
	data_in = 0;
	transmissionData_in.data = &data_in;
	transmissionData_in.lengthOfData = 1;
	transmissionData_in.memoryAddress = 0x0010;
	transmissionData_in.slaveAddress = DEVICE_SELECT_CODE_USER_MEMORY;
	I2C_read(&i2cObj, &transmissionData_in);

	assert_uint_are_equal(129, (uint32_t)data_in, "Read data at address 0x0010");

	// tear down
	// write memory at address 0x0000 and 0x0010 back to 0
	data_out = 0;
	transmissionData_out.data = &data_out;
	transmissionData_out.lengthOfData = 1;
	transmissionData_out.slaveAddress = DEVICE_SELECT_CODE_USER_MEMORY;
	transmissionData_out.memoryAddress = 0x0000;
	I2C_write(&i2cObj, &transmissionData_out);

	delay_ms(50, SYS_FRQ); // wait for the internal write cycle of the nfc07a1

	transmissionData_out.memoryAddress = 0x0010;
	I2C_write(&i2cObj, &transmissionData_out);
	delay_ms(50, SYS_FRQ); // wait for the internal write cycle of the nfc07a1
}

void test_burst_write_read_user_memory()
{
	uint8_t expected_data[5] = {28,29,48,49,88};
	uint8_t actual_data[5] = {0};
	char msg[] = "Test field at position: 0\0";
	I2CData transmissionData_out, transmissionData_in;

	printf("Test: burst write and burst read user memory of nfc07a1:\r\n\r\n");

	// write memory from address 0x0040
	transmissionData_out.data = expected_data;
	transmissionData_out.lengthOfData = 5;
	transmissionData_out.memoryAddress = 0x0040;
	transmissionData_out.slaveAddress = DEVICE_SELECT_CODE_USER_MEMORY;
	I2C_write(&i2cObj, &transmissionData_out);

	delay_ms(50, SYS_FRQ); // wait for the internal write cycle of the nfc07a1

	// read memory from address 0x0040
	transmissionData_in.data = actual_data;
	transmissionData_in.lengthOfData = 5;
	transmissionData_in.memoryAddress = 0x0040;
	transmissionData_in.slaveAddress = DEVICE_SELECT_CODE_USER_MEMORY;
	I2C_read(&i2cObj, &transmissionData_in);

	// test
	for(int i = 0; i < 5; i++)
	{
		msg[24] = ((char)(48+i));
		assert_uint_are_equal(expected_data[i], actual_data[i], msg);
	}

	// tear down
	uint8_t tear_down_data[5] = {0};

	transmissionData_out.data = tear_down_data;
	transmissionData_out.lengthOfData = 5;
	transmissionData_out.memoryAddress = 0x0040;
	transmissionData_out.slaveAddress = DEVICE_SELECT_CODE_USER_MEMORY;
	I2C_write(&i2cObj, &transmissionData_out);

	delay_ms(50, SYS_FRQ); // wait for the internal write cycle of the nfc07a1
}

void test_open_security_session()
{
	uint8_t data = 0;
	uint8_t st25_i2c_passwd_seq[17] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	I2CData transmissionData_out, transmissionData_in;

	printf("Test: open I2C security session with the nfc07a1:\r\n\r\n");

	// open security session
	transmissionData_out.data = st25_i2c_passwd_seq;
	transmissionData_out.lengthOfData = 17;
	transmissionData_out.memoryAddress = 0x0900;
	transmissionData_out.slaveAddress = DEVICE_SELECT_CODE_SYSTEM_MEMORY;
	I2C_write(&i2cObj, &transmissionData_out);

	// check if security session is open
	transmissionData_in.data = &data;
	transmissionData_in.lengthOfData = 1;
	transmissionData_in.memoryAddress = I2C_SSO_DYN_R;
	transmissionData_in.slaveAddress = DEVICE_SELECT_CODE_USER_MEMORY;
	I2C_read(&i2cObj, &transmissionData_in);

	assert_true((data & (1U<<0)), "Check bit 0 of I2C_SSO_DYN register");
}

void test_write_read_system_memory()
{
	uint8_t expected_data = ((1U<<0)|(1U<<3));
	uint8_t data_read = 0;
	I2CData transmissionData_out, transmissionData_in;

	printf("Test: write to the GPO1 system register of the nfc07a1:\r\n\r\n");

	transmissionData_out.data = &expected_data;
	transmissionData_out.lengthOfData = 1;
	transmissionData_out.memoryAddress = 0x0000;
	transmissionData_out.slaveAddress = DEVICE_SELECT_CODE_SYSTEM_MEMORY;
	I2C_write(&i2cObj, &transmissionData_out);

	delay_ms(50, SYS_FRQ); // wait for the internal write cycle of the nfc07a1

	transmissionData_in.data = &data_read;
	transmissionData_in.lengthOfData = 1;
	transmissionData_in.memoryAddress = 0x0000;
	transmissionData_in.slaveAddress = DEVICE_SELECT_CODE_SYSTEM_MEMORY;
	I2C_read(&i2cObj, &transmissionData_in);

	assert_uint_are_equal(expected_data, data_read, "Check if flag 0 and 3 are set in GPO1 register");

	// tear down
	expected_data = 0;
	I2C_write(&i2cObj, &transmissionData_out);

	delay_ms(50, SYS_FRQ); // wait for the internal write cycle of the nfc07a1
}




