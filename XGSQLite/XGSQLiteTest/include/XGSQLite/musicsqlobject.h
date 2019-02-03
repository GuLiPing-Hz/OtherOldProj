#pragma once
#ifndef _H_MUSICSQLOBJECT
#define _H_MUSICSQLOBJECT


#include <list>

using namespace std;
namespace XGSQLITE{

#ifdef XGSQLITE_EXPORTS
#	define XGSQLITE_API		__declspec( dllexport )
#else
#	define XGSQLITE_API		__declspec( dllimport )
#endif

#define XGSQLITE_SUCCESSD	0
#define XGSQLITE_FAILED			-1

//歌手类型
enum SINGER_TYPE
{
	SINGER_TYPE_ALL=-1,					//所有歌星
	SINGER_TYPE_HKTW_MAN=0,		//港台男歌手
	SINGER_TYPE_HKTW_WOMAN,		//港台女歌手
	SINGER_TYPE_CHN_MAN,				//大陆男歌手
	SINGER_TYPE_CHN_WOMAN,		//大陆女歌手
	SINGER_TYPE_CHN_GROUP,			//大陆组合
	SINGER_TYPE_FOREIGN_STAR,		//外国歌手
	SINGER_TYPE_FOREIGN_GROUP,	//外国组合
};

//音乐类型
enum MUSIC_TYPE
{
	MUSIC_TYPE_ALL=-1,
	MUSIC_TYPE_DISCO=0,		//DISCO歌曲
	MUSIC_TYPE_CHILD,			//儿童歌曲
	MUSIC_TYPE_LOVER,			//情侣对唱
	MUSIC_TYPE_RED,				//革命歌曲（红歌）
	MUSIC_TYPE_ROCK,			//摇滚歌曲
	MUSIC_TYPE_DANCE,		//舞曲
	MUSIC_TYPE_HAPPY,		//喜庆歌曲
	MUSIC_TYPE_DRAMA,		//戏曲
	MUSIC_TYPE_CHORUS,		//群星合唱
};
//歌曲语言类型
enum LANG_TYPE
{
	LANG_TYPE_ALL=-1,
	LANG_TYPE_CN=0,	//中文国语
	LANG_TYPE_CT,			//粤语
	LANG_TYPE_TN,		//闽南语
	LANG_TYPE_EN,		//英文
	LANG_TYPE_JP,			//日文
	LANG_TYPE_KR,			//韩文
};

/*****************************
*歌曲信息类
*****************************/
class XGSQLITE_API CMusicInfo
{
public:
	CMusicInfo(void);
	~CMusicInfo(void);

	void ClearInfo();
public:
	int m_music_id;							//歌曲id
	char m_music_name[100];			//歌曲名
	char m_music_filename[200];	//歌曲文件名
	char m_music_singer[50];			//歌手名
	int m_music_type;						//歌曲类型
	char m_music_namepy[50];		//歌曲名拼音缩写
	char m_music_singerpy[50];		//歌手名拼音缩写
	int m_music_singertype;		//歌手类别
	int m_music_langtype;			//歌曲语言类别
	int m_music_namecount;		//歌曲名字数
	int m_music_voicevalue;		//歌曲音量值
	int m_music_lrbc;					//歌曲音轨
	int m_music_musicpos;			//曲谱偏移位置
	char m_music_timemark[100];	//简版歌曲时间戳
	int m_music_lighteffect;			//歌曲灯光效果
	int m_music_subtype;				//推荐歌曲
	int m_music_hot;						//热门歌曲
	int m_music_new;						//新歌推荐
};

/*****************************
*歌手信息类
*****************************/
class XGSQLITE_API CSingerInfo
{
public :
	CSingerInfo()
	{
		memset(m_singername,0,sizeof(m_singername));
		m_songnum=0;
	};
	~CSingerInfo(){};
public :
	char	m_singername[100];		//歌手名
	int	m_songnum;					//歌手所有的歌曲数
} ;


/*****************************
*SQLite数据库操作类
*****************************/
class XGSQLITE_API CMusicSQLObject 
{
public:
	/*
	**参数说明：dbname 本地数据库文件名
	**					trace	    是否在控制台上输出跟踪信息，默认为不输出
	*/
	CMusicSQLObject(std::string dbname,bool trace=false);
	~CMusicSQLObject(void);

