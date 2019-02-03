// Opengl.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "glputil.h"
#include <math.h>
#include <locale.h>

//#include "CWavePlayer.h"

void Str2Wstr(IN const std::string str,OUT std::wstring &wstr)
{
// 	std::wstring tmp_ws(str.begin(),str.end());
// 	wstr = tmp_ws;
	std::string curLocale = setlocale(LC_ALL, NULL); // curLocale = "C";
	setlocale(LC_ALL, "chs"); 
	const char* _Source = str.c_str();
	size_t _Dsize = str.size() + 1;
	wchar_t *_Dest = new wchar_t[_Dsize];
	wmemset(_Dest, 0, _Dsize);
	mbstowcs(_Dest,_Source,_Dsize);
	wstr = _Dest;
	delete []_Dest;
	setlocale(LC_ALL, curLocale.c_str());

}
void Wstr2Str(IN const std::wstring wstr,OUT std::string &str)
{
// 	std::string tmp_s(wstr.begin(),wstr.end());
// 	str = tmp_s;
	std::string curLocale = setlocale(LC_ALL, NULL); // curLocale = "C";
	setlocale(LC_ALL, "chs"); 
	const wchar_t* _Source = wstr.c_str();
	size_t _Dsize = 2 * wstr.size() + 1;
	char *_Dest = new char[_Dsize];
	memset(_Dest,0,_Dsize);
	wcstombs(_Dest,_Source,_Dsize);
	str = _Dest;
	delete []_Dest;
	setlocale(LC_ALL, curLocale.c_str());
}


void writeLog(const wchar_t *msg)
{
	if (msg == NULL)
	{
		return;
	}
	char buf[1024] = {0};
	WideCharToMultiByte(CP_ACP,0,msg,-1,buf,1023,NULL,NULL);

	SYSTEMTIME st;
	GetLocalTime(&st);

	FILE * fp = fopen("F:\\record.log","a");
	if (fp == NULL)
	{
		return;
	}
	fprintf(fp,"[%02d:%02d:%02d.%05d],%s\n",st.wHour,st.wMinute,st.wSecond,st.wMilliseconds,buf);
	fclose(fp);
}

void writeLog(const char *msg,...)
{
	if (msg == NULL)
	{
		return;
	}
	SYSTEMTIME st;
	GetLocalTime(&st);

	FILE * fp = fopen("F:\\record.log","a");
	if (fp == NULL)
	{
		return;
	}
	fprintf(fp,"[%02d:%02d:%02d.%05d]:",st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
	va_list arg_ptr;
	//const char* str = msg;
	va_start(arg_ptr, msg);
	vfprintf(fp,msg,arg_ptr);
	va_end(arg_ptr);
	fprintf(fp,"\n");
	fclose(fp);
}

//////////////////////////////////////////////////////////////////////////

void  Ini_SetInt(const char *szIniFile,const char *section, const char *name, int value)
{
	char buf[256];

	if(szIniFile) {
		sprintf(buf,"%d",value);
		WritePrivateProfileString(section, name, buf, szIniFile);
	}
}


int  Ini_GetInt(const char *szIniFile,const char *section, const char *name, int def_val)
{
	char buf[256];

	if(szIniFile) {
		if(GetPrivateProfileString(section, name, "", buf, sizeof(buf), szIniFile))
		{ return atoi(buf); }
		else { return def_val; }
	}
	return def_val;
}


void  Ini_SetFloat(const char *szIniFile,const char *section, const char *name, float value)
{
	char buf[256];

	if(szIniFile) {
		sprintf(buf,"%f",value);
		WritePrivateProfileString(section, name, buf, szIniFile);
	}
}


float  Ini_GetFloat(const char *szIniFile,const char *section, const char *name, float def_val)
{
	char buf[256];

	if(szIniFile) {
		if(GetPrivateProfileString(section, name, "", buf, sizeof(buf), szIniFile))
		{ return (float)atof(buf); }
		else { return def_val; }
	}
	return def_val;
}


void  Ini_SetString(const char *szIniFile,const char *section, const char *name, const char *value)
{
	if(szIniFile) WritePrivateProfileString(section, name, value, szIniFile);
}


char*  Ini_GetString(const char *szIniFile,const char *section, const char *name, const char *def_val)
{
	static char szIniString[256] = {0};
	ZeroMemory(szIniString,256);
	if(szIniFile) GetPrivateProfileString(section, name, def_val, szIniString, sizeof(szIniString), szIniFile);
	else strcpy(szIniString, def_val);
	return szIniString;
}







