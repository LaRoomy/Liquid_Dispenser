#ifndef APPLICATION_NFC_HPP_
#define APPLICATION_NFC_HPP_

#include <NfcInterface.hpp>
#include <CanInterface.hpp>
#include <LoggerInterface.hpp>
#include <StaticDataList.hpp>

class IApplicationCallback
{
public:
	~IApplicationCallback() = default;
	virtual void enqueueCanData(void * data) = 0;
	virtual void enqueueNfcData(void * data) = 0;
	virtual void onCriticalError(const char * msg) = 0;
};

class Application : public ICanCallback,  public INfcCallback
{
public:
	static Application * GetInstance();

	void init(INfcController * _nfcController, ICanController * _canController, ILogger * _logger);

	void registerCallback(IApplicationCallback * _callback);

	void processCanTask(void * tData);
	void processNfcTask(void * tData);

	// can callback
	void canDataReceived(PCAN_DATA pCanData) override;
	void canErrorOccurred(const char * errInfo) override;

	// nfc callback
	void nfcDataReceived(const char * data) override;

	void enableVerboseLogging();
	void disableVerboseLogging();

	static const int BUSY_STATE;
	static const int ERROR_STATE;
	static const int NORMAL_STATE;

	int getCurrentState();

private:
	Application();

	//const uint32_t OUTPUT_MODE_REQUEST_ID = 0xE0U;

	static bool appInstanceCreated;
	static Application * hInstance;

	IApplicationCallback * appCallback;

	StaticDataList<uint8_t> canQueueDataList;
	StaticDataList<char> nfcQueueDataList;

	INfcController * nfcControllerInstance;
	ICanController * canControllerInstance;
	ILogger * logger;

	int currentState;
	bool verboseLogging;

	void enterNormalState();
	void enterErrorState();
	void enterBusyState();

	void logMessage(const char * msg);
	void logValue(const char * msg, unsigned int value);
	void processCriticalError(const char * msg);
};

#endif /* APPLICATION_NFC_HPP_ */
