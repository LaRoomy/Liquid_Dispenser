#include "Application.hpp"
#include <string.h>
#include <can_communication_codes.h>
#include <common.h>
#include <ConversionTool.hpp>

bool Application::appInstanceCreated = false;
Application * Application::hInstance = nullptr;

const int Application::NORMAL_STATE = 0;
const int Application::BUSY_STATE = 1;
const int Application::ERROR_STATE = 2;

Application * Application::GetInstance()
{
	if(appInstanceCreated == false)
	{
		hInstance = new Application();
		if(hInstance != nullptr)
		{
			appInstanceCreated = true;
		}
	}
	return hInstance;
}

Application::Application()
: appCallback(nullptr),
  nfcControllerInstance(nullptr),
  canControllerInstance(nullptr),
  logger(nullptr),
  currentState(Application::NORMAL_STATE),
  verboseLogging(false)
{}

void Application::init(INfcController * _nfcControllerInstance, ICanController * _canControllerInstance, ILogger * _logger)
{
	uint32_t canFilters[] =
		{ OUTPUT_CONFIRMED_NOTIFICATON_ID, OUTPUT_CANCELLED_NOTIFICATION_ID,
			OUTPUT_COMPLETE_NOTIFICATION_ID, OUTPUT_STOPPED_NOTIFICATION_ID, SYSTEM_ERROR_OCCURRED_NOTIFICATION_ID };

	CAN_FILTERS filters;
	filters.numFilters = 5;
	filters.stdIDFilterList = canFilters;

	if(_canControllerInstance == nullptr)
	{
		this->processCriticalError("CAN controller was null");
		return;
	}
	this->canControllerInstance = _canControllerInstance;
	this->canControllerInstance->init(&filters);
	this->canControllerInstance->registerCallback(dynamic_cast<ICanCallback*>(this));

	// TODO: TEMP!!!
	//this->canControllerInstance->enableLoopBackMode();

	if(_nfcControllerInstance == nullptr)
	{
		this->processCriticalError("NFC controller was null");
		return;
	}
	this->nfcControllerInstance = _nfcControllerInstance;
	this->nfcControllerInstance->registerCallback(dynamic_cast<INfcCallback*>(this));

	this->enterNormalState();

	if(_logger == nullptr)
	{
		this->processCriticalError("Logger object was null");
		return;
	}
	this->logger = _logger;

	if(this->verboseLogging)
	{
		this->logMessage("Application instance successfully initialized.");
	}
}

void Application::registerCallback(IApplicationCallback * _callback)
{
	this->appCallback = _callback;

	if(this->verboseLogging)
	{
		this->logMessage("Application callback registered.");
	}
}

void Application::processCanTask(void * tData)
{
	uint8_t * ptr = (uint8_t*)tData;
	uint32_t transmissionID = ConversionTools::fourUint8ToUint32(ptr[0], ptr[1], ptr[2], ptr[3]);

	// - received notifications
	// 		-> output confirmed - goto output blink mode
	//		-> output canceled - goto normal mode
	// 		-> output stopped - goto normal mode
	//		-> operation finished - goto normal mode
	//		-> error notification - goto error mode

	if(transmissionID == OUTPUT_COMPLETE_NOTIFICATION_ID)
	{
		this->enterNormalState();

		if(this->verboseLogging)
		{
			this->logMessage("CAN notification received: OUTPUT_COMPLETE.");
		}
	}
	else if(transmissionID == OUTPUT_CANCELLED_NOTIFICATION_ID)
	{
		this->enterNormalState();

		if(this->verboseLogging)
		{
			this->logMessage("CAN notification received: OUTPUT_CANCELLED.");
		}
	}
	else if(transmissionID == OUTPUT_CONFIRMED_NOTIFICATON_ID)
	{
		this->enterBusyState();

		if(this->verboseLogging)
		{
			this->logMessage("CAN notification received: OUTPUT_CONFIRMED.");
		}
	}
	else if(transmissionID == OUTPUT_STOPPED_NOTIFICATION_ID)
	{
		this->enterNormalState();

		if(this->verboseLogging)
		{
			this->logMessage("CAN notification received: OUTPUT_STOPPED.");
		}
	}
	else if(transmissionID == SYSTEM_ERROR_OCCURRED_NOTIFICATION_ID)
	{
		this->enterErrorState();

		if(this->verboseLogging)
		{
			this->logMessage("CAN notification received: SYSTEM_ERROR.");
		}
	}
	else
	{
		// unknown state
		this->logMessage("Application::processCanTask - unknown CAN-transmission ID.");
	}

	this->canQueueDataList.Delete(ptr);
}

