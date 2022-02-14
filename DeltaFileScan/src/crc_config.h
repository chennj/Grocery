/**
 *
 * author:  chenningjiang
 * desc:    ר�����ڶ�ȡ�������ݣ�
 * 			Ŀǰ���ǵ����ò�����Ҫ��Դ��main������args����
 *
 * */
#ifndef _CRC_CONFIG_H_
#define _CRC_CONFIG_H_

#include <string>
#include <map>
#include "crc_log.h"

class CRCConfig
{
private:
	CRCConfig()
	{

	}

	~CRCConfig()
	{

	}
public:
	static CRCConfig& Instance()
	{
		static  CRCConfig obj;
		return obj;
	}

	void Init(int argc, char* args[])
	{
		_exePath = args[0];
		for (int n = 1; n < argc; n++)
		{
			madeCmd(args[n]);
		}
	}

	void madeCmd(char* cmd)
	{
		//cmdֵ:strIP=127.0.0.1
		char* val = strchr(cmd, '=');
		if (val)
		{	//valֵ:=127.0.0.1
			*val = '\0';
			//cmdֵ:strIP\0
			//valֵ:\0127.0.0.1
			val++;
			//valֵ:127.0.0.1
			_kv[cmd] = val;
			CRCLog_Debug("madeCmd k<%s> v<%s>", cmd, val);
		}
		else {
			_kv[cmd] = "";
			CRCLog_Debug("madeCmd k<%s>", cmd);
		}
	}

	const char* getStr(const char* argName, const char* def)
	{
		auto itr = _kv.find(argName);
		if (itr == _kv.end())
		{
			CRCLog_Error("CRCConfig::getStr not find <%s>", argName);
		}
		else {
			def = itr->second.c_str();
		}
		CRCLog_Info("CRCConfig::getStr %s=%s", argName, def);
		return def;
	}

	int getInt(const char* argName, int def)
	{
		auto itr = _kv.find(argName);
		if (itr == _kv.end())
		{
			CRCLog_Error("CRCConfig::getStr not find <%s>", argName);
		}
		else {
			def = atoi(itr->second.c_str());
		}
		CRCLog_Info("CRCConfig::getInt %s=%d", argName, def);
		return def;
	}

	bool hasKey(const char* key)
	{
		auto itr = _kv.find(key);
		return itr != _kv.end();
	}

private:
	//��ǰ�����·��
	std::string _exePath;
	//�洢�����key-val������
	std::map<std::string, std::string> _kv;

};

#endif // !_CRC_CONFIG_HPP_

