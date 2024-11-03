#include "FlowControlImplementation.hpp"
#include <flow_control.h>

bool CFlowController::flowControlInstanceCreated = false;
CFlowController * CFlowController::hInstance = nullptr;

CFlowController * CFlowController::GetInstance()
{
	if(flowControlInstanceCreated == false)
	{
		hInstance = new CFlowController();
		if(hInstance != nullptr)
		{
			flowControlInstanceCreated = true;
		}
	}
	return hInstance;
}

CFlowController::CFlowController()
	: fcCallback(nullptr)
{
	fcObject.flowRateSensePin = 0;
	fcObject.flowRateSensePort = GPIOB;
	fcObject.pumpPin = 0;
	fcObject.pumpPort = GPIOC;
	fcObject.valvePin = 1;
	fcObject.valvePort = GPIOC;
	fcObject.pressureADCNumber = 1;
	fcObject.pressureADCPin = 0;
	fcObject.pressureADCPort = GPIOA;
	fcObject.valveOpenThresholdValue = 2100;
	fcObject.valvePressureLostUnderCutValue = 1000;
	fcObject.apbTimerFrequency = MCU_CLOCK;
	fcObject.timerNumber = 2;
	fcObject.flow_control_callback = CFlowController::flow_control_callback;

	flow_control_init(&fcObject);
}

void CFlowController::startWaterFlow(uint32_t quantity_dl)
{
	flow_control_start_water_flow(&fcObject, quantity_dl);
}

void CFlowController::stopWaterFlow()
{
	flow_control_stop_water_flow(&fcObject);
}

bool CFlowController::isFlowActive()
{
	return flow_control_is_active(&fcObject) == TRUE ? true : false;
}

void CFlowController::registerCallback(IFlowControlCallback * callback)
{
	this->fcCallback = callback;
}

void CFlowController::flow_control_callback(uint32_t event)
{
	auto this_ = CFlowController::GetInstance();

	if(this_->fcCallback != nullptr)
	{
		if(event == FLOWCONTROL_EVENT_STOPPED)
		{
			this_->fcCallback->flowStopped();
		}
		else if(event == FLOWCONTROL_EVENT_PRESSURE_LOST)
		{
			this_->fcCallback->containerEmpty();
		}
	}
}

// *****************************************

IFlowController * GetFlowControllerInstance()
{
	return reinterpret_cast<IFlowController*>(CFlowController::GetInstance());
}
