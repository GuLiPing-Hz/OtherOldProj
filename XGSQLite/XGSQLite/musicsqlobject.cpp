#include "StdAfx.h"
#include "MusicSQLObject.h"
#include "sqlite3xx/sqlite3xx" 

using namespace sqlite3xx;
using namespace XGSQLITE;
/*
**歌曲数据类
*/
CMusicInfo::CMusicInfo()
{
	ClearInfo();
}

CMusicInfo::~CMusicInfo()
{
	ClearInfo();
}

void CMusicInfo::ClearInfo()
{
	m_music_id=0;	
	m_music_singertype=0;		
	m_music_langtype=0;			
	m_music_namecount=0;		
	m_music_voicevalue=0;		
	m_music_lrbc=0;							
	m_music_lighteffect=0;	
	m_music_isScore=0;
	m_music_isChorus = 0;
	m_music_new=0;
	m_music_recommend=0;
	m_music_pop=0;
	m_music_isPlay=0;

	memset(m_music_name,0,sizeof(m_music_name));
	memset(m_music_filename,0,sizeof(m_music_filename));
	memset(m_music_singer,0,sizeof(m_music_singer));
	memset(m_music_namepy,0,sizeof(m_music_namepy));
	memset(m_music_singerpy,0,sizeof(m_music_singerpy));
	memset(m_music_timemark,0,sizeof(m_music_timemark));
	memset(m_music_typeinfo,0,sizeof(m_music_typeinfo));
	memset(m_music_topinfo,0,sizeof(m_music_topinfo));
	memset(m_music_musicpos,0,sizeof(m_music_musicpos));
}

/*
**歌曲数据操作类
*/
CMusicSQLObject::CMusicSQLObject(const std::string dbname,bool trace)
:m_dbname(dbname)
,m_trace(trace)
{
	ClearSearchCondition();
}

CMusicSQLObject::~CMusicSQLObject(void)
{}

int CMusicSQLObject::CreateDataBase()
{
	if(m_dbname.empty())
		return XGSQLITE_FAILED;

	_unlink(m_dbname.c_str());	//删除已经存在的库文件

	//创建歌曲库,设置大小写不敏感
	std::string presql="create table songinfo(\
	music_id integer ,\
	music_name text COLLATE NOCASE,\
	music_filename text ,\
	music_singer text COLLATE NOCASE,\
	music_typeinfo text,\
	music_namepy text COLLATE NOCASE,\
	music_singerpy text COLLATE NOCASE,\
	music_singertype integer,\
	music_langtype integer,\
	music_namecount integer,\
	music_voice integer,\
	music_lrbc integer,\
	music_musicpos text,\
	music_timemark text,\
	music_lighteffect integer,\
	music_topinfo text,\
	music_isScore integer,\
	music_new integer,\
	music_recommend integer,\
	music_pop integer ,\
	music_play integer,\
	music_isChorus integer\
	)";
	//清表操作
	std::string delsql=" delete from songinfo";
	//检查表是否存在
	std::string checksql = "select count(1) as count FROM sqlite_master WHERE type='table' AND name='songinfo' ";
	//创建表索引
	std::string sql_index="create index music_index on songinfo(music_id,music_singer,music_namepy,music_langtype,music_typeinfo,music_topinfo,music_namecount,music_singerpy,music_new)";


	try
	{
		connection c(m_dbname);

		//work wcheck(c,"check");

		sqlite3xx::result _result= c.exec(checksql);
		//wcheck.commit();

		int checkret=0;
		_result[0]["count"].to(checkret);

		//表已存在
		if(checkret>0)
		{
			//清表
			c.exec(delsql);
		}
		else
		{
			//重建表结构
			//work wcreart(c,"create");
			//wcreart.exec(presql);
			//wcreart.commit();
			c.exec(presql);

			//创建索引
			//work windex(c,"index");
			//windex.exec(sql_index);
			//windex.commit();
			c.exec(sql_index);
		}
	}catch(sql_error &err)
	{
			cerr << err.msg( ) << ": " << err.query( ) << endl;
			return XGSQLITE_FAILED;
	}
	catch(...)
	{
			return XGSQLITE_FAILED;
	}

	return XGSQLITE_SUCCESSD;
}

