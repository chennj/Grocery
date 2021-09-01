#include "../../include/mysql/crc_mysql_preparestatement.h"
#include "../../include/core/crc_log.h"

CRCPrepareStatement::CRCPrepareStatement()
{
    _stmt = NULL;
	_param_bind = NULL;
	_param_cnt = 0;
}


CRCPrepareStatement::~CRCPrepareStatement()
{
    if (_stmt)
	{
		mysql_stmt_close(_stmt);
		_stmt = NULL;
	}
    if (_param_bind){
        delete[] _param_bind;
        _param_bind = NULL;
    }
}

bool 
CRCPrepareStatement::Init(MYSQL* mysql, std::string & sql)
{
    mysql_ping(mysql);

    _stmt = mysql_stmt_init(mysql);
    if (!_stmt){
        CRCLog::Error("CRCPrepareStatement::Init mysql_stmt_init failed!");
        return false;
    }

    if (mysql_stmt_prepare(_stmt, sql.c_str(), sql.size())){
        CRCLog::Error("CRCPrepareStatement::Init mysql_stmt_prepare failed!\n%s", mysql_stmt_error(_stmt));
        return false;
    }

    _param_cnt = mysql_stmt_param_count(_stmt);
    if (_param_cnt > 0){
        _param_bind = new MYSQL_BIND[_param_cnt];
        if (!_param_bind){
            CRCLog::Error("CRCPrepareStatement::Init new failed");
            return false;
        }
        memset(_param_bind, 0, sizeof(MYSQL_BIND) * _param_cnt);
    }

    return true;
}

bool
CRCPrepareStatement::Update()
{
    if (!_stmt){
        CRCLog::Error("CRCPrepareStatement::Update _stmt is null");
        return false;
    }

    if (mysql_stmt_bind_param(_stmt, _param_bind)){
        CRCLog::Error("CRCPrepareStatement::Update mysql_stmt_bind_param failed\n%s",mysql_stmt_error(_stmt));
        return false;
    }

    if (int i = mysql_stmt_execute(_stmt)){
        CRCLog::Error("CRCPrepareStatement::Update mysql_stmt_execute failed:%d\n%s",i,mysql_stmt_error(_stmt));
        return false;
    }

    if (mysql_stmt_affected_rows(_stmt) == 0){
        CRCLog::Error("CRCPrepareStatement::Update have no effect");
        return false;
    }

    return true;
}

uint32_t
CRCPrepareStatement::GetInsertId()
{
    return mysql_stmt_insert_id(_stmt);
}

void
CRCPrepareStatement::SetParam(uint32_t index, int & value)
{
    if (index > _param_cnt){
        CRCLog::Error("CRCPrepareStatement::SetParam index too large: %d",index);
        return;
    }

    _param_bind[index].buffer_type      = MYSQL_TYPE_LONG;
    _param_bind[index].buffer           = &value;
}

void
CRCPrepareStatement::SetParam(uint32_t index, uint8_t & value)
{
	if (index > _param_cnt)
	{
        CRCLog::Error("CRCPrepareStatement::SetParam index too large: %d",index);
		return;
	}
	_param_bind[index].buffer_type = MYSQL_TYPE_TINY;
	_param_bind[index].buffer = &value;
}

void
CRCPrepareStatement::SetParam(uint32_t index, uint32_t & value)
{
    if (index > _param_cnt){
        CRCLog::Error("CRCPrepareStatement::SetParam index too large: %d",index);
        return;
    }

    _param_bind[index].buffer_type      = MYSQL_TYPE_LONG;
    _param_bind[index].buffer           = &value;
}

void
CRCPrepareStatement::SetParam(uint32_t index, std::string & value)
{
    if (index > _param_cnt){
        CRCLog::Error("CRCPrepareStatement::SetParam index too large: %d",index);
        return;
    }

    _param_bind[index].buffer_type      = MYSQL_TYPE_STRING;
    _param_bind[index].buffer           = (char*)value.c_str();
    _param_bind[index].buffer_length    = value.size();
}

void
CRCPrepareStatement::SetParam(uint32_t index, const std::string & value)
{
    if (index > _param_cnt){
        CRCLog::Error("CRCPrepareStatement::SetParam index too large: %d",index);
        return;
    }

    _param_bind[index].buffer_type      = MYSQL_TYPE_STRING;
    _param_bind[index].buffer           = (char*)value.c_str();
    _param_bind[index].buffer_length    = value.size();
}
