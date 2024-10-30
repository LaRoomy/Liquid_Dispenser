#ifndef NFC07A1_EXT_H_
#define NFC07A1_EXT_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include <stm32f4xx.h>

#define		NFC07A1_OK					0
#define		NFC07A1_ERR					(-1)
#define		NFC07A1_ERR_INVALID_ARG		(-22)
#define		NFC07A1_ERR_ALREADY_INIT	(-98)

#define		NFC07A1_SECSESSION_FAIL		(-140)
#define		NFC07A1_GPO1_PG_FAIL		(-141)
#define		NFC07A1_GPO2_PG_FAIL		(-142)
#define		NFC07A1_FTM_R_PG_FAIL		(-143)

#define		NFC07A1_ENABLE_GPO_FAIL		(-150)
#define		NFC07A1_ENABLE_FTM_FAIL		(-151)
#define		NFC07A1_INVALID_DATA_LEN	(-152)
#define		NFC07A1_INVALID_MB_VAL		(-153)

#define		NFC07A1_MBX_SIZE_EXCEEDED	(-160)

typedef struct
{
	uint32_t apbClockFrequency;
	uint32_t i2cNumber;
	GPIO_TypeDef * led1Port;// can be null
	uint32_t led1Pin;
	GPIO_TypeDef * led2Port;// can be null
	uint32_t led2Pin;
	GPIO_TypeDef * led3Port;// can be null
	uint32_t led3Pin;
	GPIO_TypeDef * gpoInterruptPort;// can be null
	uint32_t gpoInterruptPin;
	void (*nfc_data_received_irq)(void);
	char mailbox_buffer[256];

}NFC07A1Object;

typedef int nfc07a1_result_t;

nfc07a1_result_t nfc07a1_init(NFC07A1Object * self);
nfc07a1_result_t nfc07a1_deinit(NFC07A1Object * self);

void nfc07a1_switch_user_led_1(NFC07A1Object * self, uint32_t state);
void nfc07a1_switch_user_led_2(NFC07A1Object * self, uint32_t state);
void nfc07a1_switch_user_led_3(NFC07A1Object * self, uint32_t state);

nfc07a1_result_t nfc07a1_read_mailbox(NFC07A1Object * self);
nfc07a1_result_t nfc07a1_write_mailbox(NFC07A1Object * self, const char* data, uint32_t length);

#ifdef __cplusplus
 } // extern "C"
#endif

#endif /* NFC07A1_EXT_H_ */
