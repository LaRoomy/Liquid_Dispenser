#include "Application.hpp"
#include <ConversionTool.hpp>
#include <can_communication_codes.h>

bool Application::appInstanceCreated = false;
Application * Application::hInstance = nullptr;

const int Application::INACTIVE_STATE = 0;
const int Application::ACTIVE_STATE = 1;
const int Application::OUTPUT_STATE = 2;

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
	: canControllerInstance(nullptr),
	  flowControllerInstance(nullptr),
	  loggerInstance(nullptr),
	  verboseLogging(false),
	  currentOutputAmount(0),
	  currentState(INACTIVE_STATE)
{}

void Application::init(ICanController * _canControllerInstance, IFlowController * _flowControllerInstance, ILogger * _loggerInstance)
{
	uint32_t canFilters[] =
		{ OUTPUT_OPERATION_INITIATED_NOTIFICATION_ID, OUTPUT_CONFIRMED_NOTIFICATON_ID, OUTPUT_CANCELLED_NOTIFICATION_ID,
				OUTPUT_STOPPED_NOTIFICATION_ID, SYSTEM_ERROR_OCCURRED_NOTIFICATION_ID };

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

	// TODO: temp
	//this->canControllerInstance->enableLoopBackMode();

	if(_flowControllerInstance == nullptr)
	{
		this->processCriticalError("FlowController was null");
		return;
	}
	this->flowControllerInstance = _flowControllerInstance;
	this->flowControllerInstance->registerCallback(dynamic_cast<IFlowControlCallback*>(this));

	if(_loggerInstance == nullptr)
	{
		this->processCriticalError("Logger was null");
		return;
	}
	this->loggerInstance = _loggerInstance;

	if(this->verboseLogging)
	{
		this->logMessage("Application instance successfully initialized.");
	}
}

void Application::registerCallback(IApplicationCallback * _callback)
{
	this->appCallback = _callback;
}

void Application::processCanTask(void * data)
{
	uint8_t * ptr = (uint8_t*)data;
	uint32_t transmissionID = ConversionTools::fourUint8ToUint32(ptr[0], ptr[1], ptr[2], ptr[3]);
	//uint32_t dataSize = (uint32_t)ptr[4];

	if(transmissionID == OUTPUT_OPERATION_INITIATED_NOTIFICATION_ID)
	{
		this->currentOutputAmount = ConversionTools::fourUint8ToUint32(ptr[5], ptr[6], ptr[7], ptr[8]);
		this->currentState = ACTIVE_STATE;

		if(this->verboseLogging)
		{
			this->logValue("Application::processCanTask: OPERATION_INITIATED notification received. Output amount: ", this->currentOutputAmount);
		}
	}
	else if(transmissionID == OUTPUT_CONFIRMED_NOTIFICATON_ID)
	{
		if(this->appCallback != nullptr)
		{
			this->appCallback->startFlowOperationTask();
		}
		if(this->verboseLogging)
		{
			this->logMessage("Application::processCanTask: OUTPUT_CONFIRMED notification received");
		}
	}
	else if(transmissionID == OUTPUT_CANCELLED_NOTIFICATION_ID)
	{
		this->currentOutputAmount = 0;
		this->currentState = INACTIVE_STATE;

		if(this->verboseLogging)
		{
			this->logMessage("Application::processCanTask: OUTPUT_CANCELLED notification received");
		}
	}
	else if(transmissionID == OUTPUT_STOPPED_NOTIFICATION_ID)
	{
		this->flowControllerInstance->stopWaterFlow();
		this->currentOutputAmount = 0;
		this->currentState = INACTIVE_STATE;

		if(this->verboseLogging)
		{
			this->logMessage("Application::processCanTask: OUTPUT_STOPPED notification received");
		}
	}
	else if(transmissionID == SYSTEM_ERROR_OCCURRED_NOTIFICATION_ID)
	{
		this->currentOutputAmount = 0;
		this->currentState = INACTIVE_STATE;

		if(this->verboseLogging)
		{
			this->logMessage("Application::processCanTask: SYSTEM_ERROR notification received");
		}
	}
	else
	{
		this->logMessage("Application::processCanTask: Invalid transmission ID");
	}

	this->canQueueDataList.Delete(ptr);
}

void Application::processFlowControlOperation()
{
	if(this->currentOutputAmount != 0 && this->currentState == ACTIVE_STATE)
	{
		this->flowControllerInstance->startWaterFlow(amountToDeciliter());
		this->currentState = OUTPUT_STATE;

		if(this->verboseLogging)
		{
			this->logValue("Output started. Output amount: ", this->currentOutputAmount);
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

int Application::getApplicationState()
{
	return this->currentState;
}

void Application::cleanUp()
{
	this->canQueueDataList.eraseAll();
	this->currentState = INACTIVE_STATE;
	this->currentOutputAmount = 0;
}

void Application::canDataReceived(PCAN_DATA pCanData)
{
	if(pCanData != nullptr)
	{
		const int payLoadDataStartIndex = 5;

		// enqueue the transmission ID
		uint8_t buffer[128] = { 0 };
		ConversionTools::uint32To4Uint8Buffer(pCanData->stdID, buffer);

		// enqueue dataSize
		buffer[4] = (uint8_t)pCanData->dataSize;

		for(uint32_t i = 0; i < 124; i++)
		{
			if(i == (pCanData->dataSize))
			{
				break;
			}
			buffer[i + payLoadDataStartIndex] = pCanData->data[i];
		}

		auto addr = this->canQueueDataList.Store(buffer, pCanData->dataSize + payLoadDataStartIndex);
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

void Application::flowStopped()
{
	this->currentOutputAmount = 0;

	uint8_t rawData[] = {0};

	CAN_DATA canData;
	canData.data = rawData;
	canData.dataSize = 1;
	canData.stdID = OUTPUT_COMPLETE_NOTIFICATION_ID;

	this->canControllerInstance->writeData(&canData);
}

void Application::containerEmpty()
{
	const char errorMsg[] = "Error: container empty!";
	auto len = strlen(errorMsg);
	uint8_t uArr[128] = {0};

	this->canControllerInstance->convertCharArrayToCanUint8Array(errorMsg, uArr, len);

	CAN_DATA canData;
	canData.data = uArr;
	canData.dataSize = len;
	canData.stdID = SYSTEM_ERROR_OCCURRED_NOTIFICATION_ID;

	this->canControllerInstance->writeData(&canData);
}

void Application::processCriticalError(const char * msg)
{
	if(this->appCallback != nullptr)
	{
		this->appCallback->handleCriticalError(msg);
	}
}

void Application::logMessage(const char * msg)
{
	if(this->loggerInstance != nullptr)
	{
		this->loggerInstance->LogStringLn(msg);
	}
}

void Application::logValue(const char * msg, unsigned int value)
{
	char buffer[256] = {'\0'};
	sprintf(buffer, "%s: %u", msg, value);
	this->logMessage(buffer);
}

uint32_t Application::amountToDeciliter()
{
	if(this->currentOutputAmount < FLOWCONTROL_QUANTITY_INDETERMINATE)
	{
		return (this->currentOutputAmount/100U);
	}
	else
	{
		return this->currentOutputAmount;
	}
}





