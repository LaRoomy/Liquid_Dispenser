#ifndef LOGGERINTERFACE_HPP
#define LOGGERINTERFACE_HPP

class ILogger
{
public:
	virtual ~ILogger() = default;
	virtual void LogString(const char * str) = 0;
	virtual void LogStringLn(const char * str) = 0;
};

ILogger * GetLoggerInstance();

#endif // LOGGERINTERFACE_HPP
