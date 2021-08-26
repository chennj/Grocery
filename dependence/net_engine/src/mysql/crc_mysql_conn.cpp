#include "../../include/mysql/crc_mysql_conn.h"
#include "../../include/mysql/crc_mysql_pool_conn.h"
#include "../../include/mysql/crc_mysql_resultset.h"
#include "../../include/core/crc_log.h"

CRCMysqlConn::CRCMysqlConn(CRCMysqlConnPool * pool)
{
    _conn_pool = pool;
    _mysql = NULL;
}

CRCMysqlConn::~CRCMysqlConn()
{
    if (_mysql){
        mysql_close(_mysql);
        _mysql = NULL;
    }
}

int
CRCMysqlConn::Init()
{
    _mysql = mysql_init(NULL);
    if (!_mysql){
        CRCLog::Error("mysql init failed!\n");
        return 1;
    }

    my_bool reconnect = true;

    mysql_options(_mysql, MYSQL_OPT_RECONNECT, &reconnect);
    mysql_options(_mysql, MYSQL_SET_CHARSET_NAME, "utf8mb4");

    if (!mysql_real_connect(
        _mysql, 
        _conn_pool->GetServerIp(), 
        _conn_pool->GetUserName(),
        _conn_pool->GetPassword(),
        _conn_pool->GetDBName(),
        _conn_pool->GetServerPort(),
        NULL,0)){
        CRCLog::Error("mysql_real_connect is failed! %s", mysql_error(_mysql));
        return 2;
    }

    return 0;
}

const char* 
CRCMysqlConn::GetPoolName()
{
    return _conn_pool->Name();
}

bool 
CRCMysqlConn::ExecuteCreate(const char* sql)
{
    if (!_mysql){return false;}

    //当连接丢失的时候，使用PING能够自动重连
    mysql_ping(_mysql);

    if (mysql_real_query(_mysql, sql, strlen(sql))){
        CRCLog::Error("CRCMysqlConn::ExecuteCreate mysql_real_query failed,  %s\n%s", mysql_error(_mysql),sql);
        return false;
    }

    return true;
}

bool 
CRCMysqlConn::ExecuteDrop(const char* sql)
{
    if (!_mysql){return false;}

    //当连接丢失的时候，使用PING能够自动重连
    mysql_ping(_mysql);

    if (mysql_real_query(_mysql, sql, strlen(sql))){
        CRCLog::Error("CRCMysqlConn::ExecuteDrop mysql_real_query failed, %s\n%s", mysql_error(_mysql),sql);
        return false;
    }

    return true;
}

bool
CRCMysqlConn::ExecuteUpdate(const char* sql, bool care_affected_rows /*= true*/)
{
	mysql_ping(_mysql);
	if (mysql_real_query(_mysql, sql, strlen(sql)))
	{
		 CRCLog::Error("CRCMysqlConn::ExecuteUpdate mysql_real_query failed, %s\n%s", mysql_error(_mysql),sql);
		return false;
	}

	if (mysql_affected_rows(_mysql) >0)
	{
		return 0;
	}


	if (care_affected_rows)
	{
        CRCLog::Error("CRCMysqlConn::ExecuteUpdate real_query failed, %s\n%s", mysql_error(_mysql),sql);
		return false;
	}

	return true;
}

CRCResultSet* 
CRCMysqlConn::ExecuteQuery(const char* sql)
{
	mysql_ping(_mysql);

	if (mysql_real_query(_mysql,sql,strlen(sql)))
	{
		return NULL;
	}
	MYSQL_RES *res = mysql_store_result(_mysql);
	if (NULL == res)
	{
		if (mysql_field_count(_mysql) >0)
		{
			CRCLog::Error("CRCMysqlConn::ExecuteQuery real_query failed, %s\n%s", mysql_error(_mysql),sql);
		}
		return  NULL;
	}

	CRCResultSet *rset = new CRCResultSet(res);
	if (!rset)
	{
		return NULL;
	}
	return rset;
}

unsigned int 
CRCMysqlConn::GetInsertId()
{
	return (uint32_t)mysql_insert_id(_mysql);
}

bool 
CRCMysqlConn::StartTransaction()
{
	mysql_ping(_mysql);
	if (mysql_real_query(_mysql, "start transaction\n", 17)) {
        CRCLog::Error("CRCMysqlConn::StartTransaction failed, %s\n", mysql_error(_mysql));
		return false;
	}
	return true;
}

bool 
CRCMysqlConn::Commit()
{

	mysql_ping(_mysql);
	if (mysql_real_query(_mysql, "commit\n",6)) {
		CRCLog::Error("CRCMysqlConn::commit %s\n", mysql_error(_mysql));
		return false;
	}
	return true;
}

bool 
CRCMysqlConn::Rollback()
{
	mysql_ping(_mysql);
	if (mysql_real_query(_mysql, "rollback\n", 8)) {
		CRCLog::Error("CRCMysqlConn::Rollback %s\n", mysql_error(_mysql));
		return false;
	}
	return true;

}