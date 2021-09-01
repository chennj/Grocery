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
    uint32_t GetInsertId();

    void SetParam(uint32_t index, int & value);
    void SetParam(uint32_t index, uint8_t  & value);
    void SetParam(uint32_t index, uint32_t & value);
    void SetParam(uint32_t index, std::string & value);
    void SetParam(uint32_t index, const std::string & value);
};


#endif