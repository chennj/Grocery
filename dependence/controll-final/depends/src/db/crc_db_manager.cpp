#include "crc_db_manager.h"

bool 
CRCDBManager::open(const char* db_name)
{
    m_db_name = db_name;
    try
    {
        m_db.open(db_name);
        return true;
    }
    catch (CppSQLite3Exception& e)
    {
        CRCLog_Error("DBManager::open(%s) error: %s", db_name, e.errorMessage());
    }
    return false;    
}

bool 
CRCDBManager::close()
{
    try
    {
        m_db.close();
        return true;
    }
    catch (CppSQLite3Exception& e)
    {
        CRCLog_Error("DBManager::close(%s) error: %s", m_db_name.c_str(), e.errorMessage());
    }
    return false;    
}

void 
CRCDBManager::begin()
{
    execDML("begin;");
}

void 
CRCDBManager::commit()
{
    execDML("commit;");
}

void 
CRCDBManager::run()
{
    if (m_timestamp.getElapsedSecond() > 10.0)
    {
        commit();
        m_timestamp.update();
        begin();
    }   
}

bool 
CRCDBManager::tableExists(const char* szTable)
{
    try
    {
        return m_db.tableExists(szTable);
    }
    catch (CppSQLite3Exception& e)
    {
        CRCLog_Error("DBManager::tableExists(%s.%s) error: %s", m_db_name.c_str(), szTable, e.errorMessage());
    }
    return false;    
}

bool 
CRCDBManager::query2json(CppSQLite3Query& query, CRCJson& json)
{
    try 
    {
        while (!query.eof())
        {
            CRCJson row;
            int num = query.numFields();
            for (int n = 0; n < num; n++)
            {
                auto k = query.fieldName(n);
                auto kType = query.fieldDataType(n);
                if (SQLITE_INTEGER == kType)
                {
                    int64 v = query.getInt64Field(k, 0);
                    row.Add(k, v);
                }
                else if (SQLITE_TEXT == kType)
                {
                    auto v = query.getStringField(k);
                    row.Add(k, v);
                }
                else if (SQLITE_FLOAT == kType)
                {
                    auto v = query.getFloatField(k);
                    row.Add(k, v);
                }
                else if (SQLITE_BLOB == kType)
                {
                    //int nLen = 0;
                    //auto v = query.getBlobField(k, nLen);
                    //将数据拷贝到由我们控制的BLOB数据内存中管理
                    //row.Add新的BLOB数据地址
                    //row.Add(k, (uint64)v);
                }
                else if (SQLITE_NULL == kType)
                {
                    //row.Add(k, "NULL");
                }
            }
            json.Add(row);
            query.nextRow();
        }
    }
    catch (CppSQLite3Exception& e)
    {
        CRCLog_Error("DBManager::query2json(%s) error: %s", m_db_name.c_str(), e.errorMessage());
        return false;
    }
    return true;
}

bool 
CRCDBManager::execQuery(const char* sql, CRCJson& json)
{
    CppSQLite3Query query;
    try
    {
        query = m_db.execQuery(sql);
    }
    catch (CppSQLite3Exception& e)
    {
        CRCLog_Error("DBManager::execQuery(%s) sql:%s error: %s", m_db_name.c_str(), sql, e.errorMessage());
        return false;
    }
    return query2json(query, json);    
}

bool 
CRCDBManager::execQuery(const char* sql)
{
    CppSQLite3Query query;
    try
    {
        query = m_db.execQuery(sql);
        return !query.eof();
    }
    catch (CppSQLite3Exception& e)
    {
        CRCLog_Error("DBManager::execQuery(%s) sql:%s error: %s", m_db_name.c_str(), sql, e.errorMessage());
    }
    return false;    
}

int  
CRCDBManager::execDML(const char* sql)
{
    try
    {
        return m_db.execDML(sql);
    }
    catch (CppSQLite3Exception& e)
    {
        CRCLog_Error("DBManager::execDML(%s) sql:%s error: %s", m_db_name.c_str(), sql, e.errorMessage());
    }
    return -1;    
}