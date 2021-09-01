#ifndef _CRC_MYSQL_POOL_CONN_H_
#define _CRC_MYSQL_POOL_CONN_H_

#include "../pool/crc_pool_conn.h"
#include "crc_mysql_conn.h"
#include <atomic>

class CRCMysqlConnPool : public CRCConnPool<CRCMysqlConn>
{
    //最小连接数
    #define MIN_DB_CONN_CNT 2; 
private:
    const char*     _PoolName;
    const char*     _ServerIp;
    int             _ServerPort;
    const char*     _UserName;
    const char*     _Password;
    const char*     _DbName;

    std::atomic_int _CurConnCnt;
    unsigned int    _MinConnCnt;
    unsigned int    _MaxConnCnt;

public: 
    CRCMysqlConnPool(
        const char* poolName, 
        const char* serverIp, int serverPort, 
        const char* userName, const char* password,
        const char* dbName,
        unsigned int _MinConnCnt,
        unsigned int maxConnCnt);
    ~CRCMysqlConnPool();

public:

    inline const char*  GetServerIp()    {return _ServerIp;}
    inline int          GetServerPort()  {return _ServerPort;}
    inline const char*  GetUserName()    {return _UserName;}
    inline const char*  GetPassword()    {return _Password;}
    inline const char*  GetDBName()      {return _DbName;}

public:
    const char* Name() override;
    int GetCurConnCnt() override;
    int GetMinConnCnt() override;
    int GetMaxConnCnt() override;
    void IncreaseCurConnCnt() override;
};

#endif