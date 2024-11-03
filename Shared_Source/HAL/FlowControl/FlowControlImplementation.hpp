#ifndef FLOWCONTROLIMPLEMENTATION_HPP
#define FLOWCONTROLIMPLEMENTATION_HPP

#include <FlowControlInterface.hpp>
#include <flow_control.h>

class CFlowController : public IFlowController
{
public:
	static CFlowController * GetInstance();

	void startWaterFlow(uint32_t quantity_dl) override;
	void stopWaterFlow() override;
	bool isFlowActive() override;
	void registerCallback(IFlowControlCallback * callback) override;

private:
	static bool flowControlInstanceCreated;
	static CFlowController * hInstance;

	IFlowControlCallback * fcCallback;
	FlowControlObject fcObject;

	CFlowController();

	static void flow_control_callback(uint32_t event);
};

#endif // FLOWCONTROLIMPLEMENTATION_HPP
