#ifndef _CRC_POOL_CONN_H_
#define _CRC_POOL_CONN_H_

#include "../../include/core/crc_log.h"

#include <list>
#include <condition_variable>
#include <mutex>
#include <chrono>

template<class T>
class CRCConnPool
{
protected:
    std::list<T*> _FreeList;
    std::condition_variable _cond;
    std::mutex _mutex;
    
public: 
    virtual ~CRCConnPool()
    {
        for(std::list<T*>::iterator it = _FreeList.begin(); it!=_FreeList.end(); it++){
            T* pConn = *it;
            delete pConn;
        }
        _FreeList.clear();
    }
public:
    virtual const char* Name() = 0;

    virtual int GetCurConnCnt() = 0;

    virtual int GetMinConnCnt() = 0;

    virtual int GetMaxConnCnt() = 0;

    virtual void IncreaseCurConnCnt() = 0;

    //获取连接
    virtual T* GetConn()
    {
        std::unique_lock<std::mutex> lock(_mutex);

        while(_FreeList.empty()){
            if (GetCurConnCnt() >= GetMaxConnCnt())
            {
                //GetCurConnCnt()非空闲的连接数
                if (_cond.wait_for(lock,std::chrono::seconds(1)==std::cv_status::timeout){
                    CRCLog::CRCLog_Warring("get connection timeout");
                }
            }
            else
            {
                //新建连接
                T* pConn = new T(this);
                int ret = pConn->Init();
                if (ret){
                    CRCLog::Error("new connection failed");
                    delete pConn;
                    return NULL;
                }else{
                    _FreeList.push_back(pConn);
                    IncreaseCurConnCnt();
                }
            }
            
        }//!while

        //获取连接
        T* pConn = _FreeList.front();
        //吐出连接，从空闲队列冲删除
        _FreeList.pop_front();
        return pConn;
    }

    //释放连接
    virtual void ReleaseConn(T * pConn)
    {
        std::unique_lock<std::mutex> lock(_mutex);

        std::list<T*>::iterator it = _FreeList.begin();
        for(;it != _FreeList.end(); it++){           
            if (*it == pConn){break;}
        }

        if (it == _FreeList.end()){
            _FreeList.push_back(pConn);
        }

        _cond.notify_all();
    }

    //初始化连接池
    virtual int Init()
    {
        //建立最小的固定连接数量
        for(int i=0; i<GetCurConnCnt(); i++)
        {
            T* pConn = new T(this);
            int ret = pConn->Init();
            if (ret){
                CRCLog::Error("new connection failed");
                delete pConn;
                return ret;
            } else {
                _FreeList.push_back(pConn);
            }
        }
        return 0;
    }
};

#endif