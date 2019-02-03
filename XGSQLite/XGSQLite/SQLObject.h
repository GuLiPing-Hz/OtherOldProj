#pragma once
#ifndef _H_SQLOBJECT
#define _H_SQLOBJECT
namespace XGSQLITE{

class __declspec( dllexport ) CSQLDate
{
public:
	CSQLDate(void){};
	virtual ~CSQLDate(void){};
private:
};

class __declspec( dllexport ) CSQLObject
{
public:
	CSQLObject(void){};

	virtual ~CSQLObject(void){};

	virtual int Initialization (void)=0;

	virtual int Search(int pageindex,int pagesize=10) =0;

	virtual int Save() =0;
	
	virtual CSQLObject* GetResultLine()=0;

};
};
#endif