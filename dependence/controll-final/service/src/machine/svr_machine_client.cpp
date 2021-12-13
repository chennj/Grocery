#include "svr_machine_client.h"
#include "crc_config.h"

MachineClient::MachineClient()
{
    _bCheckMsgID = CRCConfig::Instance().hasKey("-checkMsgID");
}

void 
MachineClient::OnNetMsg(CRCDataHeader* header)
{
    _bSend = false;
    switch (header->cmd)
    {
    case CMD_EQPT_MOVE_RESULT:
    {
        CRCLog_Info("子服务 接收到来自虚拟服务器的返回：CMD_MOVE_RESULT");
        std::lock_guard<std::mutex> lock(_mutexRecvMsg);
        _machineRecvMessages.push(header);
    }
    break;
    case CMD_LOGIN_RESULT:
    {
        CRCLoginR* login = (CRCLoginR*)header;
        if (_bCheckMsgID)
        {
            if (login->msgId != _nRecvMsgID)
            {//当前消息ID和本地收消息次数不匹配
                CRCLog_Error("OnNetMsg socket<%d> msgID<%d> _nRecvMsgID<%d> %d", _pClient->sockfd(), login->msgId, _nRecvMsgID, login->msgId - _nRecvMsgID);
            }
            ++_nRecvMsgID;
        }
        CRCLog_Info("<socket=%d> recv msgType：CMD_LOGIN_RESULT", (int)_pClient->sockfd());
    }
    break;
    case CMD_LOGOUT_RESULT:
    {
        CRCLogoutR* logout = (CRCLogoutR*)header;
        //CRCLog_Info("<socket=%d> recv msgType：CMD_LOGOUT_RESULT", (int)_pClient->sockfd());
    }
    break;
    case CMD_NEW_USER_JOIN:
    {
        CRCNewUserJoin* userJoin = (CRCNewUserJoin*)header;
        //CRCLog_Info("<socket=%d> recv msgType：CMD_NEW_USER_JOIN", (int)_pClient->sockfd());
    }
    break;
    case CMD_ERROR:
    {
        CRCLog_Info("<socket=%d> recv msgType：CMD_ERROR", (int)_pClient->sockfd());
    }
    break;
    default:
    {
        CRCLog_Info("error, <socket=%d> recv undefine msgType", (int)_pClient->sockfd());
    }
    }
}

bool 
MachineClient::checkSend(time_t dt)
{
    _tRestTime += dt;
    //每经过nSendSleep毫秒
    if (_tRestTime >= _nSendSleep)
    {
        //重置计时
        _tRestTime -= _nSendSleep;
        //重置发送计数
        _nSendCount = _nMsg;
    }
    return _nSendCount > 0;
}

void 
MachineClient::debug(int nSendSleep, int nMsg)
{
    _nSendSleep = nSendSleep;
    _nMsg = nMsg;
}

void
MachineClient::Loop()
{
    _threadLoop.Start(
        nullptr, 
        [this](CRCThread* pThread) {
			while (pThread->isRun())
            {
                if (!OnRun(0)){
                    Close();
                    Connect(_ip, _port);
                    CRCThread::Sleep(1000);
                } else {
                    CRCThread::Sleep(0);
                }
            }
		}
    );
}

void 
MachineClient::PushSendMessage(SOCKET srcSock, CRCDataHeader* header)
{
    std::lock_guard<std::mutex> lock(_mutexSendMsg);
    header->sessionId = srcSock;
    _machineSendMessages.push(header);
}

void 
MachineClient::Send2Stm32Loop()
{
    _threadSendCmd.Start(
        nullptr, 
        [this](CRCThread* pThread) {
			while (pThread->isRun())
            {
                if (!_machineSendMessages.empty() && isRun())
                {
                    std::lock_guard<std::mutex> lock(_mutexSendMsg);
                    CRCDataHeader* header = _machineSendMessages.front();
                    _machineSendMessages.pop();
                    switch(header->cmd)
                    {
                        case CMD_EQPT_MOVE:{
                            CRCEqptMove* mmove = (CRCEqptMove*)header;
                            SendData(mmove);
                            break;
                        }
                    }
                } else {
                    CRCThread::Sleep(10);
                }
            }
		},
        nullptr
    );
}

CRCDataHeader* 
MachineClient::popRecvMsg()
{
    if (!_machineRecvMessages.empty()){
        std::lock_guard<std::mutex> lock(_mutexRecvMsg);
        CRCDataHeader* header = _machineRecvMessages.front();
        _machineRecvMessages.pop();
        return header;
    }
    return nullptr;
}