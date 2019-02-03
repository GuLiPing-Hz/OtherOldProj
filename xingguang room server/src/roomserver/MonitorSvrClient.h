#ifndef _MONITORSVRCLIENT_H_
#define _MONITORSVRCLIENT_H_
#pragma once
#include "network/clientsocket.h"
#include "netproxy.h"
using namespace ac;

class MonitorSvrClient :public BackClientSocketBase
{
public:
	MonitorSvrClient(Reactor *pReactor,const char* ip,int port) : BackClientSocketBase(pReactor,ip,port){};
	~MonitorSvrClient(void){};
	void OnConnect();
	
};
#endif

