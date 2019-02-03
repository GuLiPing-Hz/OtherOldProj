// Opengl.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "Opengl.h"
#include <math.h>
#include <locale.h>
#include "GLListDef.h"



CGPoint CGPointMake(float x,float y,float z)
{
	CGPoint point;
	point.x = x;
	point.y = y;
	point.z = z;
	return point;
}

CGSize CGSizeMake(int x,int y)
{
	CGSize cgSize;
	cgSize.width = x;
	cgSize.height = y;
	return cgSize;
}

CGRect CGRectMake(float x,float y,int w,int h)
{
	CGRect rect;
	rect.origin.x = x;
	rect.origin.y = y;
	rect.size.width = w;
	rect.size.height = h;
	return rect;
}

CGRect CGRectMake(const CGPoint& point,const CGSize& size)
{
	CGRect rect;
	rect.origin = point;
	rect.size = size;
	return rect;
}

CGRect CGRectIntersection(const CGRect& a,const CGRect& b)
{
	float Lmax = max(a.origin.x,b.origin.x);
	float Rmin = min(a.origin.x+a.size.width,b.origin.x+b.size.width);
	float Tmax = max(a.origin.y,b.origin.y);
	float Bmin = min(a.origin.y+a.size.height,b.origin.y+b.size.height);

	if (Lmax>=Rmin||Tmax>=Bmin)
	{
		return CGRectMake(0.0f,0.0f,0,0);
	}
	int w = (int)(Rmin-Lmax);
	int h = (int)(Bmin-Tmax);
	return CGRectMake(Lmax,Tmax,w,h);
}

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

void GetOnlyDifferentFont(std::wstring &font_all)
{
	std::wstring tmp_font = font_all;
	MAPWCHARINT tmp_map;
	font_all.clear();
	std::wstring::iterator i;
	for (i=tmp_font.begin();i!=tmp_font.end();i++)
	{
		wchar_t ch = *i;
		MAPWCHARINT::iterator i=tmp_map.find(ch);
		if (i == tmp_map.end() && (ch != L' '))
		{
			tmp_map.insert(std::pair<wchar_t,int>(ch,1));
			font_all.push_back(ch);
		}
	}
}

bool CalculateDB(IN const VECTFLOAT buffer,OUT double& db)
{
	static float m_fpeak = 0.0f;
	VECTFLOAT::const_iterator i;
	for (i=buffer.begin();i!=buffer.end();i++)
	{
		float s = *i;
		float p = s*s;
		if (p>m_fpeak)
		{
			m_fpeak = p;
		}
		else
		{
			m_fpeak *= 0.999f;
		}
	}

	db = 10.0*log10(m_fpeak);

	return true;
}

void initLogFile(const char* logFile)
{
	if (logFile)
	{
		strcpy(LOGPATH,logFile);
	}
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

	FILE * fp = fopen(LOGPATH,"a");
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

	FILE * fp = fopen(LOGPATH,"a");
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
	char buf[260]={0};

	if(szIniFile) {
		sprintf(buf,"%d",value);
		WritePrivateProfileString(section, name, buf, szIniFile);
	}
}


int  Ini_GetInt(const char *szIniFile,const char *section, const char *name, int def_val)
{
	char buf[260]={0};

	if(szIniFile) {
		if(GetPrivateProfileString(section, name, "", buf, sizeof(buf), szIniFile))
		{ return atoi(buf); }
		else { return def_val; }
	}
	return def_val;
}


void  Ini_SetFloat(const char *szIniFile,const char *section, const char *name, float value)
{
	char buf[260]={0};

	if(szIniFile) {
		sprintf(buf,"%f",value);
		WritePrivateProfileString(section, name, buf, szIniFile);
	}
}


float  Ini_GetFloat(const char *szIniFile,const char *section, const char *name, float def_val)
{
	char buf[260]={0};

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
	static char szIniString[260] = {0};
	if(szIniFile) GetPrivateProfileString(section, name, def_val, szIniString, sizeof(szIniString), szIniFile);
	else strcpy(szIniString, def_val);
	return szIniString;
}

wchar_t* Ini_GetStringW(const char *szIniFile,const char *section, const char *name, const char *def_val)
{
	static wchar_t szIniStringW[260] = {0};
	std::string str = Ini_GetString(szIniFile,section,name,def_val);
	std::wstring wstr;
	Str2Wstr(str,wstr);
	wcscpy_s(szIniStringW,259,wstr.c_str());
	return szIniStringW;
}







