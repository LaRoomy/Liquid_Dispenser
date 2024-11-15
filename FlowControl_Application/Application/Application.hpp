#ifndef APPLICATION_HPP_
#define APPLICATION_HPP_

#include <CanInterface.hpp>
#include <LoggerInterface.hpp>
#include <FlowControlInterface.hpp>
#include <StaticDataList.hpp>

class IApplicationCallback
{
public:
	virtual ~IApplicationCallback() = default;
	virtual void enqueueCanData(void * data) = 0;
	virtual void startFlowOperationTask() = 0;
	virtual void handleCriticalError(const char * msg) = 0;
};

class Application : public ICanCallback, public IFlowControlCallback
{
public:
	static Application * GetInstance();

	void init(ICanController * _canControllerInstance, IFlowController * _flowControllerInstance, ILogger * _loggerInstance);

	void registerCallback(IApplicationCallback * _callback);

	void processCanTask(void * data);
	void processFlowControlOperation();

	void enableVerboseLogging();
	void disableVerboseLogging();
	void cleanUp();
	int getApplicationState();

	// can callback
	void canDataReceived(PCAN_DATA pCanData) override;
	void canErrorOccurred(const char * errInfo) override;

	// flow control callback
	void flowStopped() override;
	void containerEmpty() override;

	static const int INACTIVE_STATE;
	static const int ACTIVE_STATE;
	static const int OUTPUT_STATE;

private:
	static bool appInstanceCreated;
	static Application * hInstance;

	Application();

	ICanController * canControllerInstance;
	IFlowController * flowControllerInstance;
	ILogger * loggerInstance;

	IApplicationCallback * appCallback;

	StaticDataList<uint8_t> canQueueDataList;

	bool verboseLogging;
	uint32_t currentOutputAmount;
	int currentState;

	void processCriticalError(const char * msg);
	void logMessage(const char * msg);
	void logValue(const char * msg, unsigned int value);

	uint32_t amountToDeciliter();
};


#endif /* APPLICATION_HPP_ */
