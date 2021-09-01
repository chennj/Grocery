#ifndef _CRC_MYSQL_TABLE_H_
#define _CRC_MYSQL_TABLE_H_

#include "crc_mysql_pool_conn.h"
#include "crc_mysql_preparestatement.h"
#include "crc_mysql_resultset.h"
#include <time.h>
#include <sstream>
#include <string>

using namespace std;

class CRCTable {
private:
	CRCMysqlConn *conn;
public:
	CRCTable(CRCMysqlConn *con) { conn = con; }
	virtual ~CRCTable() {}
public:
	virtual bool insertUser() = 0;
	virtual bool updateInfo(uint32_t id =2) = 0;
	virtual bool queryUser(uint32_t id = 1) = 0;

	CRCMysqlConn *GetConn() { return conn; }
};

class UserTable :public CRCTable
{
public:
	UserTable(CRCMysqlConn *c) ;
	~UserTable() {}

	bool insertUser();
	bool updateInfo(uint32_t id = 2);
	bool queryUser(uint32_t id = 1);
private:
	uint32_t nId;
	uint8_t nSex;
	uint8_t nStatus;
	uint32_t nValidateMethod; 
	uint32_t nDeptId;
	string strNick;
	string strDomain;
	string strName;
	string strTel;
	string strEmail;
	string strAvatar;
	string sign_info;
	string strPass; 
	string strCompany; 
	string strAddress; 

};

UserTable::UserTable(CRCMysqlConn *c):CRCTable(c) 
{
	 nId =0;
	 nSex=1;
	 nStatus=1; 
	 nValidateMethod =0; 
	 nDeptId=0;
	 strNick="test";
	 strDomain = "test";
	 strName = "test";
	 strTel = "test";
	 strEmail = "test";
	 strAvatar = "test";
	 sign_info = "test";
	 strPass = "test"; 
	 strCompany = "test"; 
	 strAddress = "test"; 
}

bool UserTable::insertUser()
{
	bool bRet = false;
	string strSql;
	strSql = "insert into IMUser(`salt`,`sex`,`nick`,`password`,`domain`,`name`,`phone`,`email`,`company`,`address`,`avatar`,`sign_info`,`departId`,`status`,`created`,`updated`) values(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";

	CRCPrepareStatement* stmt = new CRCPrepareStatement();
	MYSQL *mysql = this->GetConn()->GetMysql();


	if (stmt->Init(mysql, strSql)) {
	
		uint32_t nNow = (uint32_t)time(NULL);
		uint32_t index = 0;
		string strOutPass = "987654321";
		string strSalt = "abcd";

	
		stmt->SetParam(index++, strSalt);
		stmt->SetParam(index++, this->nSex);
		stmt->SetParam(index++, this->strNick);
		stmt->SetParam(index++, strOutPass);
		stmt->SetParam(index++, this->strDomain);
		stmt->SetParam(index++, this->strName);
		stmt->SetParam(index++, this->strTel);
		stmt->SetParam(index++, this->strEmail);
		stmt->SetParam(index++, this->strCompany);
		stmt->SetParam(index++, this->strAddress);
		stmt->SetParam(index++, this->strAvatar);
		stmt->SetParam(index++, this->sign_info);
		stmt->SetParam(index++, this->nDeptId);
		stmt->SetParam(index++, this->nStatus);
		stmt->SetParam(index++, nNow);
		stmt->SetParam(index++, nNow);

		
		bRet = stmt->Update();
		if (bRet)
		{
			uint32_t id = stmt->GetInsertId();
			if(id %300 == 0)
				CRCLog::Info("register then get User_id%d", id);
		}
	}
	delete stmt;
	return bRet;
}
static string int2string(uint32_t user_id)
{
	stringstream ss;
	ss << user_id;
	return ss.str();
}

bool UserTable::updateInfo(uint32_t id) {

	bool bRet = false;

	strDomain = "naihan";

	uint32_t nNow = (uint32_t)time(NULL);
	string strSql = "update IMUser set `sex`=" + int2string(2) + ", `nick`='" + strNick
		+ "', `domain`='" + strDomain + "', `name`='" + strName + "', `phone`='" + strTel
		+ "', `email`='" + strEmail + "', `avatar`='" + strAvatar + "', `sign_info`='" + sign_info
		+ "', `departId`='" + int2string(nDeptId) + "', `status`=" + int2string(nStatus) + ", `updated`=" + int2string(nNow)
		+ ", `company`='" + strCompany + "', `address`='" + strAddress + "' where id=" + int2string(id);

	bRet = GetConn()->ExecuteUpdate(strSql.c_str());
	if (!bRet)
	{
		CRCLog::Info("update user fail %s\n",strSql.c_str());
	}
	return bRet;
}

bool UserTable::queryUser(uint32_t id)
{

	string strsql = "select * from IMUser where id=" + int2string(id);

	CRCResultSet *pResultSet = GetConn()->ExecuteQuery(strsql.c_str());

	bool bRet = false;
	if (pResultSet)
	{
		if (pResultSet->Next())
		{
			nId = pResultSet->GetInt("id");
			nSex = pResultSet->GetInt("sex");
			strNick = pResultSet->GetString("nick");
			strDomain = pResultSet->GetString("domain");
			strName = pResultSet->GetString("name");
			strTel = pResultSet->GetString("phone");
			strEmail = pResultSet->GetString("email");
			strAvatar = pResultSet->GetString("avatar");
			sign_info = pResultSet->GetString("sign_info");
			nDeptId = pResultSet->GetInt("departId");
			nValidateMethod = pResultSet->GetInt("validateMethod");
			nStatus = pResultSet->GetInt("status");

			CRCLog::Info("nId:%u", nId);
			CRCLog::Info("nSex:%u", nSex);
			CRCLog::Info("strNick:%s", strNick.c_str());
			CRCLog::Info("strDomain:%s", strDomain.c_str());
			CRCLog::Info("strName:%s", strName.c_str());
			CRCLog::Info("strTel:%s", strTel.c_str());
			CRCLog::Info("strEmail:%s", strEmail.c_str());
			CRCLog::Info("sign_info:%s", sign_info.c_str());

			bRet = true;

		}
		delete pResultSet;
	}
	else {
		CRCLog::Info("no result for sql %s",strsql.c_str());
	}
	return bRet;
}
#endif