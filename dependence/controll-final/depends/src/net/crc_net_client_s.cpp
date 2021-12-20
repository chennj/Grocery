#include "crc_net_client_s.h"

CRCNetClientS::CRCNetClientS(SOCKET sockfd, int sendSize, int recvSize) 
    :
    CRCClientSWebSocket(sockfd, sendSize, recvSize)
{

}

std::string& 
CRCNetClientS::link_name()
{
    return _link_name;
}

void 
CRCNetClientS::link_name(std::string& str)
{
    _link_name = str;
}

std::string& 
CRCNetClientS::link_type()
{
    return _link_type;
}

void 
CRCNetClientS::link_type(std::string& str)
{
    _link_type = str;
}

bool 
CRCNetClientS::is_ss_link()
{
    return _is_ss_link;
}

void 
CRCNetClientS::is_ss_link(bool b)
{
    _is_ss_link = b;
}

void 
CRCNetClientS::response(int msgId, std::string data)
{
    CRCJson ret;
    ret.Add("msgId", msgId);
    ret.Add("is_resp", true, true);
    ret.Add("time", CRCTime::system_clock_now());
    ret.Add("data", data);

    std::string retStr = ret.ToString();
    this->writeText(retStr.c_str(), retStr.length());
}

void 
CRCNetClientS::response(CRCJson& msg, std::string data)
{
    int msgId = 0;
    if (!msg.Get("msgId", msgId))
    {
        CRCLog_Error("not found key<%s>.", "msgId");
        return;
    }

    CRCJson ret;
    ret.Add("msgId", msgId);
    ret.Add("is_resp", true, true);
    ret.Add("time", CRCTime::system_clock_now());
    ret.Add("data", data);

    std::string retStr = ret.ToString();
    this->writeText(retStr.c_str(), retStr.length());
}

void 
CRCNetClientS::response(CRCJson& msg, CRCJson& ret)
{
    int msgId = 0;
    if (!msg.Get("msgId", msgId))
    {
        CRCLog_Error("not found key<%s>.", "msgId");
        return;
    }

    ret.Add("msgId", msgId);
    ret.Add("is_resp", true, true);
    ret.Add("time", CRCTime::system_clock_now());

    std::string retStr = ret.ToString();
    this->writeText(retStr.c_str(), retStr.length());
}