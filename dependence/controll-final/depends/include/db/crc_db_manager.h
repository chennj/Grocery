/**
 * 
 * author:  chenningjiang
 * desc:    sqlite数据库封装
 * 
 * */
#ifndef _CRC_DB_MANAGER_H_
#define _CRC_DB_MANAGER_H_

#include "crc_log.h"
#include "CppSQLite3.h"
#include "CJsonObject.hpp"
#include <sstream>

using CRCJson = neb::CJsonObject;

class CRCDBManager
{
protected:
    CppSQLite3DB    m_db;
    std::string     m_db_name;
    CRCTimestamp    m_timestamp;

public:
    bool open(const char* db_name);

    bool close();

    void begin();

    void commit();

    void run();

    bool tableExists(const char* szTable);

    bool query2json(CppSQLite3Query& query, CRCJson& json);

    bool execQuery(const char* sql, CRCJson& json);

    bool execQuery(const char* sql);

    int  execDML(const char* sql);

    template<typename vT>
    bool hasByKV(const char* table, const char* k, vT v)
    {//sql = "SELECT 1 FROM table WHERE k=v LIMIT 1;"
        std::stringstream ss;
        ss << "SELECT 1 FROM " << table << " WHERE " << k << '=';

        if (typeid(v) == typeid(const char*) || typeid(v) == typeid(char*))
            ss << '\'' << v << '\'';
        else
            ss << v;

        ss << " LIMIT 1;";

        return execQuery(ss.str().c_str());
    }

    template<typename vT>
    bool findByKV(const char* table, const char* k, vT v, CRCJson& json)
    {//sql = "SELECT * FROM table WHERE k=v;"
        std::stringstream ss;
        ss << "SELECT * FROM " << table << " WHERE " << k << '=';
        //
        if (typeid(v) == typeid(const char*) || typeid(v) == typeid(char*))
            ss << '\'' << v << '\'';
        else
            ss << v;
        //
        return execQuery(ss.str().c_str(), json);
    }

    template<typename vT>
    bool findByKV(const char* sk, const char* table, const char* k, vT v, CRCJson& json)
    {//sql = "SELECT * FROM table WHERE k=v;"
        std::stringstream ss;
        ss << "SELECT "<<sk<<" FROM " << table << " WHERE " << k << '=';
        //
        if (typeid(v) == typeid(const char*) || typeid(v) == typeid(char*))
            ss << '\'' << v << '\'';
        else
            ss << v;
        //
        return execQuery(ss.str().c_str(), json);
    }

    template<typename vT, typename v2T>
    bool findByKV2(const char* table, const char* k, vT v, const char* k2, v2T v2, CRCJson& json)
    {//sql = "SELECT * FROM table WHERE k=v and k2=v2;"
        std::stringstream ss;
        ss << "SELECT * FROM " << table << " WHERE " << k << '=';
        //
        if (typeid(v) == typeid(const char*) || typeid(v) == typeid(char*))
            ss << '\'' << v << '\'';
        else
            ss << v;
        //
        ss << " AND " << k2 << '=';
        if (typeid(v2) == typeid(const char*) || typeid(v2) == typeid(char*))
            ss << '\'' << v2 << '\'';
        else
            ss << v2;
        //
        return execQuery(ss.str().c_str(), json);
    }

    template<typename vT, typename uvT>
    int updateByKV(const char* table, const char* k, vT v, const char* uk, uvT uv)
    {//sql = "UPDATE table SET uk=uv WHERE k='v';"
        std::stringstream ss;
        ss << "UPDATE " << table << " SET " << uk << '=';
        //
        if (typeid(uv) == typeid(const char*) || typeid(uv) == typeid(char*))
            ss << '\'' << uv << '\'';
        else
            ss << uv;
        //
        ss <<" WHERE "<<k<<'=';
        //
        if (typeid(v) == typeid(const char*) || typeid(v) == typeid(char*))
            ss << '\'' << v << '\'';
        else
            ss << v;
        //
        return execDML(ss.str().c_str());
    }

    template<typename vT, typename v2T, typename uvT>
    int updateByKV2(const char* table, const char* k, vT v, const char* k2, v2T v2, const char* uk, uvT uv)
    {//sql = "UPDATE table SET uk=uv WHERE k='v' AND k2='v2';"
        std::stringstream ss;
        ss << "UPDATE " << table << " SET " << uk << '=';
        //
        if (typeid(uv) == typeid(const char*) || typeid(uv) == typeid(char*))
            ss << '\'' << uv << '\'';
        else
            ss << uv;
        //
        ss << " WHERE " << k << '=';
        //
        if (typeid(v) == typeid(const char*) || typeid(v) == typeid(char*))
            ss << '\'' << v << '\'';
        else
            ss << v;
        //
        ss << " AND " << k2 << '=';
        if (typeid(v2) == typeid(const char*) || typeid(v2) == typeid(char*))
            ss << '\'' << v2 << '\'';
        else
            ss << v2;
        //
        return execDML(ss.str().c_str());
    }

    template<typename vT>
    int deleteByKV(const char* table, const char* k, vT v)
    {//sql = "DELETE FROM table WHERE k=v;"
        std::stringstream ss;
        ss << "DELETE FROM " << table << " WHERE " << k << '=';
        //
        if (typeid(v) == typeid(const char*) || typeid(v) == typeid(char*))
            ss << '\'' << v << '\'';
        else
            ss << v;
        //
        return execDML(ss.str().c_str());
    }

    template<typename vT, typename v2T>
    int deleteByKV2(const char* table, const char* k, vT v, const char* k2, v2T v2)
    {//sql = "DELETE FROM table WHERE k=v and k2=v2;"
        std::stringstream ss;
        ss << "DELETE FROM " << table << " WHERE " << k << '=';
        //
        if (typeid(v) == typeid(const char*) || typeid(v) == typeid(char*))
            ss << '\'' << v << '\'';
        else
            ss << v;
        //
        ss << " AND " << k2 << '=';
        if (typeid(v2) == typeid(const char*) || typeid(v2) == typeid(char*))
            ss << '\'' << v2 << '\'';
        else
            ss << v2;
        //
        return execDML(ss.str().c_str());
    }    
};

#endif //!_CRC_DB_MANAGER_H_