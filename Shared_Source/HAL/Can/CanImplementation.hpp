#ifndef CANIMPLEMENTATION_HPP
#define CANIMPLEMENTATION_HPP

#include <CanInterface.hpp>
#include <can.h>

class CCanController : public ICanController
{
public:
	static CCanController * GetInstance();

	void init(PCAN_FILTERS pFilters) override;
	void deInit() override;
	void writeData(PCAN_DATA pCanData) override;
	void registerCallback(ICanCallback * _callback) override;
	void enableLoopBackMode() override;
	void convertCanDataToCharArray(PCAN_DATA canData, char * cArr) override;
	void convertCharArrayToCanUint8Array(const char * cArr, uint8_t * out, uint32_t size) override;

private:
	CCanController();

	static bool canControllerInstanceCreated;
	static CCanController * canControllerInstance;

	ICanCallback * callback;

	static void dataReceived(CanTransmissionData * tData);
	static void errorOccurred(uint32_t errorCode);
};

#endif // CANIMPLEMENTATION_HPP