	/*
	**创建本地数据库，如果数据库已存在，则删除后重建
	**返回值：成功：XGSQLITE_SUCCESSD
	**			 ：失败：XGSQLITE_FAILED （非法数据库名）
	*/
	int CreateDataBase();
public:
	/*
	**按页搜索结果
	**参数说明：	pageindex	[in] 页索引，取第几页数据，从1开始
	**					pagesize		[in] 页大小 ，每页显示几条数据
	**					list				[out] 歌曲信息列表，取完歌曲信息后务必释放CMusicInfo对象
	**												否则会引起内存泄漏
	**返回值： 成功：搜索结果的总页数  
	**				失败：XGSQLITE_FAILED
	*/
	int GetMusicInfo(int pageindex,int pagesize,list<CMusicInfo*> &_list);

	/*
	**将歌曲信息保存到SQLite本地数据库
	**参数说明：	pMusicInfo	[in] 歌曲信息对象
	**返回值： 成功：XGSQLITE_SUCCESSD  
	**				失败：XGSQLITE_FAILED
	*/
	int PutMusicInfo(list<CMusicInfo*> &list);
	/*
	**清空搜索条件
	**参数说明：无
	**返回值： 无  
	*/
	void ClearSearchCondition();
	//设置搜索条件--歌曲名（搜索歌手时无效）
	void SetSearchConditionName(char* value);
	//设置搜索条件--歌曲名拼音缩写（搜索歌手时无效）
	void SetSearchConditionNamePY(char* value);
	//设置搜索条件--歌手名
	void SetSearchConditionSinger(char* value);
	//设置搜索条件--歌手名拼音缩写（搜索歌曲时无效）
	void SetSearchConditionSingerPY(char* value);
	//设置搜索条件--歌曲类型（搜索歌手时无效）
	void SetSearchConditionType(int value);
	//设置搜索条件--歌手类型（搜索歌曲时无效）
	void SetSearchConditionSingerType(int value);
	//设置搜索条件--歌曲名字数（搜索歌手时无效）
	void SetSearchConditionNameCount(int value);
	//设置搜索条件--歌曲语言类型（搜索歌手时无效）
	void SetSearchConditionLangType(int value);
	//设置搜索条件--推荐歌曲（搜索歌手时无效）
	void SetSearchConditionSubType(int value=1);
	//设置搜索条件--热门歌曲（搜索歌手时无效）
	void SetSearchConditionHot(int value=1);
	//设置搜索条件--新歌推荐（搜索歌手时无效）
	void SetSearchConditionNew(int value=1);
	/*
	**返回搜索歌曲的总数
	**参数说明：	无
	**返回值： 成功：返回实际歌曲数  
	**				失败：XGSQLITE_FAILED
	*/
	int  GetMusicCount();
	/*
	**返回搜索歌手的总数
	**参数说明：	无
	**返回值： 成功：返回实际歌手数  
	**				失败：XGSQLITE_FAILED
	*/
	int GetSingerCount();
	/*
	**按页取得搜索的歌手信息
	**参数说明：	pageindex	[in] 页索引，取第几页数据，从1开始
	**					pagesize		[in] 页大小 ，每页显示几条数据
	**					list				[out] 歌手信息列表，取完歌手信息后务必释放CSingerInfo对象
	**												否则会引起内存泄漏
	**返回值： 成功：搜索结果的总页数  
	**				失败：XGSQLITE_FAILED
	*/
	int GetSingerList(int pageindex,int pagesize,list<CSingerInfo*> &_list);
private:

	void CreateSearchMusicConditionStr(std::string& _condition);

	void CreateSearchSingerConditionStr(std::string& _condition);
private:
	char		m_musicname[100];			//歌曲名
	char		m_musicsinger[50];			//歌手名
	int		m_musictype;					//歌曲类型
	char		m_musicnamepy[50];		//歌曲名拼音缩写
	char		m_musicsingerpy[50];		//歌手名拼音缩写
	int		m_musicsingertype;			//歌手类别
	int		m_musiclangtype;				//歌曲语言类别
	int		m_musicnamecount;			//歌曲名字数
	int		m_musicsubtype;				//推荐歌曲
	int		m_musichot;						//热门歌曲
	int		m_musicnew;					//新歌推荐

	//sqlite3xx::connection*   m_sqliteconn;	//sqlite连接对象

	std::string m_dbname;		//本地数据库名

	bool m_trace;					//是否开启跟踪日志
	//result* m_presult;				//SQLite执行结果
};
};
#endif