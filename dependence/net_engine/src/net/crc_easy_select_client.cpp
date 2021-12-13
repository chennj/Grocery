#include "crc_easy_select_client.h"

CRCEasySelectClient::CRCEasySelectClient()
{
    _fdRead.create(1);
    _fdWrite.create(1);
}

bool 
CRCEasySelectClient::OnRun(int microseconds)
{
    if (isRun())
    {
        SOCKET _sock = _pClient->sockfd();

        _fdRead.zero();
        _fdRead.add(_sock);

        _fdWrite.zero();

        timeval t = { 0,microseconds };
        int ret = 0;
        if (_pClient->needWrite())
        {
            _fdWrite.add(_sock);
            ret = select(_sock + 1, _fdRead.fdset(), _fdWrite.fdset(), nullptr, &t);
        }else {
            ret = select(_sock + 1, _fdRead.fdset(), nullptr, nullptr, &t);
        }

        if (ret < 0)
        {
            CRCLog_Error("CRCSelectClient.OnRun.wait clientId<%d> sockfd<%d>", _pClient->id, (int)_sock);

            Close();
            return false;
        }

        if (_fdRead.has(_sock))
        {
            if (SOCKET_ERROR == RecvData())
            {
                CRCLog_Error("<socket=%d>OnRun.select RecvData exit", (int)_sock);
                Close();
                return false;
            }
        }

        if (_fdWrite.has(_sock))
        {
            if (SOCKET_ERROR == _pClient->SendDataReal())
            {
                CRCLog_Error("<socket=%d>OnRun.select SendDataReal exit", (int)_sock);
                Close();
                return false;
            }
        }
        return true;
    }
    return false;
}