#ifndef _CRC_MYSQL_CONN_H_
#define _CRC_MYSQL_CONN_H_

#include "../pool/crc_pool_conn.h"

#include <mysql/mysql.h>
#include <mysql/errmsg.h>
#include <string>
#include <map>

#define MAX_ESCAPE_STRING_LEN 10240

class CRCMysqlConnPool;
class CRCResultSet;

class CRCMysqlConn
{
private: 
    CRCMysqlConnPool* _conn_pool;
    MYSQL * _mysql;
    char _escape_string[MAX_ESCAPE_STRING_LEN+1];

public: 
    CRCMysqlConn(CRCConnPool<CRCMysqlConn>* pool);
    ~CRCMysqlConn();

public: 
    inline MYSQL* GetMysql(){return _mysql;}
    
    int Init();
    //获取连接池名
    const char* GetPoolName();
    //建表
    bool ExecuteCreate(const char* sql);
    //删表
    bool ExecuteDrop(const char* sql);
    //查询
    CRCResultSet* ExecuteQuery(const char* sql);
    //修改
    bool ExecuteUpdate(const char* sql, bool care_affected_rows = true);

    unsigned int  GetInsertId();

    //事务开始
    bool StartTransaction();
    //事务提交
    bool Commit();
    //事务回滚
    bool Rollback();

};

#endif