//搜索
int CMusicSQLObject::GetMusicInfo(int _pageindex,int _pagesize,list<CMusicInfo*> &_list)
{	
	if(_pagesize==0||_pageindex<=0)
		return XGSQLITE_FAILED;

	int startpos=(_pageindex-1)*_pagesize;
	int poscount=_pagesize;

	//int pagenum=0;
	//int musicnum=GetMusicCount();

	//if(musicnum%_pagesize>0)
	//	pagenum=musicnum/_pagesize+1;
	//else
	//	pagenum=musicnum/_pagesize;


	std::string table="select  music_id,music_name,music_singer,music_new,music_isScore,music_recommend,music_pop,music_play,music_isChorus  from songinfo  where 1=1   ";
	CreateSearchMusicConditionStr(table);

	//条件初次搜索时创建视图
	if(_pageindex==1&&m_CreateView)
	{
		if(CreateSearchView(table)==XGSQLITE_SUCCESSD)
			m_CreateView=false;
		else
			cout << " create view errror.." << endl;
	}

	std::string searchsql="select  *  from temp_view LIMIT $1,$2 ";

	try
	{	
		connection c(m_dbname);
		work wquery(c,"querysong");
		c.prepare("querysong",searchsql)
			("integer",prepare::treat_direct)
			("integer",prepare::treat_direct);

		sqlite3xx::result  t_result=wquery.prepared("querysong")(startpos)(poscount).exec();
		wquery.commit();
		cout << "found " << t_result.size( ) << " records.." << endl;

		for(sqlite3xx::result::size_type i=0;i<t_result.size( );i++)
		{
			CMusicInfo* pMusicInfo=new CMusicInfo();

			t_result[i]["music_id"].to(pMusicInfo->m_music_id);
			//t_result[i]["music_singertype"].to(pMusicInfo->m_music_singertype);
			//t_result[i]["music_langtype"].to(pMusicInfo->m_music_langtype);
			//t_result[i]["music_namecount"].to(pMusicInfo->m_music_namecount);
			//t_result[i]["music_lighteffect"].to(pMusicInfo->m_music_lighteffect);
			//t_result[i]["music_musicpos"].to(pMusicInfo->m_music_musicpos);
			//t_result[i]["music_lrbc"].to(pMusicInfo->m_music_lrbc);
			//t_result[i]["music_voice"].to(pMusicInfo->m_music_voicevalue);
			t_result[i]["music_new"].to(pMusicInfo->m_music_new);
			t_result[i]["music_isScore"].to(pMusicInfo->m_music_isScore);
			t_result[i]["music_isChorus"].to(pMusicInfo->m_music_isChorus);
			t_result[i]["music_recommend"].to(pMusicInfo->m_music_recommend);
			t_result[i]["music_pop"].to(pMusicInfo->m_music_pop);
			t_result[i]["music_play"].to(pMusicInfo->m_music_isPlay);

			std::string str;
			t_result[i]["music_name"].to(str);
			int nLen = (int)str.length();
			if(nLen > sizeof(pMusicInfo->m_music_name))
			{
				int glp = 1;
			}
			strcpy_s(pMusicInfo->m_music_name,sizeof(pMusicInfo->m_music_name),str.c_str());

			//t_result[i]["music_filename"].to(str);
			//strcpy_s(pMusicInfo->m_music_filename,sizeof(pMusicInfo->m_music_filename),str.c_str());

			//t_result[i]["music_typeinfo"].to(str);
			//strcpy_s(pMusicInfo->m_music_typeinfo,sizeof(pMusicInfo->m_music_typeinfo),str.c_str());

			t_result[i]["music_singer"].to(str);

			nLen = (int)str.length();
			if(nLen >= sizeof(pMusicInfo->m_music_singer))
			{
				int glp = 1;
			}
			strcpy_s(pMusicInfo->m_music_singer,sizeof(pMusicInfo->m_music_singer),str.c_str());

			//t_result[i]["music_namepy"].to(str);
			//strcpy_s(pMusicInfo->m_music_namepy,sizeof(pMusicInfo->m_music_namepy),str.c_str());

			//t_result[i]["music_singerpy"].to(str);
			//strcpy_s(pMusicInfo->m_music_singerpy,sizeof(pMusicInfo->m_music_singerpy),str.c_str());

			//t_result[i]["music_timemark"].to(str);
			//strcpy_s(pMusicInfo->m_music_timemark,sizeof(pMusicInfo->m_music_timemark),str.c_str());

			//t_result[i]["music_topinfo"].to(str);
			//strcpy_s(pMusicInfo->m_music_topinfo,sizeof(pMusicInfo->m_music_topinfo),str.c_str());

			_list.push_back(pMusicInfo);
		}
	}
	catch(sql_error& err)
	{
		cerr << err.msg( ) << ": " << err.query( ) << endl;
		return XGSQLITE_FAILED;
	}
	catch(...)
	{
		return XGSQLITE_FAILED;
	}

	return XGSQLITE_SUCCESSD;
}

