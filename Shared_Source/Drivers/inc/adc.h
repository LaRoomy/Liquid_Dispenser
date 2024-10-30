#ifndef ADC_H_
#define ADC_H_

#ifdef __cplusplus
 extern "C" {
#endif

#define		ADC_MODULE_1		1U
#define		ADC_MODULE_2		2U
#define		ADC_MODULE_3		3U

#define		ADC_OK								0
#define		ADC_ERROR_INVALID_ARGUMENT			(-22)
#define		ADC_ERROR_INVALID_PIN_DEFINITION	(-33)

typedef struct
{
	GPIO_TypeDef * adcPort;
	uint32_t adcPin;
	uint32_t adcNumber;

}ADCObject;

typedef int adc_result_t;

adc_result_t adc_init(ADCObject * self);
adc_result_t adc_deinit(ADCObject * self);
adc_result_t adc_read_single(ADCObject * self, uint32_t * adcValue);
adc_result_t adc_read_average(ADCObject * self, uint32_t conversionCount, uint32_t * adcAverageValue);
/*
 * ADC Channels:
 *
 * PC0 - ADC123_IN10
 * PC1 - ADC123_IN11
 * PC2 - ADC123_IN12
 * PC3 - ADC123_IN13
 * PC4 - ADC12_IN14
 * PC5 - ADC12_IN15
 *
 * PB0 - ADC12_IN8
 * PB1 - ADC12_IN9
 *
 * PA0 - ADC123_IN0
 * PA1 - ADC123_IN1
 * PA2 - ADC123_IN2
 * PA3 - ADC123_IN3
 * PA4 - ADC12_IN4
 * PA5 - ADC12_IN5
 * PA6 - ADC12_IN6
 * PA7 - ADC12_IN7
 *
 * */

#ifdef __cplusplus
 } // extern "C"
#endif

#endif /* ADC_H_ */
