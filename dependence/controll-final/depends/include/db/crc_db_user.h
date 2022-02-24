#ifndef _CRC_DB_USER_H_
#define _CRC_DB_USER_H_

#include "crc_log.h"
#include "crc_db_manager.h"

class CRCDBUser : public CRCDBManager
{
protected:
    int64 m_max_userId = 100000;
public:
    ~CRCDBUser();

    int64 makeId()
    {
        return m_max_userId+1;
    }
protected:
    void load_id();

    bool create_table_user_info();
public:
    void init();

    bool has_username(const std::string& username);

    bool has_nickname(const std::string& nickname);

    int64 add_user(const std::string& username, const std::string& password, const std::string& nickname, int sex);
};

#endif //!_CRC_DB_USER_H_