//保存数据
int CMusicSQLObject::PutMusicInfo(list<CMusicInfo*> &_list)
{	
	std::string insertsql="insert into songinfo\
			(music_id,music_name,music_filename,music_singer,music_typeinfo,\
			music_namepy,	music_singerpy,music_singertype,music_langtype,music_topinfo,\
			music_namecount,music_voice,music_lrbc,music_musicpos,music_timemark,\
			music_lighteffect,music_isScore,music_new,music_recommend,music_pop,music_play,music_isChorus) \
			values($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12,$13,$14,$15,$16,$17,$18,$19,$20,$21,$22)";	//22个字段
	
	connection c(m_dbname);
	c.exec( "PRAGMA synchronous=0" );
	work winsert(c,"insert");
	try{
		c.exec("BEGIN TRANSACTION trans_ins");
		c.prepare("ins",insertsql)
				( "integer", sqlite3xx::prepare::treat_direct )				//music_id
				( "text", sqlite3xx::prepare::treat_direct )					//music_name
				( "text", sqlite3xx::prepare::treat_direct )					//music_filename
				( "text", sqlite3xx::prepare::treat_direct )					//music_singer
				( "text", sqlite3xx::prepare::treat_direct )					//music_typeinfo
				( "text", sqlite3xx::prepare::treat_direct )					//music_namepy
				( "text", sqlite3xx::prepare::treat_direct )					//music_singerpy
				( "integer", sqlite3xx::prepare::treat_direct )				//music_singertype
				( "integer", sqlite3xx::prepare::treat_direct )				//music_langtype
				( "text", sqlite3xx::prepare::treat_direct )					//music_topinfo
				( "integer", sqlite3xx::prepare::treat_direct )				//music_namecount
				( "integer", sqlite3xx::prepare::treat_direct )				//music_voice
				( "integer", sqlite3xx::prepare::treat_direct )				//music_lrbc
				( "text", sqlite3xx::prepare::treat_direct )				//music_musicpos
				( "text", sqlite3xx::prepare::treat_direct )					//music_timemark
				( "integer", sqlite3xx::prepare::treat_direct )				//music_lighteffect
				( "integer", sqlite3xx::prepare::treat_direct )				//music_isScore
				( "integer", sqlite3xx::prepare::treat_direct)				//music_new
				( "integer", sqlite3xx::prepare::treat_direct)				//music_recommend
				( "integer", sqlite3xx::prepare::treat_direct)				//music_pop
				( "integer", sqlite3xx::prepare::treat_direct)				//music_play
				( "integer", sqlite3xx::prepare::treat_direct)				//music_isChorus
				;
	
		list<CMusicInfo*>::iterator   it;
		for(it=_list.begin();it!=_list.end();it++)
		{
			CMusicInfo* pMusicInfo=(CMusicInfo*)*it;
			winsert.prepared("ins")
				(pMusicInfo->m_music_id)
				(pMusicInfo->m_music_name)
				(pMusicInfo->m_music_filename)
				(pMusicInfo->m_music_singer)
				(pMusicInfo->m_music_typeinfo)
				(pMusicInfo->m_music_namepy)
				(pMusicInfo->m_music_singerpy)
				(pMusicInfo->m_music_singertype)
				(pMusicInfo->m_music_langtype)
				(pMusicInfo->m_music_topinfo)
				(pMusicInfo->m_music_namecount)
				(pMusicInfo->m_music_voicevalue)
				(pMusicInfo->m_music_lrbc)
				(pMusicInfo->m_music_musicpos)
				(pMusicInfo->m_music_timemark)
				(pMusicInfo->m_music_lighteffect)
				(pMusicInfo->m_music_isScore)
				(pMusicInfo->m_music_new)
				(pMusicInfo->m_music_recommend)
				(pMusicInfo->m_music_pop)
				(pMusicInfo->m_music_isPlay)
				(pMusicInfo->m_music_isChorus)
				;

			sqlite3xx::result  t_result=c.prepared_exec("ins");
		}
		c.exec("COMMIT TRANSACTION trans_ins");
	}catch(sqlite3xx::sql_error err)
	{
		c.exec("ROLLBACK TRANSACTION trans_ins");
		return XGSQLITE_FAILED;
	}
	winsert.commit();
	return XGSQLITE_SUCCESSD;
}