void Application::processNfcTask(void * tData)
{
	char * ptr = (char*)tData;

	// - when user has scanned app
	//  1	-> write response to app
	//  2	-> put leds in output mode
	//	3	-> post CAN message with the output amount

	if(this->getCurrentState() == Application::NORMAL_STATE)
	{
		// format: OUTPUT:<value> (value=milliliter)
		uint32_t outputAmount = ConversionTools::extractOutputAmount(ptr);
		if(outputAmount != INVALID_OUTPUT_AMOUNT)
		{
			if(verboseLogging)
			{
				this->logMessage("NFC Transmission: Valid output data received.");
				this->logValue("Output amount", outputAmount);
			}

			this->nfcControllerInstance->writeData("COMMAND:CONFIRMED");
			this->enterBusyState();

			uint8_t data[4];
			ConversionTools::uint32To4Uint8Buffer(outputAmount, data);

			CAN_DATA canData;
			canData.stdID = OUTPUT_OPERATION_INITIATED_NOTIFICATION_ID;
			canData.dataSize = 4;
			canData.data = data;
			this->canControllerInstance->writeData(&canData);
		}
		else
		{
			this->logMessage("NFC transmission: Invalid format / Invalid output amount.");
		}
	}
	else
	{
		// system in wrong state
		this->logMessage("NFC transmission: Not accepted - System not ready.");
	}

	this->nfcQueueDataList.Delete(ptr);
}

int Application::getCurrentState()
{
	return this->currentState;
}

void Application::enterNormalState()
{
	this->nfcControllerInstance->switchBlueLED(ON);
	this->nfcControllerInstance->switchGreenLED(OFF);
	this->nfcControllerInstance->switchYellowLED(OFF);
	this->currentState = Application::NORMAL_STATE;

	if(this->verboseLogging)
	{
		this->logMessage("Application entered NORMAL state");
	}
}

void Application::enterErrorState()
{
	this->nfcControllerInstance->switchBlueLED(OFF);
	this->nfcControllerInstance->switchGreenLED(OFF);
	this->nfcControllerInstance->switchYellowLED(ON);
	this->currentState = Application::ERROR_STATE;

	if(this->verboseLogging)
	{
		this->logMessage("Application entered ERROR state");
	}
}

void Application::enterBusyState()
{
	this->nfcControllerInstance->switchBlueLED(OFF);
	this->nfcControllerInstance->switchGreenLED(ON);
	this->nfcControllerInstance->switchYellowLED(OFF);
	this->currentState = Application::BUSY_STATE;

	if(this->verboseLogging)
	{
		this->logMessage("Application entered BUSY state");
	}
}

void Application::logMessage(const char * msg)
{
	if(this->logger != nullptr)
	{
		this->logger->LogStringLn(msg);
	}
}

void Application::logValue(const char * msg, unsigned int value)
{
	char buffer[256] = {'\0'};
	sprintf(buffer, "%s: %u", msg, value);
	this->logMessage(buffer);
}

void Application::processCriticalError(const char * msg)
{
	if(this->appCallback != nullptr)
	{
		this->appCallback->onCriticalError(msg);
	}
}

void Application::canDataReceived(PCAN_DATA pCanData)
{
	if(pCanData != nullptr)
	{
		// enqueue the transmission ID
		uint8_t buffer[128] = { 0 };
		ConversionTools::uint32To4Uint8Buffer(pCanData->stdID, buffer);

		for(uint32_t i = 0; i < 124; i++)
		{
			if(i == pCanData->dataSize)
			{
				break;
			}
			buffer[i + 4] = pCanData->data[i];
		}

		auto addr = this->canQueueDataList.Store(buffer, pCanData->dataSize + 4);
		if(addr != nullptr)
		{
			if(this->appCallback != nullptr)
			{
				this->appCallback->enqueueCanData(addr);
			}
		}
		else
		{
			this->logMessage("Application::canDataReceived - Error while storing data in the CAN queue data list.");
		}
	}
}

void Application::canErrorOccurred(const char * errInfo)
{
	this->logMessage(errInfo);
}

void Application::nfcDataReceived(const char * data)
{
	if(data != nullptr)
	{
		auto addr = this->nfcQueueDataList.Store(data, strlen(data));
		if(addr != nullptr)
		{
			if(this->appCallback != nullptr)
			{
				this->appCallback->enqueueNfcData(addr);
			}
		}
		else
		{
			this->logMessage("Application::canDataReceived - Error while storing data in the NFC queue data list.");
		}
	}
}

void Application::enableVerboseLogging()
{
	this->verboseLogging = true;
}

void Application::disableVerboseLogging()
{
	this->verboseLogging = false;
}









