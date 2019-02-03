#ifndef AC_NETWORK_SOCKET_H_
#define AC_NETWORK_SOCKET_H_

#include <ac/util/handle.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

namespace ac {

class Socket
{
private:
	Handle handle;

public:
	Socket(Handle handle_=Handle())
	: handle(handle_)
	{ }

	Handle GetHandle() const { return handle; }

	void SetHandle(Handle handle) { this->handle = handle; }

	int Create(int domain, int type, int protocol);

	int ConnectUnixSocket(const char* path, unsigned int usec_timeout);

	int Connect(const char* host, unsigned short port, unsigned int usec_timeout);

	int Connect(const sockaddr* addr, size_t addrlen, unsigned int usec_timeout);

	int BindUnixSocket(const char* path);

	int Bind(const char* host, unsigned short port);

	int Bind(const sockaddr* addr, size_t addrlen);

	int Listen(unsigned int backlog);

	int Accept(Socket& client);

	int SetSendTimeout(unsigned int usec_timeout);

	int SetRecvTimeout(unsigned int usec_timeout);

	int SetDelay(bool delay);

	int SetBlocking(bool blocking);

	int GetBlocking(bool& blocking);

	int Send(const char* p, size_t len);

	int Sendn(const char* p, size_t len);

	int Recv(char* p, size_t len);

	void Close();
};

} // namespace ac

#endif // AC_NETWORK_SOCKET_H_