//清除搜索条件
void CMusicSQLObject::ClearSearchCondition()
{
	memset(m_musicname,0,sizeof(m_musicname));
	memset(m_musicnamepy,0,sizeof(m_musicnamepy));
	memset(m_musicsinger,0,sizeof(m_musicsinger));
	memset(m_musicsingerpy,0,sizeof(m_musicsingerpy));
	memset(m_musictypeinfo,0,sizeof(m_musictypeinfo));
	memset(m_musictopinfo,0,sizeof(m_musictopinfo));

	m_musicsingertype=-1;
	m_musiclangtype=-1;
	m_musicnamecount=-1;
	m_musicnew=-1;
	m_musicisscore=-1;
	m_musicplay=-1;
	//设置重建视图标志位
	m_CreateView=true;
}
//设置搜索条件--歌曲名
void CMusicSQLObject::SetSearchConditionName(const char* value)
{
	strcpy_s(m_musicname,sizeof(m_musicname)-1,value);
}
//设置搜索条件--歌曲名拼音缩写
void CMusicSQLObject::SetSearchConditionNamePY(const char* value)
{
	strcpy_s(m_musicnamepy,sizeof(m_musicnamepy)-1,value);
}
//设置搜索条件--歌手名
void CMusicSQLObject::SetSearchConditionSinger(const char* value)
{
	strcpy_s(m_musicsinger,sizeof(m_musicsinger)-1,value);
}
//设置搜索条件--歌手名拼音缩写
void CMusicSQLObject::SetSearchConditionSingerPY(const char* value)
{
	strcpy_s(m_musicsingerpy,sizeof(m_musicsingerpy)-1,value);
}
////设置搜索条件--歌曲类型
void CMusicSQLObject::SetSearchConditionSongType(int value)
{
	if(value>16||value<=0)
		return;

	for(int i=1;i<=16;i++)
	{
		if(i==value)
		{
			m_musictypeinfo[i-1]='1';
			break;
		}
		else
		{
			m_musictypeinfo[i-1]='_';
		}
	}
}

//设置搜索条件--歌手类型
void CMusicSQLObject::SetSearchConditionSingerType(int value)
{
	m_musicsingertype=value;
}

//设置搜索条件--歌曲名字数
void CMusicSQLObject::SetSearchConditionNameCount(int value)
{
	m_musicnamecount=value;
}
//设置搜索条件--歌曲语言类型
void CMusicSQLObject::SetSearchConditionLangType(int value)
{
	m_musiclangtype=value;
}

//设置搜索条件--是否带评分（搜索歌手时无效）
void CMusicSQLObject::SetSearchConditionIsScore(int value )
{
	m_musicisscore=value;
}

//设置搜索条件--新曲推荐
void CMusicSQLObject::SetSearchConditionNew(int value)
{
	m_musicnew=value;
}

//设置搜索条件--默认播放歌曲
void CMusicSQLObject::SetSearchConditionIsPlay(int value)
{
	m_musicplay=value;
}
//取得搜索后的歌曲总数
int  CMusicSQLObject::GetMusicCount()
{
	int ret_count;
	connection c(m_dbname);
	work 	wquery(c,"querymusiccount");

	std::string sql="select count(1) as count from songinfo where 1=1 ";

	CreateSearchMusicConditionStr(sql);

	try{
		c.prepare("querymusiccount",sql);
		sqlite3xx::result t_result=wquery.prepared("querymusiccount").exec();
	
		ret_count=0;
		for(sqlite3xx::result::size_type i=0;i<t_result.size();i++)
		{
			t_result[i]["count"].to(ret_count);
		}
	}catch(sql_error &err)
	{
		cerr << err.msg( ) << ": " << err.query( ) << endl;
		ret_count=0;
	}
	catch(...)
	{
		ret_count=0;
	}

	return ret_count;
}

