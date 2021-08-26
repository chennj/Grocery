#ifndef _CRC_CONFIG_H_
#define _CRC_CONFIG_H_
/*
    file: crc_config.h
	专门用于读取配置数据
	目前我们的配置参数主要来源于main函数的args传入
*/
#include <string>
#include <map>
#include "crc_common.h"

class CRCConfig
{
private:
	//当前程序的路径
	std::string _exePath;
	//存储传入的key-val型数据
	std::map<std::string, std::string> _kv;

private:
    CRCConfig();
    ~CRCConfig();

public:
	static CRCConfig& Instance();

    void Init(int argc, char* args[]);
    void madeCmd(char* cmd);
    const char* getStr(const char* argName, const char* def);
    int getInt(const char* argName, int def);
    bool hasKey(const char* key);
};

#endif