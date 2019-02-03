#ifndef AC_LOG_OSTREAM_LOGGER_H_
#define AC_LOG_OSTREAM_LOGGER_H_

#include <string>
#include <iostream>
#include "ac/util/mutex.h"
#include "ac/log/logger.h"

namespace ac
{

class OStreamLogger : public Logger
{
public:
	OStreamLogger(std::ostream & stream, const std::string & sLogFormat=Logger::DEFAULT_FORMAT);
	
	virtual int Log(LogPriority , const std::string & s);

public:
	//static OStreamLogger StdoutLogger;
	//static OStreamLogger StderrLogger;

private:
	std::ostream & stream_;
	ThreadMutex mutex_;
};

}

#endif // AC_LOG_OSTREAM_LOGGER_H_
