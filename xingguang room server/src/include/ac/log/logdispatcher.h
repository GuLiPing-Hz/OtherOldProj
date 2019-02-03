#ifndef AC_LOG_LOG_DISPATCHER_H_
#define AC_LOG_LOG_DISPATCHER_H_
 
#include <map>
#include <string>

#include "ac/util/rw_mutex.h"
#include "ac/util/non_copyable.h"
#include "ac/log/logpriority.h"
#include "ac/log/logger.h"


namespace ac
{

class LogDispatcher : public NonCopyable
{
public:
	static LogDispatcher * Instance();

public:
	
	void SetDefaultLogger(Logger * pDefaultLogger);

	void SetLogger(LogPriority iPriority, Logger * pNewLogger);

	void SetLogger(LogPriorities iPriorities, Logger * pNewLogger);
	
	Logger * GetLogger(LogPriority iPriority);

	inline int Log(LogPriority iPriority, const std::string & s)
	{
		return GetLogger(iPriority)->Log(iPriority, s);
	}

protected:
	
	LogDispatcher();

	void AtomicSetLogger(LogPriority iPriority, Logger * pNewLogger);

private:

	typedef std::map<LogPriority, Logger *> LoggerMap;

	LoggerMap mLoggerMap_;

	Logger * pDefaultLogger_;

	ThreadRWMutex mutex_;
};

}

#endif // AC_LOG_LOG_DISPATCHER_H_

