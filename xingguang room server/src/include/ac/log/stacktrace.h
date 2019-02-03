#ifndef AC_LOG_STACKTRACE_H_
#define AC_LOG_STACKTRACE_H_

#include <sys/types.h>

namespace ac {

class StackTrace
{
	const char* file;
	size_t		line;
	const char* func;
public:
	StackTrace(const char* file, size_t line, const char* func);
	~StackTrace();

private:
	void Print(size_t indent, const char* action, const char* file, size_t line, const char* func);
};

#ifdef _AC_HAS_TRACE
# define _AC_STACK_TRACE() ac::StackTrace __##trace_##__LINE__(__FILE__, __LINE__, __PRETTY_FUNCTION__)
#else
# define _AC_STACK_TRACE() ((void)0)
#endif

} // namespace ac

#endif // AC_UTIL_TRACE_H_

