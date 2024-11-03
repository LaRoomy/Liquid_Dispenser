#include "LoggerImplementation.hpp"

bool CLogger::loggerInstanceCreated = false;
CLogger * CLogger::hInstance = nullptr;

CLogger::CLogger()
	: serialCom(nullptr)
{
	this->serialCom = CreateSerialObject();
}


CLogger * CLogger::GetInstance()
{
	if(loggerInstanceCreated == false)
	{
		hInstance = new CLogger();
		if(hInstance != nullptr)
		{
			loggerInstanceCreated = true;
		}
	}
	return hInstance;
}

void CLogger::LogString(const char * str)
{
	if(this->serialCom != nullptr)
	{
		this->serialCom->write(str);
	}
}

void CLogger::LogStringLn(const char * str)
{
	if(this->serialCom != nullptr)
	{
		this->serialCom->write(str);
		this->serialCom->write("\r\n");
	}
}


// ********************************
ILogger * GetLoggerInstance()
{
	return reinterpret_cast<ILogger*>(CLogger::GetInstance());
}
