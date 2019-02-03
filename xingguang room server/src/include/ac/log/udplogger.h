#ifndef AC_LOG_UDPLOGGER_H_
#define AC_LOG_UDPLOGGER_H_

#include <string>
#include <arpa/inet.h>

#include "ac/log/logger.h"

namespace ac
{

class UdpLogger : public Logger
{
public:
	UdpLogger(const char * sIP, unsigned short uPort, const std::string & sLogFormat=Logger::DEFAULT_FORMAT);
	virtual int Log(LogPriority, const std::string & s);

private:
	struct sockaddr_in servaddr_;
};

}

#endif // AC_LOG_UDPLOGGER_H_

