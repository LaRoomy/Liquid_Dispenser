#include <CanImplementation.hpp>
#include <uint8_tools.h>

static CanCallback canCallback;

bool CCanController::canControllerInstanceCreated = false;
CCanController * CCanController::canControllerInstance = nullptr;

CCanController * CCanController::GetInstance()
{
	if(canControllerInstanceCreated == false)
	{
		canControllerInstance = new CCanController();
		if(canControllerInstance != nullptr)
		{
			canControllerInstanceCreated = true;
		}
	}
	return canControllerInstance;
}

CCanController::CCanController()
	: callback(nullptr)
{
	canCallback.dataReceived = CCanController::dataReceived;
	canCallback.errorOccurred = CCanController::errorOccurred;
	can_set_callback(&canCallback);
}

void CCanController::init(PCAN_FILTERS pFilters)
{
	can_init(CAN_NORMAL_MODE, (int)pFilters->numFilters, pFilters->stdIDFilterList);
}

void CCanController::deInit()
{
	can_deinit();
}

void CCanController::writeData(PCAN_DATA pCanData)
{
	if(pCanData != nullptr)
	{
		CanTransmissionData transmissionData;
		transmissionData.stdID = pCanData->stdID;
		transmissionData.data = pCanData->data;
		transmissionData.dataSize = pCanData->dataSize;
		can_result_t res = can_send_data(&transmissionData);

		if(callback != nullptr)
		{
			switch(res)
			{
			case CAN_ERROR_BUSY:
				this->callback->canErrorOccurred("CAN Error: can bus busy.");
				break;
			case CAN_ERROR_INVALID_ARGUMENT:
				this->callback->canErrorOccurred("CAN Error: invalid argument.");
				break;
			case CAN_ERROR_DATASIZE_TOO_LARGE:
				this->callback->canErrorOccurred("CAN Error: data-size too large.");
				break;
			default:
				break;
			}
		}
	}
}

void CCanController::registerCallback(ICanCallback * _callback)
{
	this->callback = _callback;
}

void CCanController::enableLoopBackMode()
{
	can_enable_loop_back_mode();
}

void CCanController::convertCanDataToCharArray(PCAN_DATA canData, char * cArr)
{
	uint8_to_char_array(canData->data, cArr, canData->dataSize);
}

void CCanController::convertCharArrayToCanUint8Array(const char * cArr, uint8_t * out, uint32_t size)
{
	char_to_uint8_array(cArr, out, size);
}

void CCanController::dataReceived(CanTransmissionData * tData)
{
	auto _this_ = canControllerInstance;
	if(_this_ != nullptr)
	{
		if(_this_->callback != nullptr)
		{
			CAN_DATA canData =
			{
				.stdID = tData->stdID,
				.data = tData->data,
				.dataSize = tData->dataSize
			};
			_this_->callback->canDataReceived(&canData);
		}
	}
}

void CCanController::errorOccurred(uint32_t errorCode)
{
	// TODO: implement error handling
}

// ***********************************************************

ICanController * GetCanControllerInstance()
{
	return CCanController::GetInstance();
}
