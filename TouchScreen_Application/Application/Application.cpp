#include "Application.hpp"
#include <ConversionTool.hpp>
#include <can_communication_codes.h>
#include <shape.h>

#include "laroomy_image.h"
#include "confirm_image.h"
#include "cancel_image.h"
#include "stop_output_image.h"
#include "error_image.h"

RECT CONFIRM_BUTTON_RECT = {20, 40, 120, 120};
RECT CANCEL_BUTTON_RECT = {180, 40, 120, 120};
RECT STOP_OUTPUT_BUTTON_RECT = {70, 10, 180, 180};
RECT ERROR_IMAGE_RECT = {100, 40, 120, 108};

bool Application::appInstanceCreated = false;
Application * Application::hInstance = nullptr;

const int Application::INACTIVE_STATE = 0;
const int Application::CONFIRM_OR_CANCEL_STATE = 1;
const int Application::STOP_OUTPUT_STATE = 2;
const int Application::ERROR_STATE = 3;
const int Application::INVALID_STATE = 99;

Application * Application::GetInstance()
{
	if(Application::appInstanceCreated == false)
	{
		Application::hInstance = new Application();
		if(Application::hInstance != nullptr)
		{
			Application::appInstanceCreated = true;
		}
	}
	return Application::hInstance;
}

Application::Application()
	: appCallback(nullptr),
	  canControllerInstance(nullptr),
	  displayControllerInstance(nullptr),
	  touchControllerInstance(nullptr),
	  loggerInstance(nullptr),
	  verboseLogging(false),
	  touchActionInProgress(false),
	  currentOutputAmount(0)
{
	for(int i = 0; i < 128; i++)
		this->lastErrorMessage[i] = '\0';
}

void Application::init(ICanController * _canControllerInstance, IDisplayController * _displayControllerInstance,
		ITouchController * _touchControllerInstance, ILogger * _loggerInstance)
{
	uint32_t canFilters[] =
		{ OUTPUT_OPERATION_INITIATED_NOTIFICATION_ID,
			OUTPUT_COMPLETE_NOTIFICATION_ID, SYSTEM_ERROR_OCCURRED_NOTIFICATION_ID };

	CAN_FILTERS filters;
	filters.numFilters = 3;
	filters.stdIDFilterList = canFilters;

	if(_canControllerInstance == nullptr)
	{
		this->processCriticalError("CAN controller was null");
		return;
	}
	this->canControllerInstance = _canControllerInstance;
	this->canControllerInstance->init(&filters);
	this->canControllerInstance->registerCallback(dynamic_cast<ICanCallback*>(this));

	if(_displayControllerInstance == nullptr)
	{
		this->processCriticalError("DisplayController instance was null");
		return;
	}
	this->displayControllerInstance = _displayControllerInstance;

	if(_touchControllerInstance == nullptr)
	{
		this->processCriticalError("TouchController instance was null");
		return;
	}
	this->touchControllerInstance = _touchControllerInstance;

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

	this->drawInactiveScreen();
}

void Application::registerCallback(IApplicationCallback * callback)
{
	this->appCallback = callback;
}

void Application::processDisplayTask(void * instruction)
{
	uint32_t uInstruction = *((uint32_t*)instruction);

	// four screen conditions
	// - inactive screen
	// - confirmation / cancel screen
	// - abort action screen
	// - error screen

	if(uInstruction == DISPLAYINSTRUCTION_SHOW_INACTIVE_SCREEN)
	{
		this->drawInactiveScreen();
	}
	else if(uInstruction == DISPLAYINSTRUCTION_SHOW_CONFIRM_CANCEL_SCREEN)
	{
		this->drawConfirmOrCancelScreen(this->currentOutputAmount);
	}
	else if(uInstruction == DISPLAYINSTRUCTION_SHOW_STOP_SCREEN)
	{
		this->drawStopOutputScreen();
	}
	else if(uInstruction == DISPLAYINSTRUCTION_SHOW_ERROR_SCREEN)
	{
		this->drawErrorScreen(this->lastErrorMessage);
	}
	else
	{
		if(this->verboseLogging)
		{
			this->logMessage("Application::processDisplayTask: Unknown display instruction");
		}
	}
	this->displayInstructionList.Delete((uint32_t*)instruction);
}

