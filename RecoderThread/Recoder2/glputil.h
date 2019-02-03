#ifndef OPENGL__H__
#define  OPENGL__H__

#include <string>

//≈‰÷√Œƒº˛
void			Ini_SetInt(const char *szIniFile,const char *section, const char *name, int value);
int 			Ini_GetInt(const char *szIniFile,const char *section, const char *name, int def_val);
void			Ini_SetFloat(const char *szIniFile,const char *section, const char *name, float value);
float			Ini_GetFloat(const char *szIniFile,const char *section, const char *name, float def_val);
void			Ini_SetString(const char *szIniFile,const char *section, const char *name, const char *value);
char*		Ini_GetString(const char *szIniFile,const char *section, const char *name, const char *def_val);

void Str2Wstr(IN const std::string str,OUT std::wstring &wstr);
void Wstr2Str(IN const std::wstring wstr,OUT std::string &str);

void writeLog(const char *msg,...);
void writeLog(const wchar_t *msg);

#endif //OPENGL__H__
