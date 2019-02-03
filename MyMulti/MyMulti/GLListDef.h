#pragma  once

#include <vector>
#include <list>
#include <map>
#include <wchar.h>


#include "KKType.h"
typedef std::map<eVideoType,bool>	MAPVIDEOTYPESWITCH;
typedef std::list<wchar_t> LISTWCHAR;
typedef std::map<wchar_t,int> MAPWCHARINT;
typedef std::list<GLRealtimeGrade>	LISTGLREALTIMEGRADE;
typedef std::list<GLSentenceGrade>	LISTGLSENTENCEGRADE;
typedef std::list<_tGuiLyricInfo>					LISTGUILYRICVECT;
typedef std::list<_tGuiWaveInfo>				LISTGUIPITCHVECT;
typedef std::vector<_tGuiParagraphInfo>	VECTPARAGRAPHVECT;
typedef std::list<float>										LISTSENTENCELINEVECT;
typedef std::list<int>										LISTINT;
typedef std::list<float>									LISTFLOAT;
typedef std::list<bool>									LISTBOOL;
typedef std::vector<ulong>							VECTULONG;
typedef std::vector<float>							VECTFLOAT;

typedef std::list<RealTimePoint>					LISTREALTIMEPOINT;
typedef std::vector<LINEVERTEX>				VECTLINEVERTEX;

typedef std::vector<_tGuiLyricInfo>				VECTGUILYRICVECT;
typedef std::vector<_tGuiWaveInfo>			VECTGUIPITCHVECT;

typedef std::vector<int>			VECTORINT;
class Image;
typedef std::vector<Image*>	VECTORIMAGE;
typedef std::map<std::string,DWORD_PTR> MAPSTRINGID;
