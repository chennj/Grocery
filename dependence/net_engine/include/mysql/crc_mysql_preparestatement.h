#ifndef _CRC_MYSQL_PREPARESTATEMENT_H_
#define _CRC_MYSQL_PREPARESTATEMENT_H_

#include <mysql/mysql.h>
#include <mysql/errmsg.h>
#include <string>

//插入数据用
class CRCPrepareStatement
{
private:
    MYSQL_STMT*     _stmt;
    MYSQL_BIND*     _param_bind;
    unsigned long   _param_cnt;
public: 
    CRCPrepareStatement();
    virtual ~CRCPrepareStatement();
public: 
    bool Init(MYSQL* mysql, std::string & sql);
    bool Update();
    u_int32_t GetInsertId();

    void SetParam(u_int32_t index, int & value);
    void SetParam(u_int32_t index, u_int8_t  & value);
    void SetParam(u_int32_t index, u_int32_t & value);
    void SetParam(u_int32_t index, std::string & value);
    void SetParam(u_int32_t index, const std::string & value);
};


#endif