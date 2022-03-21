#include "controller.h"

void
NlController::Init()
{
    m_thread_pool.init(4);
    m_thread_pool.start();

    m_csGate.set_groupId("0000");

    auto csGateUrl = CRCConfig::Instance().getStr("csGateUrl", "ws://127.0.0.1:4567");
    m_csGate.connect("csGate", csGateUrl, 1024 * 1024 * 10, 1024 * 1024 * 10);

    m_csGate.reg_msg_call("onopen", std::bind(&NlController::onopen_csGate, this, std::placeholders::_1, std::placeholders::_2));

    m_csGate.reg_msg_call("ctl_nl_burn_iso_file", std::bind(&NlController::ctl_nl_burn_iso_file, this, std::placeholders::_1, std::placeholders::_2));
}

void 
NlController::Run()
{
    m_csGate.run(0);
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

    client->request("ss_reg_api", json, [](CRCNetClientC* client, CRCJson& msg) {
        CRCJson cj; 
        msg.Get("data", cj);
        CRCLog_Info("NlController::ss_reg_api return: %s", cj.ToString().c_str());
    });
}


int 
NlController::ctl_nl_burn_iso_file(CRCNetClientC* client, CRCJson& msg)
{
    CRCLog_Info("NlController::ctl_nl_burn_iso_file entered");
    
    m_thread_pool.exec(
        [this](CRCNetClientC* client, CRCJson copy_msg)
        {
            int state = STATE_CODE_OK;
            std::mutex task_mutex;
            std::condition_variable task_condition;

            CRCJson json;
            json.Add("type",    "NlContorller");
            json.Add("cckey",   "ccmm00@123456");
            json.Add("cdType",  copy_msg("cdType"));

            m_csGate.request("cs_package_iso_file", copy_msg("groupId"), json, [&](CRCNetClientC* client, CRCJson& org_msg) {
                
                if (!org_msg.Get("state", state)) { 
                    CRCLog_Error("-- not found key <state>"); 
                    json.Add("data", "not found key <state>");
                    task_condition.notify_one();
                    return; 
                }

                if (state != STATE_CODE_OK) 
                { 
                    CRCLog_Error("-- <state=%d> msg: %s", state, org_msg("data").c_str()); 
                    json.Add("data", org_msg("data").c_str());
                    task_condition.notify_one();
                    return; 
                }
            });

            if (!wait_for(task_condition, "-- waitting cs_package_iso_file return")){
                json.Add("state", STATE_CODE_ERROR);
                json.Add("data", "waitting package iso file timeout");
                goto EXIT;
            }
            
            if (state != STATE_CODE_OK){
                json.Add("state", state);
                goto EXIT;
            }

            json.Add("data", "package iso file successs.");

            EXIT:
            client->response(copy_msg, json);
            return;
        }, 
        client, 
        msg
    );

    /*
    int state = STATE_CODE_OK;
    std::mutex task_mutex;
    std::condition_variable task_condition;

    CRCJson json;
    json.Add("type",    "NlContorller");
    json.Add("cckey",   "ccmm00@123456");
    json.Add("cdType",  msg("cdType"));

    m_csGate.request("cs_package_iso_file", msg("groupId"), json, [&](CRCNetClientC* client, CRCJson& msg) {
        
        if (!msg.Get("state", state)) { 
            CRCLog_Error("-- not found key <state>"); 
            task_condition.notify_one();
            return; 
        }

        if (state != STATE_CODE_OK) 
        { 
            CRCLog_Error("-- <state=%d> msg: %s", state, msg("data").c_str()); 
            task_condition.notify_one();
            return; 
        }
    });

    if (!wait_for(task_condition, "-- waitting cs_package_iso_file return")){
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
            CRCLog_Error("-- not found key <state>"); 
            return; 
        }

        if (state != STATE_CODE_OK) { 
            CRCLog_Error("-- cs_reg_client error <state=%d> msg: %s", 
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
    return -1;*/
}

bool 
NlController::wait_for(std::condition_variable& condition, const std::string& msg, uint32_t timeout)
{
    CRCLog_Info(msg.c_str());
    std::mutex task_mutex;
    std::unique_lock<std::mutex> lock(task_mutex);
    auto no_timeout = condition.wait_for(lock, std::chrono::milliseconds(timeout));
    if (no_timeout == std::cv_status::timeout){
        CRCLog_Error(msg.c_str());
        return false;
    }
    return true;
}