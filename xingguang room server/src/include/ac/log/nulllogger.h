#ifndef AC_LOG_NULL_LOGGER_H_
#define AC_LOG_NULL_LOGGER_H_

#include <string>
#include "ac/log/logger.h"

namespace ac
{
class NullLogger : public Logger
{
public:
	virtual int Log(LogPriority, const std::string &)
		{ return 0; }

public:
	static NullLogger * Instance();
};

}

#endif // AC_LOG_NULL_LOGGER_H_
