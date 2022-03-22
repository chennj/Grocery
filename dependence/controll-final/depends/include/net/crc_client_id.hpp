#ifndef _CRC_CLIENT_ID_HPP_
#define _CRC_CLIENT_ID_HPP_


#define MSG_TYPE_REQ                1
#define MSG_TYPE_RESP               2
#define MSG_TYPE_PUSH               3
#define MSG_TYPE_BROADCAST          4
#define MSG_TYPE_PUSH_S             5

#define STATE_CODE_OK               0
#define STATE_CODE_ERROR            1
#define STATE_CODE_TIMEOUT          2
#define STATE_CODE_UNDEFINE_CMD     3
#define STATE_CODE_SERVER_BUSY      4
#define STATE_CODE_SERVER_OFF       5
#define STATE_CODE_PROCESSING       6

class ClientId
{
    static const int m_link_id_mask = 100000;
public:
    static int set_link_id(int link_id, int client_id)
    {
        return ((link_id * m_link_id_mask) + client_id);
    }

    static int get_link_id(int id)
    {
        if (id >= m_link_id_mask)
        {
            return id / m_link_id_mask;
        }
        return id;
    }

    static int get_client_id(int id)
    {
        if (id >= m_link_id_mask)
        {
            return id % m_link_id_mask;
        }
        return id;
    }
};

#endif //!_CRC_CLIENT_ID_HPP_