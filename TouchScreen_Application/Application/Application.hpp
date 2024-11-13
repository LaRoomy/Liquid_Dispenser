#ifndef APPLICATION_DSP_HPP
#define APPLICATION_DSP_HPP

#include <CanInterface.hpp>
#include <LoggerInterface.hpp>
#include <DisplayInterface.hpp>
#include <TouchInterface.hpp>
#include <StaticDataList.hpp>

constexpr uint32_t DISPLAYINSTRUCTION_SHOW_INACTIVE_SCREEN = 0;
constexpr uint32_t DISPLAYINSTRUCTION_SHOW_CONFIRM_CANCEL_SCREEN = 1;
constexpr uint32_t DISPLAYINSTRUCTION_SHOW_STOP_SCREEN = 2;
constexpr uint32_t DISPLAYINSTRUCTION_SHOW_ERROR_SCREEN = 3;

class IApplicationCallback
{
public:
	virtual ~IApplicationCallback() = default;
	virtual void enqueueDisplayInstruction(void * instruction) = 0;
	virtual void enqueueCanData(void * data) = 0;
	virtual void handleCriticalError(const char * msg) = 0;
};

class Application : public ICanCallback
{
public:
	static Application * GetInstance();

	void init(ICanController * _canControllerInstance, IDisplayController * _displayControllerInstance,
			ITouchController * touchControllerInstance, ILogger * _loggerInstance);

	void registerCallback(IApplicationCallback * callback);

	void processDisplayTask(void * instruction);
	void processTouchTask();
	void processCanTask(void * data);

	void canDataReceived(PCAN_DATA pCanData) override;
	void canErrorOccurred(const char * errInfo) override;

	void enableVerboseLogging();
	void disableVerboseLogging();

	static const int INACTIVE_STATE;
	static const int CONFIRM_OR_CANCEL_STATE;
	static const int STOP_OUTPUT_STATE;
	static const int ERROR_STATE;
	static const int INVALID_STATE;

	int getCurrentState();
	void CleanUp();

private:
	static bool appInstanceCreated;
	static Application * hInstance;

	Application();

	IApplicationCallback * appCallback;

	ICanController * canControllerInstance;
	IDisplayController * displayControllerInstance;
	ITouchController * touchControllerInstance;
	ILogger * loggerInstance;

	StaticDataList<uint32_t> displayInstructionList;
	StaticDataList<uint8_t> canQueueDataList;

	bool verboseLogging;
	bool touchActionInProgress;
	uint32_t currentOutputAmount;
	char lastErrorMessage[128];

	void drawInactiveScreen();
	void drawConfirmOrCancelScreen(uint32_t outputAmount);
	void drawStopOutputScreen();
	void drawErrorScreen(const char * msg);
	void erasePreviousScreen();

	void processCriticalError(const char * msg);
	void logMessage(const char * msg);
	void logValue(const char * msg, unsigned int value);
	bool isPointInRect(const PPOINT pt, const PRECT rc);

	class ApplicationState
	{
	public:
		int getState() { return this->state; }
		int getPreviousState() { return this->previousState; }
		void setState(const int state) {
			this->previousState = this->state;
			this->state = state; }
	private:
		int state = Application::INACTIVE_STATE;
		int previousState = Application::INVALID_STATE;
	};

	ApplicationState appState;
};

#endif // APPLICATION_DSP_HPP