//取得搜索歌手总数
int  CMusicSQLObject::GetSingerCount()
{
	int ret_count=0;
	
	std::string sql="select count(distinct  music_singer) as count from songinfo where 1=1 ";

	CreateSearchSingerConditionStr(sql,1);
	try{
		connection c(m_dbname);
		work wqry(c,"qrysingercont");
		sqlite3xx::result t_result=wqry.exec(sql);

		for(sqlite3xx::result::size_type i =0;i<t_result.size();i++)
		{
			t_result[i]["count"].to(ret_count); 
		}
		wqry.commit();
	}catch(sql_error &err)
	{
		cerr << err.msg( ) << ": " << err.query( ) << endl;
		ret_count=0; 
	}
	catch(...)
	{
		ret_count=0; 
	}
	return ret_count;
}

//取得歌手列表
int CMusicSQLObject::GetSingerList(int _pageindex,int _pagesize,list<CSingerInfo*> &_list)
{
	int startpos=(_pageindex-1)*_pagesize;
	int poscount=_pagesize;
	//int singernum=GetSingerCount();
	//int pagenum=0;

	if(_pagesize==0||_pageindex<=0)
		return XGSQLITE_FAILED;

	/*if(singernum%_pagesize>0)
		pagenum=singernum/_pagesize+1;
	else
		pagenum=singernum/_pagesize;

	if(singernum<=0)
		return singernum;*/

	std::string sql="select music_singer,num,music_singertype from ( ";
	std::string table="select distinct  music_singer,music_singertype,count(1)  as num from songinfo where 1=1 ";
	CreateSearchSingerConditionStr(table);
	sql.append(table);
	sql.append( ")  LIMIT $1,$2 ");

	
	try{
		connection c(m_dbname);
		work wquery(c,"qrysinger");
		c.prepare("qrysinger",sql)
			("integer",prepare::treat_direct)
			("integer",prepare::treat_direct);
		sqlite3xx::result t_result=wquery.prepared("qrysinger")(startpos)(poscount).exec();
		
			for(sqlite3xx::result::size_type i =0;i<t_result.size();i++)
			{
				std::string str;
				t_result[i]["music_singer"].to(str);

				CSingerInfo* pSingerinfo=new CSingerInfo();
				strcpy_s(pSingerinfo->m_singername,sizeof(pSingerinfo->m_singername),str.c_str());

				t_result[i]["num"].to(pSingerinfo->m_songnum);
				t_result[i]["music_singertype"].to(pSingerinfo->m_singertype);

				_list.push_back(pSingerinfo);

				
			}
	}
	catch(sql_error &err)
	{
		cerr<<err.msg()<<":"<<err.query()<<endl;
		return XGSQLITE_FAILED;
	}
	catch(...)
	{
		return XGSQLITE_FAILED;
	}

	return XGSQLITE_SUCCESSD;
}


void CMusicSQLObject::CreateSearchMusicConditionStr(std::string& _condition)
{
	char temp[1024]={0};

	if(strlen(m_musicname)>0)
	{
		memset(temp,0,sizeof(temp));
		sprintf_s(temp,sizeof(temp)-1,"and music_name like '%s%s%s' ","%",m_musicname,"%");
		_condition.append(temp);
	}

	if(strlen(m_musicsinger)>0)
	{
		memset(temp,0,sizeof(temp));
		sprintf_s(temp,sizeof(temp)-1,"and music_singer like '%s%s' ",m_musicsinger,"%");
		_condition.append(temp);
	}

	if(strlen(m_musicnamepy)>0)
	{
		memset(temp,0,sizeof(temp));
		sprintf_s(temp,sizeof(temp)-1,"and music_namepy like '%s%s' ",m_musicnamepy,"%");
		_condition.append(temp);
	}

	if(strlen(m_musictypeinfo)>0)
	{
		memset(temp,0,sizeof(temp));
		sprintf_s(temp,sizeof(temp)-1,"and music_typeinfo like '%s%s' ",m_musictypeinfo,"%");
		_condition.append(temp);
	}

	if(m_musicsingertype!=SINGER_TYPE_ALL)
	{
		memset(temp,0,sizeof(temp));
		sprintf_s(temp,sizeof(temp)-1,"and music_singertype=%d ",m_musicsingertype);
		_condition.append(temp);
	}

	if(m_musicnamecount>0)
	{
		if(m_musicnamecount>=9 )//搜索9字以上歌曲的所有歌曲
		{
			memset(temp,0,sizeof(temp));
			sprintf_s(temp,sizeof(temp)-1,"and music_namecount>=%d ",m_musicnamecount);
			_condition.append(temp);
		}
		else
		{
			memset(temp,0,sizeof(temp));
			sprintf_s(temp,sizeof(temp)-1,"and music_namecount=%d ",m_musicnamecount);
			_condition.append(temp);
		}
	}

	
	if(m_musiclangtype!=LANG_TYPE_ALL)
	{
		memset(temp,0,sizeof(temp));
		sprintf_s(temp,sizeof(temp)-1,"and music_langtype=%d ",m_musiclangtype);
		_condition.append(temp);
	}
	
	if(m_musicisscore!=-1)
	{
		memset(temp,0,sizeof(temp));
		sprintf_s(temp,sizeof(temp)-1,"and music_isScore=%d ",m_musicisscore);
		_condition.append(temp);
	}

	if(strlen(m_musictopinfo)>0)
	{
		memset(temp,0,sizeof(temp));
		sprintf_s(temp,sizeof(temp)-1,"and music_topinfo like '%s%s' ",m_musictopinfo,"%");
		_condition.append(temp);
	}

	if(m_musicnew!=-1)
	{
		memset(temp,0,sizeof(temp));
		sprintf_s(temp,sizeof(temp)-1,"and music_new = %d ",m_musicnew);
		_condition.append(temp);
	}

	if(m_musicplay!=-1)
	{
		memset(temp,0,sizeof(temp));
		sprintf_s(temp,sizeof(temp)-1,"and music_play = %d ",m_musicplay);
		_condition.append(temp);
	}
	_condition.append(" order by music_namepy");

}