void Application::processTouchTask()
{
	if(this->touchControllerInstance->isTouchPressed())
	{
		if(this->touchActionInProgress == false)
		{
			this->touchActionInProgress = true;

			uint16_t x, y;
			this->touchControllerInstance->getCoordinates(&x, &y);

			POINT pt;
			pt.x = (int)x;
			pt.y = (int)y;

			uint8_t rawData[] = {0};

			CAN_DATA canData;
			canData.data = rawData;
			canData.dataSize = 1;
			canData.stdID = INVALID_NOTIFICATION_ID;

			uint32_t ** addr = nullptr;

			if(this->appState.getState() == Application::CONFIRM_OR_CANCEL_STATE)
			{
				if(isPointInRect(&pt, &CONFIRM_BUTTON_RECT))
				{
					// confirm button pressed
					canData.stdID = OUTPUT_CONFIRMED_NOTIFICATON_ID;
					this->appState.setState(Application::STOP_OUTPUT_STATE);
					addr = this->displayInstructionList.Store(&DISPLAYINSTRUCTION_SHOW_STOP_SCREEN, 1);

					if(this->verboseLogging)
					{
						this->logMessage("Application::processTouchTask: confirm button pressed");
					}
				}
				else if(isPointInRect(&pt, &CANCEL_BUTTON_RECT))
				{
					// cancel button pressed
					canData.stdID = OUTPUT_CANCELLED_NOTIFICATION_ID;
					this->appState.setState(Application::INACTIVE_STATE);
					addr = this->displayInstructionList.Store(&DISPLAYINSTRUCTION_SHOW_INACTIVE_SCREEN, 1);

					if(this->verboseLogging)
					{
						this->logMessage("Application::processTouchTask: cancel button pressed");
					}
				}
			}
			else if(this->appState.getState() == Application::STOP_OUTPUT_STATE)
			{
				if(isPointInRect(&pt, &STOP_OUTPUT_BUTTON_RECT))
				{
					// stop output button pressed
					canData.stdID = OUTPUT_STOPPED_NOTIFICATION_ID;
					this->appState.setState(Application::INACTIVE_STATE);
					addr = this->displayInstructionList.Store(&DISPLAYINSTRUCTION_SHOW_INACTIVE_SCREEN, 1);

					if(this->verboseLogging)
					{
						this->logMessage("Application::processTouchTask: stop button pressed");
					}
				}
			}
			else
			{
				if(this->verboseLogging)
				{
					this->logMessage("Application::processTouchTask: Useless touch action received");
				}
			}

			if(canData.stdID != INVALID_NOTIFICATION_ID)
			{
				this->canControllerInstance->writeData(&canData);
			}

			if(addr != nullptr)
			{
				if(this->appCallback != nullptr)
				{
					this->appCallback->enqueueDisplayInstruction(addr);
				}
			}
		}
	}
	else
	{
		this->touchActionInProgress = false;
	}
}

