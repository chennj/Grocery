#ifndef _CRC_MYSQL_RESULTSET_H_
#define _CRC_MYSQL_RESULTSET_H_

#include <mysql/mysql.h>
#include <mysql/errmsg.h>
#include <string>
#include <map>

//结果集，select的时候用
class CRCResultSet
{
private: 
    MYSQL_RES* _res;
    MYSQL_ROW _row;
    std::map<std::string, int> _key_map;

    int _GetIndex(const char* key);

public: 
    CRCResultSet(MYSQL_RES* res);
    virtual ~CRCResultSet();

public: 
    bool Next();
    int GetInt(const char* key);
    char* GetString(const char* key);
};

#endif