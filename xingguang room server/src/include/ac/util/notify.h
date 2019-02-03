#ifndef AC_UTIL_NOTIFY_H_
#define AC_UTIL_NOTIFY_H_

#include <pthread.h>
#include <ac/util/handle.h>

namespace ac {

struct Notification
{
	virtual ~Notification() {}
	virtual void notify() = 0;
};

class SocketNotification : public Notification
{
	//Handle handle;
	int m_fd;
public:
	void SetFD(int fd){m_fd = fd;}
	int GetFD(){ return m_fd; }
	/*void SetHandle(Handle handle)
	{ this->handle = handle; }

	Handle GetHandle() const
	{ return this->handle; }*/

	virtual void notify();
};

class SignalNotification : public Notification
{
public:
	SignalNotification(int sig) : m_sig(sig){}
	virtual void notify();
private:
	int m_sig;
};

} // namespace ac

#endif // AC_UTIL_NOTIFY_H_

