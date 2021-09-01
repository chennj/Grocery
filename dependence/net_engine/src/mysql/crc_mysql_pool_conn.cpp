#include "../../include/mysql/crc_mysql_pool_conn.h"
#include "../../include/core/crc_log.h"

CRCMysqlConnPool::CRCMysqlConnPool(
        const char* poolName, 
        const char* serverIp, int serverPort, 
        const char* userName, const char* password,
        const char* dbName,
        unsigned int minConnCnt,
        unsigned int maxConnCnt)
{
    _PoolName   = poolName;
    _ServerIp   = serverIp;
    _ServerPort = serverPort;
    _UserName   = userName;
    _Password   = password;
    _DbName     = dbName;
    _CurConnCnt = MIN_DB_CONN_CNT;
    _MinConnCnt = minConnCnt;
    _MaxConnCnt = maxConnCnt;
}

CRCMysqlConnPool::~CRCMysqlConnPool()
{
}

const char *
CRCMysqlConnPool::Name()
{
    return _PoolName;
}

int
CRCMysqlConnPool::GetCurConnCnt()
{
    return _CurConnCnt;
}

int
CRCMysqlConnPool::GetMinConnCnt()
{
    return _MinConnCnt;
}

int
CRCMysqlConnPool::GetMaxConnCnt()
{
    return _MaxConnCnt;
}

void
CRCMysqlConnPool::IncreaseCurConnCnt()
{
    _CurConnCnt++;
}