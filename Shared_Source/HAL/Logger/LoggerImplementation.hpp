#ifndef LOGGERIMPLEMENTATION_HPP
#define LOGGERIMPLEMENTATION_HPP

#include <LoggerInterface.hpp>
#include <SerialInterface.hpp>

class CLogger : public ILogger
{
public:
	static CLogger * GetInstance();

	void LogString(const char * str) override;
	void LogStringLn(const char * str) override;

private:
	static bool loggerInstanceCreated;
	static CLogger * hInstance;

	CLogger();

	ISerial * serialCom;
};

#endif // LOGGERIMPLEMENTATION_HPP
