#ifndef AC_LOG_LOGGER_H_
#define AC_LOG_LOGGER_H_

#include <string>
#include "ac/log/logpriority.h"

namespace ac
{

class Logger
{
public:
	Logger(const std::string & sLogFormat=DEFAULT_FORMAT) : sLogFormat_(sLogFormat) {}
	virtual ~Logger() {}
	virtual int Log(LogPriority, const std::string &) = 0;

	inline const std::string & GetFormat() const { return sLogFormat_; }

public:
	static const std::string DEFAULT_FORMAT;

private:
	const std::string sLogFormat_;
};

}

#endif // AC_LOG_LOGGER_H_

