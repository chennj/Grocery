#include "../../include/core/crc_config.h"

CRCConfig::CRCConfig(){}

CRCConfig::~CRCConfig(){}

CRCConfig&
CRCConfig::Instance()
{
    static CRCConfig cf;
    return cf;
}

void
CRCConfig::Init(int argc, char* args[])
{
    _exePath = args[0];
    for (int n = 1; n < argc; n++)
    {
        //CRCLog::Debug("Config Parameters: argc<%d>,args<%s>",n,args[n]);
        madeCmd(args[n]);
    }    
}

void
CRCConfig::madeCmd(char* cmd)
{
    //cmd值:strIP=127.0.0.1
    char* val = strchr(cmd, '=');
    if (val)
    {	//val值:=127.0.0.1
        *val = '\0';
        //cmd值:strIP\0
        //val值:\0127.0.0.1
        val++;
        //val值:127.0.0.1
        _kv[cmd] = val;
        CRCLog_Debug("madeCmd k<%s> v<%s>", cmd, val);
    } else {
        _kv[cmd] = "";
        CRCLog_Debug("madeCmd k<%s>", cmd);
    }    
}

const char*
CRCConfig::getStr(const char* argName, const char* def)
{
    auto itr = _kv.find(argName);
    if (itr == _kv.end())
    {
        CRCLog_Error("CRCConfig::getStr not find <%s>", argName);
    } else {
        def = itr->second.c_str();
    }
    CRCLog_Info("CRCConfig::getStr %s=%s", argName, def);
    return def;    
}

int
CRCConfig::getInt(const char* argName, int def)
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

bool
CRCConfig::hasKey(const char* key)
{
    auto itr = _kv.find(key);
    return itr != _kv.end();    
}