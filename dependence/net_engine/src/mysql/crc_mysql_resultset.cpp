#include "../../include/mysql/crc_mysql_resultset.h"
#include "../../include/mysql/crc_mysql_pool_conn.h"

CRCResultSet::CRCResultSet(MYSQL_RES* res)
{
	_res = res;

	int num_fields = mysql_num_fields(_res);
	MYSQL_FIELD *fileds = mysql_fetch_fields(_res);
	int i;
	for ( i =0;i< num_fields;i++)
	{
		_key_map.insert(std::make_pair(fileds[i].name, i));
	}
}

CRCResultSet::~CRCResultSet()
{
	if (_res)
	{
		mysql_free_result(_res);
		_res = NULL;
	}
}

bool 
CRCResultSet::Next()
{
	_row = mysql_fetch_row(_res);
	if (_row)
	{
		return true;
	}
	return false;
}

int 
CRCResultSet::GetInt(const char *key)
{
	int idx = _GetIndex(key);
	if (idx != -1)
	{
		return atoi(_row[idx]);
	}
	return -1;
}

char * 
CRCResultSet::GetString(const char* key)
{
	int idx = _GetIndex(key);
	if (idx != -1)
	{
		return _row[idx];
	}
	return NULL;
}

int 
CRCResultSet::_GetIndex(const char* key)
{
	std::map<std::string, int>::iterator it = _key_map.find(key);
	if (it != _key_map.end())
	{
		return it->second;
	}
	return -1;
}