void Application::processCanTask(void * data)
{
	uint32_t ** addr = nullptr;
	uint8_t * ptr = (uint8_t*)data;
	uint32_t transmissionID = ConversionTools::fourUint8ToUint32(ptr[0], ptr[1], ptr[2], ptr[3]);
	uint32_t dataSize = (uint32_t)ptr[4];

	if(transmissionID == OUTPUT_COMPLETE_NOTIFICATION_ID)
	{
		addr = this->displayInstructionList.Store(&DISPLAYINSTRUCTION_SHOW_INACTIVE_SCREEN, 1);
		this->appState.setState(Application::INACTIVE_STATE);

		if(this->verboseLogging)
		{
			this->logMessage("Application::processCanTask: OUTPUT_COMPLETE notification received.");
		}
	}
	else if(transmissionID == OUTPUT_OPERATION_INITIATED_NOTIFICATION_ID)
	{
		addr = this->displayInstructionList.Store(&DISPLAYINSTRUCTION_SHOW_CONFIRM_CANCEL_SCREEN, 1);

		this->appState.setState(Application::CONFIRM_OR_CANCEL_STATE);
		this->currentOutputAmount = ConversionTools::fourUint8ToUint32(ptr[5], ptr[6], ptr[7], ptr[8]);

		if(this->verboseLogging)
		{
			this->logMessage("Application::processCanTask: OPERATION_INITIATED notification received.");
		}
	}
	else if(transmissionID == SYSTEM_ERROR_OCCURRED_NOTIFICATION_ID)
	{
		addr = this->displayInstructionList.Store(&DISPLAYINSTRUCTION_SHOW_ERROR_SCREEN, 1);

		this->appState.setState(Application::ERROR_STATE);

		uint8_t rawData[128] = {0};
		for(uint32_t i = 0; i < dataSize; i++)
		{
			rawData[i] = ptr[i+5];
		}
		CAN_DATA cData;
		cData.data = rawData;
		cData.dataSize = dataSize;
		this->canControllerInstance->convertCanDataToCharArray(&cData, lastErrorMessage);

		if(this->verboseLogging)
		{
			this->logMessage("Application::processCanTask: SYSTEM_ERROR notification received.");
		}
	}
	else
	{
		this->logMessage("Application::processCanTask: Invalid transmission ID");
	}

	if(addr != nullptr)
	{
		if(this->appCallback != nullptr)
		{
			this->appCallback->enqueueDisplayInstruction(addr);
		}
	}
	else
	{
		this->logMessage("Application::processCanTask: Failed to store display instruction");
	}

	this->canQueueDataList.Delete(ptr);
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

void Application::enableVerboseLogging()
{
	this->verboseLogging = true;
}

void Application::disableVerboseLogging()
{
	this->verboseLogging = false;
}

int Application::getCurrentState()
{
	return this->appState.getState();
}

void Application::CleanUp()
{
	this->canQueueDataList.eraseAll();
	this->displayInstructionList.eraseAll();
	this->touchActionInProgress = false;
}

void Application::drawInactiveScreen()
{
	if(this->appState.getPreviousState() == this->appState.getState())
	{
		return;
	}
	this->erasePreviousScreen();

	if(this->displayControllerInstance != nullptr)
	{
		const char userMsg[] = "Scan App to Start";

		IMAGE lrImage;
		lrImage.imageData = (const uint16_t *)laroomy_image;
		lrImage.pos_x = ((this->displayControllerInstance->getScreenWidth() / 2) - (LAROOMY_IMAGE_WIDTH / 2));
		lrImage.pos_y = ((this->displayControllerInstance->getScreenHeight() / 2) - (LAROOMY_IMAGE_HEIGHT / 2)) - 20;
		lrImage.width = LAROOMY_IMAGE_WIDTH;
		lrImage.heigth = LAROOMY_IMAGE_HEIGHT;

		SIZE textSize;
		this->displayControllerInstance->getTextSize(userMsg, Font::Medium, &textSize);

		POINT textCoordinates;
		textCoordinates.x = (this->displayControllerInstance->getScreenWidth() / 2) - (textSize.cx / 2);
		textCoordinates.y = lrImage.pos_y + lrImage.heigth + 2;

		this->displayControllerInstance->drawImage(&lrImage);
		this->displayControllerInstance->drawText(&textCoordinates, userMsg, Font::Medium, Colors::Gold, Colors::Black);
	}
}

void Application::drawConfirmOrCancelScreen(uint32_t outputAmount)
{
	this->erasePreviousScreen();

	if(this->displayControllerInstance != nullptr)
	{
		char userMsg[255] = {'\0'};
		sprintf(userMsg, "Output: %u ml", (unsigned int)outputAmount);

		IMAGE confirmImage;
		confirmImage.imageData = (const uint16_t *)confirm_image;
		confirmImage.pos_x = CONFIRM_BUTTON_RECT.x;
		confirmImage.pos_y = CONFIRM_BUTTON_RECT.y;
		confirmImage.heigth = CONFIRM_BUTTON_RECT.cy;
		confirmImage.width = CONFIRM_BUTTON_RECT.cx;

		IMAGE cancelImage;
		cancelImage.imageData = (const uint16_t *)cancel_image;
		cancelImage.pos_x = CANCEL_BUTTON_RECT.x;
		cancelImage.pos_y = CANCEL_BUTTON_RECT.y;
		cancelImage.heigth = CANCEL_BUTTON_RECT.cy;
		cancelImage.width = CANCEL_BUTTON_RECT.cx;

		SIZE textSize;
		this->displayControllerInstance->getTextSize(userMsg, Font::Medium, &textSize);

		POINT textCoordinates;
		textCoordinates.x = (this->displayControllerInstance->getScreenWidth() / 2) - (textSize.cx / 2);
		textCoordinates.y = 180;

		this->displayControllerInstance->drawImage(&confirmImage);
		this->displayControllerInstance->drawImage(&cancelImage);
		this->displayControllerInstance->drawText(&textCoordinates, userMsg, Font::Medium, Colors::Gold, Colors::Black);
	}
}

void Application::drawStopOutputScreen()
{
	this->erasePreviousScreen();

	if(this->displayControllerInstance != nullptr)
	{
		const char userMsg[] = "Tap button to stop output..";

		IMAGE stopImage;
		stopImage.imageData = (const uint16_t *)stop_output_image;
		stopImage.pos_x = STOP_OUTPUT_BUTTON_RECT.x;
		stopImage.pos_y = STOP_OUTPUT_BUTTON_RECT.y;
		stopImage.heigth = STOP_OUTPUT_BUTTON_RECT.cy;
		stopImage.width = STOP_OUTPUT_BUTTON_RECT.cx;

		SIZE textSize;
		this->displayControllerInstance->getTextSize(userMsg, Font::Medium, &textSize);

		POINT textCoordinates;
		textCoordinates.x = (this->displayControllerInstance->getScreenWidth() / 2) - (textSize.cx / 2);
		textCoordinates.y = 205;

		this->displayControllerInstance->drawImage(&stopImage);
		this->displayControllerInstance->drawText(&textCoordinates, userMsg, Font::Medium, Colors::Gold, Colors::Black);
	}

}

void Application::drawErrorScreen(const char * msg)
{
	this->erasePreviousScreen();

	if(this->displayControllerInstance != nullptr)
	{
		const char placeholderMsg[] = "No error message provided";

		IMAGE errorImage;
		errorImage.imageData = (const uint16_t *)error_image;
		errorImage.pos_x = ERROR_IMAGE_RECT.x;
		errorImage.pos_y = ERROR_IMAGE_RECT.y;
		errorImage.heigth = ERROR_IMAGE_RECT.cy;
		errorImage.width = ERROR_IMAGE_RECT.cx;

		const char * userMsg = (msg == nullptr) ? placeholderMsg : msg;

		SIZE textSize;
		this->displayControllerInstance->getTextSize(userMsg, Font::Medium, &textSize);

		POINT textCoordinates;
		if(textSize.cx > ((int)this->displayControllerInstance->getScreenWidth()))
		{
			textCoordinates.x = 10;
		}
		else
		{
			textCoordinates.x = (this->displayControllerInstance->getScreenWidth() / 2) - (textSize.cx / 2);
		}
		textCoordinates.y = 170;

		this->displayControllerInstance->drawImage(&errorImage);
		this->displayControllerInstance->drawText(&textCoordinates, userMsg, Font::Medium, Colors::Gold, Colors::Black);
	}
}

void Application::erasePreviousScreen()
{
	if(this->displayControllerInstance != nullptr)
	{
		auto prevState = this->appState.getPreviousState();

		if(prevState == Application::INACTIVE_STATE)
		{
			RECT imageEraseRect;
			imageEraseRect.x = ((this->displayControllerInstance->getScreenWidth() / 2) - (LAROOMY_IMAGE_WIDTH / 2));
			imageEraseRect.y = ((this->displayControllerInstance->getScreenHeight() / 2) - (LAROOMY_IMAGE_HEIGHT / 2)) - 20;
			imageEraseRect.cx = LAROOMY_IMAGE_WIDTH;
			imageEraseRect.cy = LAROOMY_IMAGE_HEIGHT;

			RECT textRect;
			textRect.x = 0;
			textRect.y = imageEraseRect.y + imageEraseRect.cy + 2;
			textRect.cx = this->displayControllerInstance->getScreenWidth();
			textRect.cy = this->displayControllerInstance->getScreenHeight() - textRect.y;

			this->displayControllerInstance->fillRect(&imageEraseRect, Colors::Black);
			this->displayControllerInstance->fillRect(&textRect, Colors::Black);
		}
		else if(prevState == Application::CONFIRM_OR_CANCEL_STATE)
		{
			RECT textRect;
			textRect.x = 0;
			textRect.y = 180;
			textRect.cx = this->displayControllerInstance->getScreenWidth();
			textRect.cy = this->displayControllerInstance->getScreenHeight() - 180;

			this->displayControllerInstance->fillRect(&CONFIRM_BUTTON_RECT, Colors::Black);
			this->displayControllerInstance->fillRect(&CANCEL_BUTTON_RECT, Colors::Black);
			this->displayControllerInstance->fillRect(&textRect, Colors::Black);
		}
		else if(prevState == Application::STOP_OUTPUT_STATE)
		{
			RECT textRect;
			textRect.x = 0;
			textRect.y = 205;
			textRect.cx = this->displayControllerInstance->getScreenWidth();
			textRect.cy = this->displayControllerInstance->getScreenHeight() - 205;

			this->displayControllerInstance->fillRect(&STOP_OUTPUT_BUTTON_RECT, Colors::Black);
			this->displayControllerInstance->fillRect(&textRect, Colors::Black);

		}
		else if(prevState == Application::ERROR_STATE)
		{
			RECT textRect;
			textRect.x = 0;
			textRect.y = 170;
			textRect.cx = this->displayControllerInstance->getScreenWidth();
			textRect.cy = this->displayControllerInstance->getScreenHeight() - 170;

			this->displayControllerInstance->fillRect(&ERROR_IMAGE_RECT, Colors::Black);
			this->displayControllerInstance->fillRect(&textRect, Colors::Black);
		}
		else
		{
			this->displayControllerInstance->fillScreen(Colors::Black);
		}
	}
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

bool Application::isPointInRect(const PPOINT pt, const PRECT rc)
{
	if(pt != NULL && rc != NULL)
	{
		if((pt->x >= rc->x && pt->x <= (rc->x + rc->cx)) && (pt->y >= rc->y && pt->y <= (rc->y + rc->cy)))
		{
			return true;
		}
	}
	return false;
}





