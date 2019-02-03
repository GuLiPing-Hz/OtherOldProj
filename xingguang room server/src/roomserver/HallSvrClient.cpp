#include "netproxy.h"
#include "HallSvrClient.h"
#include "roomtype.h"


void HallSvrClient::OnConnect()
{
	AC_DEBUG("HallSvrClient::OnConnect: Connect to Lobby");
	g_pNetProxy->ReportMyStatus(HALL_CMD_ROOMLOGIN);
}
