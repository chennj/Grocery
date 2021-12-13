#include "crc_net_transfer.h"
#include <algorithm>

void 
CRCNetTransfer::Listeners::add(CRCNetClientS* client)
{
    auto itr = std::find(_listeners.begin(), _listeners.end(), client);
    if (itr == _listeners.end())
    {
        _listeners.push_back(client);
    }
}

void 
CRCNetTransfer::Listeners::del(CRCNetClientS* client)
{
    auto itr = std::find(_listeners.begin(), _listeners.end(), client);
    if (itr != _listeners.end())
    {
        _listeners.erase(itr);
    }
}

CRCNetClientS* 
CRCNetTransfer::Listeners::get()
{
    auto size = _listeners.size();

    if (size == 0)
        return nullptr;

    if (size == 1)
        return _listeners[0];

    if (index >= size)
        index = 0;

    return _listeners[index++];
}

void 
CRCNetTransfer::add(std::string cmd, CRCNetClientS* client)
{
    auto itr = _msg_listeners.find(cmd);
    if (itr != _msg_listeners.end())
    {
        itr->second.add(client);
    }
    else {
        Listeners a;
        a.add(client);
        _msg_listeners[cmd] = a;
    }
}

void 
CRCNetTransfer::del(CRCNetClientS* client)
{
    for (auto& itr: _msg_listeners)
    {
        itr.second.del(client);
    }
}

void 
CRCNetTransfer::del(std::string cmd, CRCNetClientS* client)
{
    auto itr = _msg_listeners.find(cmd);
    if (itr != _msg_listeners.end())
    {
        itr->second.del(client);
    }
}

bool 
CRCNetTransfer::on_net_msg_do(std::string& cmd, std::string& data)
{
    auto itr = _msg_listeners.find(cmd);
    if (itr == _msg_listeners.end())
        return false;

    auto s = itr->second.get();
    if (s)
    {
        s->writeText(data.c_str(), data.length());
        return true;
    }

    return false;
}