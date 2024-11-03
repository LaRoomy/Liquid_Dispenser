#include <NFCImplementation.hpp>
#include <nfc07a1.h>
#include <string.h>

NFC07A1Object nfc07a1Obj =
{
	.apbClockFrequency = MCU_CLOCK,
	.i2cNumber = 1,
	.led1Port = GPIOB,
	.led1Pin = 4,
	.led2Port = GPIOB,
	.led2Pin = 5,
	.led3Port = GPIOA,
	.led3Pin = 10,
	.gpoInterruptPort = GPIOA,
	.gpoInterruptPin = 6,
	.nfc_data_received_irq = CNfcController::nfcDataReceivedCallback
};

bool CNfcController::nfcInstanceCreated = false;
CNfcController * CNfcController::nfcInstance = nullptr;

CNfcController* CNfcController::GetInstance()
{
	if(nfcInstanceCreated == false)
	{
		nfcInstance = new CNfcController();
		if(nfcInstance != nullptr)
		{
			nfcInstanceCreated = true;
		}
	}
	return nfcInstance;
}

CNfcController::CNfcController()
	: callback(nullptr)
{
	nfc07a1_init(&nfc07a1Obj);
}

void CNfcController::writeData(const char * data)
{
	if(data != nullptr)
	{
		nfc07a1_write_mailbox(&nfc07a1Obj, data, strlen(data));
	}
}

void CNfcController::registerCallback(INfcCallback * _callback)
{
	this->callback = _callback;
}

void CNfcController::nfcDataReceivedCallback()
{
	auto _this_ = nfcInstance;
	if(_this_ != nullptr)
	{
		if(_this_->callback != nullptr)
		{
			nfc07a1_read_mailbox(&nfc07a1Obj);
			_this_->callback->nfcDataReceived(nfc07a1Obj.mailbox_buffer);
		}
	}
}

void CNfcController::switchGreenLED(uint32_t state)
{
	nfc07a1_switch_user_led_1(&nfc07a1Obj, state);
}

void CNfcController::switchBlueLED(uint32_t state)
{
	nfc07a1_switch_user_led_2(&nfc07a1Obj, state);
}

void CNfcController::switchYellowLED(uint32_t state)
{
	nfc07a1_switch_user_led_3(&nfc07a1Obj, state);
}

// ****************************************************

INfcController * GetNfcControllerInstance()
{
	return CNfcController::GetInstance();
}
