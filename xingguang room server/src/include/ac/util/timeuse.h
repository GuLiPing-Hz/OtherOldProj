#ifndef AC_TIMEUSE_H_
#define AC_TIMEUSE_H_

namespace ac
{
	inline long time_use(timeval *timebegin,timeval *timeend)
	{
		long timeuse = 1000000*(timeend->tv_sec-timebegin->tv_sec)+ timeend->tv_usec-timebegin->tv_usec;
		return timeuse;
	}
}

#endif
