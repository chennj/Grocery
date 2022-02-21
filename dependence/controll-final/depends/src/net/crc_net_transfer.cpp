#include "crc_net_transfer.h"
#include <algorithm>

void 
CRCNetTransfer::Listeners::add(CRCNetClientS* client)
{
    auto itr = std::find(m_listeners.begin(), m_listeners.end(), client);
    if (itr == m_listeners.end())
    {
        m_listeners.push_back(client);
    }
}

void 
CRCNetTransfer::Listeners::del(CRCNetClientS* client)
{
    auto itr = std::find(m_listeners.begin(), m_listeners.end(), client);
    if (itr != m_listeners.end())
    {
        m_listeners.erase(itr);
    }
}

CRCNetClientS* 
CRCNetTransfer::Listeners::get()
{
    auto size = m_listeners.size();

    if (size == 0)
        return nullptr;

    if (size == 1)
        return m_listeners[0];

    if (index >= size)
        index = 0;

    return m_listeners[index++];
}

void 
CRCNetTransfer::Listeners::on_broadcast_do(const char* pData, int len)
{
    size_t length = m_listeners.size();
    for (size_t i = 0; i < length; i++)
    {
        m_listeners[i]->writeText(pData, len);
    }    
}

void 
CRCNetTransfer::add(std::string cmd, CRCNetClientS* client)
{
    auto itr = m_msg_listeners.find(cmd);
    if (itr != m_msg_listeners.end())
    {
        itr->second.add(client);
    }
    else {
        Listeners a;
        a.add(client);
        m_msg_listeners[cmd] = a;
    }
}

void 
CRCNetTransfer::del(CRCNetClientS* client)
{
    for (auto& itr: m_msg_listeners)
    {
        itr.second.del(client);
    }
}

void 
CRCNetTransfer::del(std::string cmd, CRCNetClientS* client)
{
    auto itr = m_msg_listeners.find(cmd);
    if (itr != m_msg_listeners.end())
    {
        itr->second.del(client);
    }
}

int 
CRCNetTransfer::on_net_msg_do(std::string& cmd, std::string& data)
{
    auto itr = m_msg_listeners.find(cmd);
    if (itr == m_msg_listeners.end())
        return STATE_CODE_UNDEFINE_CMD;

    auto s = itr->second.get();
    if (s)
    {
        if (SOCKET_ERROR == s->writeText(data.c_str(), data.length())){
            return STATE_CODE_SERVER_BUSY;
        }
        return STATE_CODE_OK;
    }

    return STATE_CODE_SERVER_OFF;
}

int 
CRCNetTransfer::on_broadcast_do(const std::string& cmd, const std::string& data)
{
    auto itr = m_msg_listeners.find(cmd);
    if (itr == m_msg_listeners.end())
        return STATE_CODE_UNDEFINE_CMD;

    itr->second.on_broadcast_do(data.c_str(), data.length());
    return STATE_CODE_OK;    
}