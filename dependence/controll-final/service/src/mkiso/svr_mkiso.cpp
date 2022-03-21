#include <regex>

#include "svr_mkiso.h"

void 
MkisoServer::Init()
{
    m_csGate.set_groupId("0001");

    m_csGate.connect("csCtrl","ws://127.0.0.1:4567", 1024 * 1024 * 10, 1024 * 1024 * 10);

    m_csGate.reg_msg_call("onopen", std::bind(&MkisoServer::onopen_csGate, this, std::placeholders::_1, std::placeholders::_2));

    m_csGate.reg_msg_call("cs_package_iso_file",std::bind(&MkisoServer::cs_package_iso_file, this, std::placeholders::_1, std::placeholders::_2));
    
}

void 
MkisoServer::Run()
{
    m_csGate.run(1);
}

void 
MkisoServer::Close()
{
    m_csGate.close();
}

void 
MkisoServer::onopen_csGate(CRCNetClientC* client, CRCJson& msg)
{
    CRCJson json;
    json.Add("type",    "MkisoServer");
    json.Add("name",    "MkisoServer001");
    json.Add("sskey",   "ssmm00@123456");
    json.AddEmptySubArray("apis");
    json["apis"].Add("cs_package_iso_file");

    client->request("ss_reg_api", json, [](CRCNetClientC* client, CRCJson& msg) {
        CRCJson cj; 
        msg.Get("data", cj);
        CRCLog_Info("MkisoServer::ss_reg_api return: %s", cj.ToString().c_str());
    });
}

void 
MkisoServer::cs_package_iso_file(CRCNetClientC* client, CRCJson& msg)
{
    CRCLog_Info("MkisoServer::cs_package_iso_file entered");

    int i=1;
    while(i++<=10){
        CRCThread::Sleep(1000);
        CRCLog::Info("-- processing %d%",i*10);
        CRCJson processing;
        processing.Add("process",(i*10)+"%");
        m_csGate.push(client->get_clientId(),msg("cmd"), processing);
    }

    CRCJson ret;
    ret.Add("data", "cs_package_iso_file successs.");
    client->response(msg, ret);
}