#include "main.h"

#define		TEST_CASE_MAX_COUNT		4U

/*
 * Test 11 - ADC
 * When the board switch is pressed the test cases are executed on after each other. Various
 * pins are tested in conjunction with different adc modules.
 * ----------------------------
 * | TestNum. | PIN | ADC Num |
 * ----------------------------
 * |   0      | PC4 |    3    |
 * |   1      | PB2 |    1    |
 * |   2      | PA0 |    1    |
 * |   3      | PC5 |    2    |
 * |   4      | PA1 |    3    |
 * ----------------------------
 * Take a look at the output monitor to see the assertions and the results.
 *
 * */

ADCObject adcObj =
{
	.adcNumber = 1,
	.adcPin = 0,
	.adcPort = GPIOA
};

uint32_t get_next_test_case_number(uint32_t num);
void perform_adc_read_test(ADCObject * target, const char * description);

void execute_test_case_0();
void execute_test_case_1();
void execute_test_case_2();
void execute_test_case_3();
void execute_test_case_4();

int main(void)
{
	CPU_Clock_Config();

	init_board_led();
	init_board_switch();
	init_test_tools(SYS_FRQ);

	uint32_t testCaseCounter = TEST_CASE_MAX_COUNT + 1U;

	while(1)
	{
		if(get_board_switch_status_with_debounce() == PRESSED)
		{
			testCaseCounter = get_next_test_case_number(testCaseCounter);

			if(testCaseCounter == 0)
			{
				execute_test_case_0();
			}
			else if(testCaseCounter == 1)
			{
				execute_test_case_1();
			}
			else if(testCaseCounter == 2)
			{
				execute_test_case_2();
			}
			else if(testCaseCounter == 3)
			{
				execute_test_case_3();
			}
			else if(testCaseCounter == 4)
			{
				execute_test_case_4();
			}
			printf("\r\nPress button to execute next test..\r\n\r\n");
		}
	}
}

void execute_test_case_0()
{
	printf("TEST 0: Initializing the adc with a wrong pin for ADC3\r\n");
	adcObj.adcNumber = 3;
	adcObj.adcPin = 4;
	adcObj.adcPort = GPIOC;
	adc_result_t res = adc_init(&adcObj);
	assert_int_are_equal(ADC_ERROR_INVALID_PIN_DEFINITION, (int)res, "ADC3 + PC4 = invalid");
}

void execute_test_case_1()
{
	printf("TEST 1: Initializing the adc with a wrong pin for ADC1\r\n");
	adcObj.adcNumber = 1;
	adcObj.adcPin = 2;
	adcObj.adcPort = GPIOB;
	adc_result_t res = adc_init(&adcObj);
	assert_int_are_equal(ADC_ERROR_INVALID_PIN_DEFINITION, (int)res, "ADC1 + PB2 = invalid");
}

void execute_test_case_2()
{
	adcObj.adcNumber = 1;
	adcObj.adcPin = 0;
	adcObj.adcPort = GPIOA;

	perform_adc_read_test(&adcObj, "TEST 2: Initializing the adc on pin PA0 on ADC1 (valid state)\r\n");
}

void execute_test_case_3()
{
	adcObj.adcNumber = 2;
	adcObj.adcPin = 5;
	adcObj.adcPort = GPIOC;

	perform_adc_read_test(&adcObj, "TEST 3: Initializing the adc on pin PC5 on ADC2 (valid state)\r\n");
}

void execute_test_case_4()
{
	adcObj.adcNumber = 3;
	adcObj.adcPin = 1;
	adcObj.adcPort = GPIOA;

	perform_adc_read_test(&adcObj, "TEST 4: Initializing the adc on pin PA1 on ADC3 (valid state)\r\n");
}

uint32_t get_next_test_case_number(uint32_t num)
{
	if(num < TEST_CASE_MAX_COUNT)
	{
		return (num + 1U);
	}
	else
	{
		return 0;
	}
}

void perform_adc_read_test(ADCObject * target, const char * description)
{
	adc_result_t res;

	printf(description);
	res = adc_init(target);
	assert_int_are_equal(ADC_OK, (int)res, "adc_init: Checking result for ADC_OK");

	printf("\r\nStarting adc conversions in 500ms interval - press button to stop\r\n\r\n");

	int counter = 0;
	uint32_t curValue;

	while(1)
	{
		if(get_board_switch_status_with_debounce() == PRESSED)
		{
			break;
		}
		if(counter == 10)
		{
			res = adc_read_single(&adcObj, &curValue);
			if(res == ADC_OK)
			{
				printf("ADC value: %u\r\n", (unsigned int)curValue);
			}
			else
			{
				printf("ADC read failed with code: %i\r\n", res);
			}


			counter = 0;
		}
		delay_ms(50, SYS_FRQ);
		counter++;
	}
	res = adc_deinit(&adcObj);
	printf("\r\n");
	assert_int_are_equal(ADC_OK, (int)res, "adc_deinit: Checking result for ADC_OK");
}