void CMusicSQLObject::CreateSearchSingerConditionStr(std::string& _condition,int _ByCount)
{
	char temp[1024]={0};

	if(strlen(m_musicsinger)>0)
	{
		memset(temp,0,sizeof(temp));
		sprintf_s(temp,sizeof(temp)-1,"and music_singer like '%s%s' ",m_musicsinger,"%");
		_condition.append(temp);
	}

	if(strlen(m_musicsingerpy)>0)
	{
		memset(temp,0,sizeof(temp));
		sprintf_s(temp,sizeof(temp)-1,"and music_singerpy like '%s%s' ",m_musicsingerpy,"%");
		_condition.append(temp);
	}

	if(m_musicsingertype!=SINGER_TYPE_ALL)
	{
		memset(temp,0,sizeof(temp));
		sprintf_s(temp,sizeof(temp)-1,"and music_singertype = %d ",m_musicsingertype);
		_condition.append(temp);
	}

	if(_ByCount==0)
	{
		_condition.append(" group by music_singer ");
		_condition.append(" order by music_singerPY");
	}
}

int CMusicSQLObject::GetMusicInfoById(int _musicid,CMusicInfo  &_musicinfo)
{
	
	std::string  selectsql="select music_id,music_name,music_filename,music_singer,music_typeinfo,\
			music_namepy,	music_singerpy,music_singertype,music_langtype,music_topinfo,\
			music_namecount,music_voice,music_lrbc,music_musicpos,music_timemark,\
			music_lighteffect,	music_isScore,music_new,music_recommend,music_pop,music_play\
			from songinfo where music_id=$1 ";
	try{
			connection c(m_dbname);

			work wselect(c,"select");

			c.prepare("select",selectsql)
					("integer",prepare::treat_direct);
			
			sqlite3xx::result  t_result=wselect.prepared("select")(_musicid).exec();
			wselect.commit();

			_musicinfo.ClearInfo();
			for(sqlite3xx::result::size_type i=0;i<t_result.size( );i++)
			{
					t_result[i]["music_id"].to(_musicinfo.m_music_id);
					//t_result[i]["music_singertype"].to(_musicinfo.m_music_singertype);
					//t_result[i]["music_langtype"].to(_musicinfo.m_music_langtype);
					//t_result[i]["music_namecount"].to(_musicinfo.m_music_namecount);
					t_result[i]["music_lighteffect"].to(_musicinfo.m_music_lighteffect);
					//t_result[i]["music_musicpos"].to(_musicinfo.m_music_musicpos);
					t_result[i]["music_lrbc"].to(_musicinfo.m_music_lrbc);
					t_result[i]["music_voice"].to(_musicinfo.m_music_voicevalue);
					t_result[i]["music_new"].to(_musicinfo.m_music_new);
					t_result[i]["music_isScore"].to(_musicinfo.m_music_isScore);
					t_result[i]["music_recommend"].to(_musicinfo.m_music_recommend);
					t_result[i]["music_pop"].to(_musicinfo.m_music_pop);
					t_result[i]["music_play"].to(_musicinfo.m_music_isPlay);

					std::string str;
					t_result[i]["music_name"].to(str);
					strcpy_s(_musicinfo.m_music_name,sizeof(_musicinfo.m_music_name),str.c_str());

					t_result[i]["music_filename"].to(str);
					strcpy_s(_musicinfo.m_music_filename,sizeof(_musicinfo.m_music_filename),str.c_str());

					//t_result[i]["music_typeinfo"].to(str);
					//strcpy_s(_musicinfo.m_music_typeinfo,sizeof(_musicinfo.m_music_typeinfo),str.c_str());

					t_result[i]["music_singer"].to(str);
					strcpy_s(_musicinfo.m_music_singer,sizeof(_musicinfo.m_music_singer),str.c_str());

					//t_result[i]["music_namepy"].to(str);
					//strcpy_s(_musicinfo.m_music_namepy,sizeof(_musicinfo.m_music_namepy),str.c_str());

					//t_result[i]["music_singerpy"].to(str);
					//strcpy_s(_musicinfo.m_music_singerpy,sizeof(_musicinfo.m_music_singerpy),str.c_str());

					t_result[i]["music_timemark"].to(str);
					strcpy_s(_musicinfo.m_music_timemark,sizeof(_musicinfo.m_music_timemark),str.c_str());

					t_result[i]["music_topinfo"].to(str);
					strcpy_s(_musicinfo.m_music_topinfo,sizeof(_musicinfo.m_music_topinfo),str.c_str());
	
					t_result[i]["music_musicpos"].to(str);
					strcpy_s(_musicinfo.m_music_musicpos,sizeof(_musicinfo.m_music_musicpos),str.c_str());

			}

	}catch(sql_error &err)
	{
		cerr << err.msg( ) << ": " << err.query( ) << endl;
		return XGSQLITE_FAILED;
	}
	catch(...)
	{
		return XGSQLITE_FAILED;
	}
	return XGSQLITE_SUCCESSD;
}

