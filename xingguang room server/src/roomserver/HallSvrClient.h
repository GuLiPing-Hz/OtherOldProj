#ifndef _HALLSVRCLIENT_H_
#define _HALLSVRCLIENT_H_
#pragma once
#include "network/clientsocket.h"
using namespace ac;

class HallSvrClient :public BackClientSocketBase
{
public:
	HallSvrClient(Reactor *pReactor,const char* ip,int port) : BackClientSocketBase(pReactor,ip,port){};
	~HallSvrClient(void){};
	void OnConnect();
	
};
#endif
