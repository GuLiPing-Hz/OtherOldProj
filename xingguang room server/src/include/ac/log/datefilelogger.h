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
 * \brief �����ļ���־������
 */
class DateFileLogger : public Logger
{
public:

	/*! \brief ���캯��
	* \param[in] szLogFile ��־�ļ�·�� ( �뾡��ʹ�þ���·�� )
	*/
	DateFileLogger(const std::string & sLogFile, const std::string & sLogFormat=Logger::DEFAULT_FORMAT);
	

	/*! \brief ʵ����Log��������sLogInfoд��ʱ���ļ�
	* \param[in] iLogPriority ��־����
	* \param[in] sLogMsg ��־��Ϣ
	* \return 0:�ɹ� ��0:ʧ��
	*/
	virtual int Log(LogPriority iLogPriority, const std::string & sLogMsg);
	
protected:
	
	//! \brief �õ���־�ļ���
	//! \return �����ڵ��ļ���
	std::string GetLogFileName();
	
private:

	//! \brief ��־·��
	std::string sLogFile_;

	ThreadMutex mutex_;

};

}

#endif // AC_LOG_DATE_FILE_LOGGER_H_