void CMusicSQLObject::SetSearchConditionTopType(int value)
{
	if(value>16||value<=0)
		return;

	for(int i=1;i<=16;i++)
	{
		if(i==value)
		{
			m_musictopinfo[i-1]='1';
			break;
		}
		else
		{
			m_musictopinfo[i-1]='_';
		}
	}
	return ;
}

int CMusicSQLObject::CreateSearchView(std::string _searcher)
{
	//重建视图
	std::string del="drop view if  exists temp_view ";

	std::string sql="create view temp_view as ";

	sql.append(_searcher);

	try
	{
		connection c(m_dbname);
		//work wquery(c,"createview");
		c.exec(del);
		c.exec(sql);
	}
	catch(sql_error& err)
	{
		cerr << err.msg( ) << ": " << err.query( ) << endl;
		return XGSQLITE_FAILED;
	}
	catch(...)
	{
		return XGSQLITE_FAILED;
	}

	return XGSQLITE_SUCCESSD;
}


int CMusicSQLObject::GetSingerByMusicID(int _musicid,CSingerInfo &_singerinfo)
{
	std::string sql="select distinct  music_singer,music_singertype,count(1)  as num from songinfo where music_singer in (select music_singer from songinfo where music_id=$1) group by music_singer  ";

	try
	{
		connection c(m_dbname);
		work wquery(c,"qrysinger");
		c.prepare("qrysinger",sql)
			("integer",prepare::treat_direct);

		sqlite3xx::result t_result=wquery.prepared("qrysinger")(_musicid).exec();

		for(sqlite3xx::result::size_type i =0;i<t_result.size();i++)
			{
				std::string str;
				t_result[i]["music_singer"].to(str);

				strcpy_s(_singerinfo.m_singername,sizeof(_singerinfo.m_singername),str.c_str());

				t_result[i]["music_singertype"].to(_singerinfo.m_singertype);
				t_result[i]["num"].to(_singerinfo.m_songnum);
			}

	}
	catch(sql_error &err)
	{
		cerr<<err.msg()<<":"<<err.query()<<endl;
		return XGSQLITE_FAILED;
	}
	catch(...)
	{
		return XGSQLITE_FAILED;
	}

	return XGSQLITE_SUCCESSD;
}

