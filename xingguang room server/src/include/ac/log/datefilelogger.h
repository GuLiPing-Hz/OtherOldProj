#ifndef AC_LOG_DATE_FILE_LOGGER_H_
#define AC_LOG_DATE_FILE_LOGGER_H_

#include <sys/types.h>
#include <string>

#include "ac/log/logger.h"
#include "ac/util/mutex.h"

//! \namespace OssBase
namespace ac
{

/*! \class DateFileLogger
 * \brief 日期文件日志处理类
 */
class DateFileLogger : public Logger
{
public:

	/*! \brief 构造函数
	* \param[in] szLogFile 日志文件路径 ( 请尽量使用绝对路径 )
	*/
	DateFileLogger(const std::string & sLogFile, const std::string & sLogFormat=Logger::DEFAULT_FORMAT);
	

	/*! \brief 实现了Log方法，将sLogInfo写入时间文件
	* \param[in] iLogPriority 日志级别
	* \param[in] sLogMsg 日志信息
	* \return 0:成功 非0:失败
	*/
	virtual int Log(LogPriority iLogPriority, const std::string & sLogMsg);
	
protected:
	
	//! \brief 得到日志文件名
	//! \return 带日期的文件名
	std::string GetLogFileName();
	
private:

	//! \brief 日志路径
	std::string sLogFile_;

	ThreadMutex mutex_;

};

}

#endif // AC_LOG_DATE_FILE_LOGGER_H_

