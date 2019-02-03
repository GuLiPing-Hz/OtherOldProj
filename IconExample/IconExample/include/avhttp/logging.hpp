//
// logging.hpp
// ~~~~~~~~~~~
//
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef AVHTTP_LOGGING_HPP
#define AVHTTP_LOGGING_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <iostream>
#include <string>
#include <fstream>

#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/noncopyable.hpp>
#include <boost/filesystem.hpp>

namespace avhttp {

namespace fs = boost::filesystem;

///�ڲ�ʹ�õļ�����־��.
// ʹ��˵��:
//	�ڳ������(��:main)�������� AVHTTP_INIT_LOGGER ��, ������������, ��һ������ָ������־�ļ�����
//	��·��, �ڶ�������ָ������־�ļ�������ļ���, ��ϸ��AVHTTP_INIT_LOGGER.
//	Ȼ��Ϳ���ʹ��AVHTTP_LOG_DBG/AVHTTP_LOG_INFO/AVHTTP_LOG_WARN/AVHTTP_LOG_ERR�⼸�����������־��Ϣ.
// @begin example
//  #include "avhttp.hpp" // avhttp.hpp �Ѿ����� logging.hpp, Ҳ�ɵ�������logging.hpp.
//  int main()
//  {
//     AVHTTP_INIT_LOGGER(".", "example.log");	// �ڵ�ǰĿ¼������־�ļ�Ϊexample.log.
//     // Ҳ�� AVHTTP_INIT_LOGGER("", ""); �մ�Ϊ��������ֹ�����־���ļ�, �����������̨.
//     LOG_DEBUG("Initialized.");
//     std::string result = do_something();
//     LOG_DEBUG("do_something return : " << result);	// ���do_something���ؽ������־.
//     ...
//     AVHTTP_UNINIT_LOGGER();	// ж����־ģ��.
//  }
// @end example

class logger : public boost::noncopyable
{
public:
	logger(fs::path const& logpath, fs::path const& filename, bool append = true, bool inited = false)
	{
		try
		{
			m_inited = inited;
			if (!inited)
				return;
			if (filename.empty())
				return;
			if (!fs::exists(logpath)) fs::create_directories(logpath);
			m_file.open((logpath / filename).string().c_str(),
				std::ios_base::out | (append ? std::ios_base::app : std::ios_base::out));
			*this << "\n\n\n*** starting log ***\n\n\n";
		}
		catch (std::exception& e)
		{
			std::cerr << "failed to create log '" << filename.string() << "': " << e.what() << std::endl;
		}
	}
	~logger() {}

public:

	template <class T>
	logger& operator<<(T const& v)
	{
		if (!m_inited)
			return *this;

#ifndef AVHTTP_DISABLE_LOGGER_TO_CONSOLE
		std::cout << v;
		std::cout.flush();
#endif

		if (m_file.is_open())
		{
			m_file << v;
			m_file.flush();
		}

#if defined(WIN32) && defined(LOGGER_DBG_VIEW)
		std::ostringstream oss;
		oss << v;
		m_dbg_view += oss.str();
#endif
		return *this;
	}

	const std::string& dbg_view() const
	{
		return m_dbg_view;
	}

	void clear_dbg_view()
	{
		m_dbg_view = "";
	}

	bool inited() const
	{
		return m_inited;
	}

private:
	std::ofstream m_file;
	std::string m_dbg_view;
	bool m_inited;
};

inline char const* time_now_string()
{
	time_t t = std::time(0);
	tm* timeinfo = std::localtime(&t);
	static char str[200];
	std::strftime(str, 200, " %b %d %X ", timeinfo);
	return str;
}

namespace aux {

template<class Lock>
Lock& lock_single()
{
	static Lock lock_instance;
	return lock_instance;
}

template<class Log, class Logger_ptr>
Logger_ptr& logger_single(std::string path = ".",
	std::string filename = "avhttp.log", bool append = true, bool binit = false)
{
	static Logger_ptr logger_instance(boost::make_shared<Log>(path, filename, append, binit));
	if (logger_instance && !binit)
		return logger_instance;
	if (logger_instance)
	{
		if (logger_instance->inited())
			return logger_instance;
	}
	logger_instance = boost::make_shared<Log>(path, filename, append, binit);
	return logger_instance;
}

}

//////////////////////////////////////////////////////////////////////////
// ��־����ڲ�ʵ�ֶ���.

#define LOGGER_LOGS_ (*(avhttp::aux::logger_single<avhttp::logger, boost::shared_ptr<avhttp::logger> >()))

#ifdef LOGGER_THREAD_SAFE
#define LOGGER_LOCKS_() boost::mutex::scoped_lock lock(avhttp::aux::lock_single<boost::mutex>())
#else
#define LOGGER_LOCKS_() ((void)0)
#endif // LOGGER_THREAD_SAFE

#if defined(WIN32) && defined(LOGGER_DBG_VIEW)
#define LOGGER_DBG_VIEW_() do { OutputDebugStringA(LOGGER_LOGS_.dbg_view().c_str()); LOGGER_LOGS_.clear_dbg_view(); } while (0)
#else
#define LOGGER_DBG_VIEW_() ((void)0)
#endif // WIN32 && LOGGER_DEBUG_VIEW

//////////////////////////////////////////////////////////////////////////
// ��־����ⲿ�ӿڶ���.

///��ʼ����־�ӿ�.
// @param pathָ������־�ļ������·��.
// @param fileָ������־�ļ���.
// ��ע: ���fileΪ�մ�, �򲻲�����־�ļ�, ���������־����Ļ.
#define AVHTTP_INIT_LOGGER(path, file) do {\
	LOGGER_LOCKS_();\
	avhttp::aux::logger_single<avhttp::logger, boost::shared_ptr<avhttp::logger> >(path, file, true, true);\
} while (0)

///ж����־ģ��ӿ�.
#define AVHTTP_UNINIT_LOGGER() do {\
	LOGGER_LOCKS_();\
	avhttp::aux::logger_single<avhttp::logger, boost::shared_ptr<avhttp::logger> >().reset();\
} while (0)

#if defined(DEBUG) || defined(_DEBUG) || defined(AVHTTP_ENABLE_LOGGER)
#define AVHTTP_LOG_DBG(message) do { \
	LOGGER_LOCKS_(); \
	LOGGER_LOGS_ << avhttp::time_now_string() << "[DEBUG]: " << message << "\n"; \
	LOGGER_DBG_VIEW_(); \
} while (0)

#define AVHTTP_LOG_INFO(message) do { \
	LOGGER_LOCKS_(); \
	LOGGER_LOGS_ << avhttp::time_now_string() << "[INFO]: " << message << "\n"; \
	LOGGER_DBG_VIEW_(); \
} while (0)

#define AVHTTP_LOG_WARN(message) do { \
	LOGGER_LOCKS_(); \
	LOGGER_LOGS_ << avhttp::time_now_string() << "[WARNING]: " << message << "\n"; \
	LOGGER_DBG_VIEW_(); \
} while (0)

#define AVHTTP_LOG_ERR(message) do { \
	LOGGER_LOCKS_(); \
	LOGGER_LOGS_ << avhttp::time_now_string() << "[ERROR]: " << message << "\n"; \
	LOGGER_DBG_VIEW_(); \
} while (0)

#else
#define AVHTTP_LOG_DBG(message) ((void)0)
#define AVHTTP_LOG_INFO(message) ((void)0)
#define AVHTTP_LOG_WARN(message) ((void)0)
#define AVHTTP_LOG_ERR(message) ((void)0)
#endif

}

#endif // AVHTTP_LOGGING_HPP
