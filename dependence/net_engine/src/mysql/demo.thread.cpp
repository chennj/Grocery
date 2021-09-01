#include "../../include/mysql/crc_mysql_pool_conn.h"
#include "../../include/mysql/crc_mysql_conn.h"
#include "../../include/mysql/crc_mysql_table.h"
#include <time.h>
#include <unistd.h>
#define MAX_LEN 1024*32

namespace sqlinfo {

        static const char *db_pool_name = "mypool";
        static const char* db_host = "127.0.0.1";
        static const int   db_port = 3306;
        static const char* db_dbname = "test_pool";
        static const char* db_username = "root";
        static const char* db_password = "fantasy";
        static const int   db_minconncnt = 2;
        static const int   db_maxconncnt = 8;

};

using namespace std;
using namespace sqlinfo;

#define DROP_IMUSER_TABLE	"DROP TABLE IF EXISTS IMUser"  


#define CREATE_IMUSER_TABLE "CREATE TABLE IMUser (     \
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT COMMENT '用户id',   \
  `sex` tinyint(1) unsigned NOT NULL DEFAULT '0' COMMENT '1男2女0未知', \
  `name` varchar(32) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '用户名',  \
  `domain` varchar(32) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '拼音',  \
  `nick` varchar(32) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '花名,绰号等', \
  `password` varchar(32) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '密码',    \
  `salt` varchar(4) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '混淆码',   \
  `phone` varchar(11) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '手机号码',   \
  `email` varchar(64) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT 'email',  \
  `company` varchar(64) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '公司名称', \
  `address` varchar(64) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '所在地区', \
  `avatar` varchar(255) COLLATE utf8mb4_bin DEFAULT '' COMMENT '自定义用户头像',    \
  `validateMethod` tinyint(2) unsigned DEFAULT '1' COMMENT '好友验证方式',  \
  `departId` int(11) unsigned NOT NULL DEFAULT '1' COMMENT '所属部门Id',    \
  `status` tinyint(2) unsigned DEFAULT '0' COMMENT '1. 试用期 2. 正式 3. 离职 4.实习',  \
  `created` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '创建时间',   \
  `updated` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '更新时间',   \
  `push_shield_status` tinyint(1) unsigned NOT NULL DEFAULT '0' COMMENT '0关闭勿扰 1开启勿扰',  \
  `sign_info` varchar(128) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '个性签名',  \
  PRIMARY KEY (`id`),   \
  KEY `idx_domain` (`domain`),  \
  KEY `idx_name` (`name`),  \
  KEY `idx_phone` (`phone`) \
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;"   

void testOneConnect() {

	CRCMysqlConnPool *pool = new CRCMysqlConnPool(db_pool_name,db_host,db_port,db_username,db_password,db_dbname,db_minconncnt,db_maxconncnt);
	if (pool->Init())
	{
		return;
	}
	CRCMysqlConn *conn = pool->GetConn();

	if (conn)
	{
		bool ret = conn->ExecuteDrop(DROP_IMUSER_TABLE);
		if (ret)
		{
			printf("drop table ok");
		}
		ret = conn->ExecuteCreate(CREATE_IMUSER_TABLE);
		if (ret )
		{
			printf("create table ok");
		}
		pool->ReleaseConn(conn);
	}
	delete pool;

}

void testCURD() {
	CRCMysqlConnPool *pool = new CRCMysqlConnPool(db_pool_name, db_host, db_port, db_username, db_password, db_dbname, db_minconncnt, db_maxconncnt);
	if (pool->Init())
	{
		return;
	}
	CRCMysqlConn *conn = pool->GetConn();
	if (conn)
	{
		bool ret = conn->ExecuteDrop(DROP_IMUSER_TABLE);
		if (ret)
		{
			CRCLog::Info("Drop Table ok");
		}
		else
		{
			CRCLog::Error("Drop Table Failed !!!");
			return;
		}
		ret = conn->ExecuteCreate(CREATE_IMUSER_TABLE);
		if (ret)
		{
			CRCLog::Info("create Table ok");
		}
		else
		{
			CRCLog::Error("create Table Failed !!!");
			return;
		}
		UserTable *table = new UserTable(conn);
		if (NULL == table)
		{
			CRCLog::Error("table is null !!!");
			return;
		}
		ret = table->insertUser();
		if (ret)
		{
			CRCLog::Info("Insert User Table ok");
		}
		else
		{
			CRCLog::Error("Insert User Table Failed !!!");
			delete table;
			return;
		}
		ret = table->queryUser(1);
		if (ret)
		{
			CRCLog::Info("query User Table ok");
		}
		else
		{
			CRCLog::Error("query User Table Failed !!!");
			delete table;
			return;
		}
		ret = table->updateInfo(1);
		if (ret)
		{
			CRCLog::Info("update info Table ok \n");
		}
		else
		{
			CRCLog::Error("update info Table Failed !!!");
			delete table;
			return;
		}
		ret = table->queryUser(1);
		if (ret)
		{
			CRCLog::Info("query User 1 Table ok \n");
		}
		else
		{
			CRCLog::Error("query User 1 Table Failed !!!");
			delete table;
			return;
		}
		delete table;
	}
}

int main()
{
	//testOneConnect();
	testCURD();
	return 0;
}