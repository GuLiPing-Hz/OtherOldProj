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
};

//歌曲语言类型
enum LANG_TYPE
{
	LANG_TYPE_ALL=-1,
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
	char m_music_name[300];			//歌曲名
	char m_music_filename[500];	//歌曲文件名
	char m_music_singer[300];			//歌手名
	char m_music_typeinfo[20];		//歌曲类型
	char m_music_namepy[50];		//歌曲名拼音缩写
	char m_music_singerpy[50];		//歌手名拼音缩写
	int m_music_singertype;		//歌手类别
	int m_music_langtype;			//歌曲语言类别
	int m_music_namecount;		//歌曲名字数
	char m_music_topinfo[20];			//排行榜信息
	int m_music_voicevalue;		//歌曲音量值
	int m_music_lrbc;					//歌曲音轨
	char m_music_musicpos[20];			//曲谱高度
	char m_music_timemark[100];	//简版歌曲时间戳
	int m_music_lighteffect;			//歌曲灯光效果
	int m_music_recommend;				//推荐歌曲
	int m_music_pop;						//热门歌曲
	int m_music_new;						//新曲
	int m_music_isScore;					//是否带评分
	int m_music_isPlay;				//默认播放歌曲
	int m_music_isChorus;			//1是合唱，0不是合唱
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
		m_singertype=0;
	};
	~CSingerInfo(){};
public :
	char	m_singername[100];		//歌手名
	int	m_songnum;					//歌手所有的歌曲数
	int	m_singertype;				//歌手类型
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
	void SetSearchConditionName(const char* value);
	//设置搜索条件--歌曲名拼音缩写（搜索歌手时无效）
	void SetSearchConditionNamePY(const char* value);
	//设置搜索条件--歌手名
	void SetSearchConditionSinger(const char* value);
	//设置搜索条件--歌手名拼音缩写（搜索歌曲时无效）
	void SetSearchConditionSingerPY(const char* value);
	//设置搜索条件--歌曲类型（搜索歌手时无效）
	void SetSearchConditionSongType(int value);
	//设置搜索条件--歌手类型
	void SetSearchConditionSingerType(int value);
	//设置搜索条件--歌曲名字数（搜索歌手时无效）
	void SetSearchConditionNameCount(int value);
	//设置搜索条件--歌曲语言类型（搜索歌手时无效）
	void SetSearchConditionLangType(int value);
	//设置搜索条件--是否带评分（搜索歌手时无效）
	void SetSearchConditionIsScore(int value=1);
	//设置搜索条件--曲种、榜单搜索，value取值见配置表
	void SetSearchConditionTopType(int value);
	//设置搜索条件--新曲
	void SetSearchConditionNew(int value=1);
	//设置搜索条件--默认播放歌曲
	void SetSearchConditionIsPlay(int value=1);
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
	/*
	**按页取得搜索的歌手信息
	**参数说明：	_musicid		[in]    歌曲ID
	**					_musicinfo	[out]  返回歌曲数据
	**返回值：	成功：XGSQLITE_SUCCESS
	**				失败：XGSQLITE_FAILED
	*/
	int GetMusicInfoById(int _musicid,CMusicInfo  &_musicinfo);

	/*
	**根据歌曲ID取得对应的歌手信息
	**参数说明：	_musicid		[in]    歌曲ID
	**					_singerinfo	[out]  返回歌手数据
	**返回值：	成功：XGSQLITE_SUCCESS
	**				失败：XGSQLITE_FAILED
	*/
	int GetSingerByMusicID(int _musicid,CSingerInfo &_singerinfo);

	//int GetMusicInfoBySingerCount();
	/*
	**根据歌手名精确查找歌曲信息
	**参数说明：	_musicid		[in]    歌曲ID
	**					_singerinfo	[out]  返回歌手数据
	**返回值：	成功：XGSQLITE_SUCCESS
	**				失败：XGSQLITE_FAILED
	*/
	//int GetMusicInfoBySinger(int _pageindex,int _pagesize,list<CMusicInfo*> &_list );

private:

	void CreateSearchMusicConditionStr(std::string& _condition);

	//_ByCount 需要产生取count的条件时设置为1
	void CreateSearchSingerConditionStr(std::string& _condition,int _ByCount=0);
	
	//搜索结果保存成视图
	int CreateSearchView(std::string _searcher);

private:
	char		m_musicname[100];			//歌曲名
	char		m_musicsinger[50];			//歌手名
	char		m_musictypeinfo[20];		//歌曲信息类型
	char		m_musicnamepy[50];		//歌曲名拼音缩写
	char		m_musicsingerpy[50];		//歌手名拼音缩写
	int		m_musicsingertype;			//歌手类别
	int		m_musiclangtype;				//歌曲语言类别
	int		m_musicnamecount;			//歌曲名字数
	//int		m_musicsubtype;				//推荐歌曲
	//int		m_musichot;						//热门歌曲
	int		m_musicnew;					//新曲
	int		m_musicisscore;				//是否带评分
	char		m_musictopinfo[20];			//歌曲排行榜信息
	int		m_musicplay;					//默认播放歌曲

	//sqlite3xx::connection*   m_sqliteconn;	//sqlite连接对象

	std::string m_dbname;		//本地数据库名

	bool m_trace;					//是否开启跟踪日志
	//result* m_presult;				//SQLite执行结果

	bool m_CreateView;
};
};
#endif