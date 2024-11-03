#ifndef CANINTERFACE_HPP
#define	CANINTERFACE_HPP

#include <stdint.h>

typedef struct
{
	uint32_t * stdIDFilterList;
	uint32_t numFilters;

}CAN_FILTERS, *PCAN_FILTERS;

typedef struct _CAN_DATA
{
	uint32_t stdID;
	uint8_t * data;
	uint32_t dataSize;

	bool operator== (const _CAN_DATA &d) const
	{
		if(d.stdID == this->stdID && d.dataSize == this->dataSize)
		{
			for(uint32_t i = 0; i < this->dataSize; i++)
			{
				if(d.data[i] != this->data[i])
				{
					return false;
				}
			}
			return true;
		}
		return false;
	}

}CAN_DATA, *PCAN_DATA;

class ICanCallback
{
public:
	virtual ~ICanCallback() = default;
	virtual void canDataReceived(PCAN_DATA pCanData) = 0;
	virtual void canErrorOccurred(const char * errInfo) = 0;
};

class ICanController
{
public:
	virtual ~ICanController() = default;
	virtual void init(PCAN_FILTERS pFilters) = 0;
	virtual void deInit() = 0;
	virtual void writeData(PCAN_DATA pCanData) = 0;
	virtual void registerCallback(ICanCallback * callback) = 0;
	virtual void enableLoopBackMode() = 0;
	virtual void convertCanDataToCharArray(PCAN_DATA canData, char * cArr) = 0;
	virtual void convertCharArrayToCanUint8Array(const char * cArr, uint8_t * out, uint32_t size) = 0;
};

ICanController * GetCanControllerInstance();

#endif // CANINTERFACE_HPP
