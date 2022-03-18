#include "controller.h"

void
NlController::Init()
{
    auto csGateUrl = CRCConfig::Instance().getStr("csGateUrl", "ws://127.0.0.1:4567");
    m_csGate.connect("csGate", csGateUrl, 1024 * 1024 * 10, 1024 * 1024 * 10);

    m_csGate.reg_msg_call("onopen", std::bind(&NlController::onopen_csGate, this, std::placeholders::_1, std::placeholders::_2));

}

void 
NlController::Run()
{
    m_csGate.run(0);
    //m_taskTimer.OnRun();
}

void 
NlController::Close()
{
    m_csGate.close();
}

void 
NlController::onopen_csGate(CRCNetClientC* client, CRCJson& msg)
{
    CRCJson json;
    json.Add("type",  "Normal_Controller");
    json.Add("name",  "Normal_Controller");
    json.Add("sskey", "ssmm00@123456");
    json.AddEmptySubArray("apis");
    json["apis"].Add("ctl_nl_burn_iso_file");
    json["apis"].Add("ctl_nl_register");
    json["apis"].Add("ctl_nl_change_pw");
    json["apis"].Add("ctl_nl_login_by_token");
    json["apis"].Add("ctl_nl_client_exit");
    json["apis"].Add("ctl_nl_user_exit");

    client->request("ss_reg_server", json, [](CRCNetClientC* client, CRCJson& msg) {
        CRCLog_Info(msg("data").c_str());
    });
}


int 
NlController::ctl_msg_burn_iso_file(CRCNetClientC* client, CRCJson& msg)
{
    int state = STATE_CODE_OK;
    std::mutex task_mutex;
    std::condition_variable task_condition;

    CRCJson json;
    json.Add("type",    "NlContorller");
    json.Add("cckey",   "ccmm00@123456");
    json.Add("cdType",  msg("cdType"));

    m_csGate.request("cs_package_iso_file", msg("groupId"), json, [&](CRCNetClientC* client, CRCJson& msg) {
        
        if (!msg.Get("state", state)) { 
            CRCLog_Error("not found key <state>"); 
            task_condition.notify_one();
            return; 
        }

        if (state != STATE_CODE_OK) { 
            CRCLog_Error("cs_reg_client error <state=%d> msg: %s", state, msg("data").c_str()); 
            task_condition.notify_one();
            return; 
        }
    });

    if (!wait_for(task_condition, "waitting cs_package_iso_file return")){
        msg.Add("state", STATE_CODE_ERROR);
        msg.Add("data", "waitting package iso file timeout");
        goto EXIT;
    }
    
    if (state != STATE_CODE_OK){
        msg.Add("state", STATE_CODE_ERROR);
        goto EXIT;
    }

    m_csGate.request("cs_burn_iso_file", msg("groupId"), json, [&](CRCNetClientC* client, CRCJson& msg) {
        
        if (!msg.Get("state", state)) { 
            CRCLog_Error("not found key <state>"); 
            return; 
        }

        if (state != STATE_CODE_OK) { 
            CRCLog_Error("cs_reg_client error <state=%d> msg: %s", 
            state, msg("data").c_str()); 
            return; 
        }
    });

    
    if (state != STATE_CODE_OK){
        goto EXIT;
    }


    m_csGate.request("cs_print_iso_file", msg("groupId"), json, [&](CRCNetClientC* client, CRCJson& msg) {
        
        if (!msg.Get("state", state)) { 
            CRCLog_Error("not found key <state>"); 
            return; 
        }

        if (state != STATE_CODE_OK) { 
            CRCLog_Error("cs_reg_client error <state=%d> msg: %s", 
            state, msg("data").c_str()); 
            return; 
        }
    });

    EXIT:
    return -1;
}

bool 
NlController::wait_for(std::condition_variable& condition, const std::string& msg, uint32_t timeout)
{
    std::mutex task_mutex;
    std::unique_lock<std::mutex> lock(task_mutex);
    bool no_timeout = condition.wait_for(lock, std::chrono::milliseconds(timeout));
    if (!no_timeout){
        CRCLog_Error(msg.c_str());
        return false;
    }
    return true;
}