/*
int  CMusicSQLObject::GetMusicInfoBySinger(int _pageindex,int _pagesize,list<CMusicInfo*> &_list )
{
	int startpos=(_pageindex-1)*_pagesize;
	int poscount=_pagesize;

	if(_pagesize==0||_pageindex<=0)
		return XGSQLITE_FAILED;

	std::string table="select music_id,music_name,music_singer,music_new,music_isScore,music_recommend,music_pop,music_play  from songinfo where 1=1  ";

	char temp[1024]={0};

	if(m_musicsingertype>0)
	{
		memset(temp,0,1024);
		sprintf_s(temp,sizeof(temp)-1,"and music_singertype=%d  ",m_musicsingertype);
		table.append(temp);
	}

	memset(temp,0,1024);
	sprintf_s(temp,sizeof(temp)-1,"and (music_singer='%s' or music_singer like '%s/%s%s' or music_singer like '%s/%s') ",m_musicsinger,"%",m_musicsinger,"%",m_musicsinger,"%");
	table.append(temp);

	//条件初次搜索时创建视图
	if(_pageindex==1&&m_CreateView)
	{
		if(CreateSearchView(table)==XGSQLITE_SUCCESSD)
			m_CreateView=false;
		else
			cout << " create view errror.." << endl;
	}

	std::string searchsql="select  *  from temp_view LIMIT $1,$2 ";

	try
	{	
		connection c(m_dbname);
		work wquery(c,"querysong");
		c.prepare("querysong",searchsql)
			("integer",prepare::treat_direct)
			("integer",prepare::treat_direct);

		sqlite3xx::result  t_result=wquery.prepared("querysong")(startpos)(poscount).exec();
		wquery.commit();
		cout << "found " << t_result.size( ) << " records.." << endl;

		for(sqlite3xx::result::size_type i=0;i<t_result.size( );i++)
		{
			CMusicInfo* pMusicInfo=new CMusicInfo();

			t_result[i]["music_id"].to(pMusicInfo->m_music_id);
			t_result[i]["music_new"].to(pMusicInfo->m_music_new);
			t_result[i]["music_isScore"].to(pMusicInfo->m_music_isScore);
			t_result[i]["music_recommend"].to(pMusicInfo->m_music_recommend);
			t_result[i]["music_pop"].to(pMusicInfo->m_music_pop);
			t_result[i]["music_play"].to(pMusicInfo->m_music_isPlay);

			std::string str;
			t_result[i]["music_name"].to(str);
			strcpy_s(pMusicInfo->m_music_name,sizeof(pMusicInfo->m_music_name),str.c_str());

			t_result[i]["music_singer"].to(str);
			strcpy_s(pMusicInfo->m_music_singer,sizeof(pMusicInfo->m_music_singer),str.c_str());

			_list.push_back(pMusicInfo);
		}
	}
	catch(sql_error& err)
	{
		cerr << err.msg( ) << ": " << err.query( ) << endl;
		return XGSQLITE_FAILED;
	}
	catch(...)
	{
		return XGSQLITE_FAILED;
	}

	return XGSQLITE_SUCCESSD;
}

*/
/*
int CMusicSQLObject::GetMusicInfoBySingerCount()
{
	int ret_count=0;
	std::string sql="select count(1) as musiccount  from songinfo where 1=1  ";

	char temp[1024]={0};

	if(m_musicsingertype>0)
	{
		memset(temp,0,1024);
		sprintf_s(temp,sizeof(temp)-1,"and music_singertype=%d  ",m_musicsingertype);
		sql.append(temp);
	}

	memset(temp,0,1024);
	sprintf_s(temp,sizeof(temp)-1,"and (music_singer='%s' or music_singer like '%s/%s%s' or music_singer like '%s/%s') ",m_musicsinger,"%",m_musicsinger,"%",m_musicsinger,"%");
	sql.append(temp);


	connection c(m_dbname);
	work 	wquery(c,"querymusiccount");
	try{
		c.prepare("querymusiccount",sql);
		sqlite3xx::result t_result=wquery.prepared("querymusiccount").exec();
	
		ret_count=0;
		for(sqlite3xx::result::size_type i=0;i<t_result.size();i++)
		{
			t_result[i]["musiccount"].to(ret_count);
		}
	}catch(sql_error &err)
	{
		cerr << err.msg( ) << ": " << err.query( ) << endl;
		ret_count=0;
	}
	catch(...)
	{
		ret_count=0;
	}

	return ret_count;
}
*/