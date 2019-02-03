#include <stdio.h>
#include <signal.h>
#include <string>
#include "ac/log/ostreamlogger.h"
#include "ac/log/log.h"
#include "ac/log/datefilelogger.h"
#include "network/netapp.h"
#include "network/dataprocessor.h"
#include "network/datadecoder.h"
#include "serverconf.h"
#include "netproxy.h"
//#include "ac/bdb/bdbenv.h"
//#include "ac/bdb/bdb_uk.h"

using namespace ac;
using namespace std;
void prog_exit(int signo);

NetProxy *g_pNetProxy;
char g_configfilename[128];
char g_rightpath[128]={0};

int main(int argc,char *argv[])
{
	memset(g_configfilename,0,sizeof(g_configfilename));
	int ch;
    while((ch = getopt(argc,argv,"f:r:"))!= -1)
	{
		switch(ch)
		{
			case 'f':
				{
					//printf("option f:'%s'\n",optarg);
					strncpy(g_configfilename,optarg,sizeof(g_configfilename));
				}
				break;
            		case 'r':
		     		{
		                   strncpy(g_rightpath,optarg,sizeof(g_rightpath));
		             }
		             break;
		}
	}
	if(strlen(g_configfilename) == 0)
	{
		strcpy(g_configfilename,"../etc/roomserver7100.xml");
	}
	printf("configfilename = %s\n",g_configfilename);

    if(strlen(g_rightpath) == 0)
    {
        strcpy(g_rightpath,"../etc/rightconfig.xml");
    }
    printf("rightpath = %s\n",g_rightpath);
	
#ifdef LINUX_DAEMON
	int pid;
	signal(SIGCHLD, SIG_IGN);
	pid = fork();
	if(pid < 0) {
		perror("fork");
		exit(-1);
	}
	else if(pid > 0)
		exit(0);
	setsid();
#endif
        signal(SIGCHLD, SIG_DFL);
        signal(SIGPIPE, SIG_IGN);
        signal(SIGINT, prog_exit);
        signal(SIGKILL, prog_exit);
        signal(SIGTERM, prog_exit);
	
	OStreamLogger *stdoutlog = new OStreamLogger(std::cout);
	AC_SET_DEFAULT_LOGGER(stdoutlog);//&OStreamLogger::StdoutLogger);
	//AC_INFO("allkind size = %d",sizeof(ReserveData_Cache));
	ServerConf sconf;
	if(sconf.ReadCfg(g_configfilename) != 0)
	{
		AC_ERROR("read servercfg error");
		return 0;
	}
	
	
	DateFileLogger *runlog = new DateFileLogger(string(sconf.m_Logdir) + "runlog");
	DateFileLogger *errlog = new DateFileLogger(string(sconf.m_Logdir) + "syslog");

	AC_SET_DEFAULT_LOGGER(errlog);
	AC_SET_LOGGER(ac::LP_USER1,runlog);

	g_pNetProxy = new NetProxy(sconf.m_sIP,sconf.m_Listenport,sconf.m_Maxclientcount,PROTOCOLTYPE_BINARY,HEADER_LEN_2,sconf.m_Udpport,&sconf.m_HallsvrCfg,&sconf.m_DbagentCfg,&sconf.m_DbagentCfgGift,&sconf.m_DbagentCfgRight,&sconf.m_DbMonitor,&sconf.m_LogSvrCfg,/*&sconf.m_FactorySvrCfg,*/sconf.m_ServerKey,sconf.m_Runscan,sconf.m_Numberonescan,sconf.m_Readtimeout,sconf.m_Conntimeout);
	printf("\n***************\nnet app start\n***************\n");
	AC_INFO("***************net app start***************");	

	if(g_pNetProxy->Start() != 0)
	{
		g_pNetProxy->Stop();
		printf("start net proxy error\n");
		AC_ERROR("start net proxy error");
	}
	delete g_pNetProxy;

	return 0;
}

void prog_exit(int signo)
{
        AC_INFO("OK: Receive a signal %d", signo);
  	g_pNetProxy->Stop();
}

