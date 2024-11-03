#ifndef FLOWCONTROLINTERFACE_HPP
#define FLOWCONTROLINTERFACE_HPP

#include <stdint.h>

constexpr uint32_t FLOWCONTROL_QUANTITY_INDETERMINATE = (0xffffff00);
constexpr uint32_t FLOWCONTROL_QUANTITY_0_25_L = (0xffffff01);
constexpr uint32_t FLOWCONTROL_QUANTITY_0_33_L = (0xffffff02);
constexpr uint32_t FLOWCONTROL_QUANTITY_0_5_L = (0xffffff03);
constexpr uint32_t FLOWCONTROL_QUANTITY_0_75_L = (0xffffff04);
constexpr uint32_t FLOWCONTROL_QUANTITY_1_L = (0xffffff05);

class IFlowControlCallback
{
public:
	virtual ~IFlowControlCallback() = default;
	virtual void flowStopped() = 0;
	virtual void containerEmpty() = 0;
};

class IFlowController
{
public:
	virtual ~IFlowController() = default;
	virtual void startWaterFlow(uint32_t quantity_dl) = 0;
	virtual void stopWaterFlow() = 0;
	virtual bool isFlowActive() = 0;
	virtual void registerCallback(IFlowControlCallback * callback) = 0;
};

IFlowController * GetFlowControllerInstance();

#endif // FLOWCONTROLINTERFACE_HPP
