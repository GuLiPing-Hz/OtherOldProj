#include "netproxy.h"
#include "MonitorSvrClient.h"
#include "roomtype.h"


void MonitorSvrClient::OnConnect()
{
	g_pNetProxy->ReportStatusToMonitor();
}

