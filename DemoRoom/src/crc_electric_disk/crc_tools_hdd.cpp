//编译命令
//export LD_LIBRARY_PATH=/usr/local/mysql/lib:LD_LIBRARY_PATH
//gcc -g -static -o tools_hdd crc_tools_hdd.cpp -lmysqlclient -I/usr/local/mysql/include -L/usr/local/mysql/lib
#if __LINUX__
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <string.h>
#include <stdbool.h>
#include <vector>
#include <map>
#include <algorithm>
#include <mysql.h>

#define SKIP_LINE_HDD_STATUS_SDA 7

using namespace std;

char sz[512] = {0};


FILE* open_file(const char* filePath)
{
	FILE *f = fopen(filePath, "r");
	return f;
}

bool get_line(FILE* f)
{
	memset(sz, 0, sizeof(sz));
	if (!fgets(sz, 500, f))
		return false;

	return true;
}

vector<string> stringsplit(const string &str, const char *delim)
{
    vector <string> strlist;
    int size = str.size();
    char *input = new char[size];
    strcpy(input, str.c_str());
    char *token = strtok(input, delim);
    while (token != NULL) {
        strlist.push_back(token);
        token = strtok(NULL, delim);
    }
    delete []input;
    return strlist;
}

int stringfind(const string & str, const char* se)
{
    string::size_type position;
    position = str.find(se);
    if (position != str.npos){
        return position;
    } else {
        -1;
    }
}


string& stringtrim(string &s) 
{
    if (s.empty()) 
    {
        return s;
    }
 
    s.erase(0,s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
}

int main(int argc,char* args[])
{
    MYSQL * _mysql;
    string sql_head = "insert into t_hdd_info( \
            `HDD_SN`,               `HDD_sys_SN`,           `HDD_size`,  \
            `HDD_used_size`,        `HDD_pw_status`,        `HDD_used_time`, \
            `HDD_raw_error`,        `HDD_start_stop_count`, `HDD_write_error_rate`, \
            `HDD_CRC_error_count`,  `HDD_calib_count`,      `HDD_locked_status`, \
            `HDD_Key`,              `HDD_bad_block_num`,    `HDD_error_info`, \
            `HDD_online_status`) value";
    int res;
    int i=0;
    int pos=-1;
    int dev_name_index = 0;
    int skip_line = 0;

    string dev_names[] = { \
        "sda","sdb","sdc","sdd","sde","sdf","sdg","sdh","sdi","sdj","sdk","sdl", \
        "sdm","sdn","sdo","sdp","sdq","sdr","sds","sdt","sdu","sdv","sdw","sdx" \
    };
    string cmd;
    string path;
    string sql;
    bool is_skip;
    FILE* f = NULL;    
    map<string, vector<string> > hdd_status_map;

    string HDD_SN;
    string HDD_sys_SN;
    string HDD_size;
    string HDD_used_size;
    string HDD_pw_status;
    string HDD_used_time;
    string HDD_raw_error;
    string HDD_start_stop_count;
    string HDD_write_error_rate;
    string HDD_CRC_error_count;
    string HDD_calib_count;
    string HDD_locked_status;
    string HDD_Key;
    string HDD_bad_block_num;
    string HDD_error_info;
    string HDD_online_status;

    _mysql = mysql_init(NULL);
    if (!_mysql){
        printf("ERROR,mysql init failed!\n");
        return 1;
    } 

    my_bool reconnect = true;

    mysql_options(_mysql, MYSQL_OPT_RECONNECT, &reconnect);
    mysql_options(_mysql, MYSQL_SET_CHARSET_NAME, "utf8mb4");

    if (!mysql_real_connect(
        _mysql, 
        args[1], 
        args[2],
        args[3],
        args[4],
        3306,
        NULL,
        CLIENT_FOUND_ROWS)){
        printf("ERROR,mysql_real_connect is failed! %s", mysql_error(_mysql));
        goto END;
    }

    //检索smartctl信息
    SMARTCTL:

    HDD_SN.clear();
    HDD_sys_SN.clear();
    HDD_size.clear();
    HDD_used_size.clear();
    HDD_pw_status.clear();
    HDD_used_time.clear();
    HDD_raw_error.clear();
    HDD_start_stop_count.clear();
    HDD_write_error_rate.clear();
    HDD_CRC_error_count.clear();
    HDD_calib_count.clear();
    HDD_locked_status.clear();
    HDD_Key.clear();
    HDD_bad_block_num.clear();
    HDD_error_info.clear();
    HDD_online_status.clear();

    hdd_status_map.clear();

    HDD_sys_SN          =  dev_names[dev_name_index];
    HDD_online_status   = "01";
    HDD_pw_status       = "01";
    HDD_locked_status   = "01";

    is_skip = true;

    path = "/jukebox/log/hdd/hdd_status_" + dev_names[dev_name_index] + ".log";
    cmd  = "smartctl -A /dev/" + dev_names[dev_name_index] +" > " + path;

    system(cmd.c_str());

    f = open_file(path.c_str());
	if (!f)
	{
		printf("open file %s failed\n",path.c_str());
		goto HDPARM;
	}

 	while(get_line(f))
	{
        if (strlen(sz) == 0 || sz[0] == '\n'){
            continue;
        }
        if (strstr(sz, "failed")){
            HDD_online_status = "00";
            break;
        }
        if (strncmp(sz, "ID#", 3) != 0 && is_skip){
            continue;
        } else {
            if (is_skip){
                get_line(f);
            }
            is_skip = false;
        }
        string fields = sz;
        printf("fields = %s\n",fields.c_str());
        vector<string> fieldary = stringsplit(fields, " ");
        if (fieldary.size() < 2){continue;}
        hdd_status_map[fieldary[1]] = fieldary;
	};

    fclose(f);
    //---------------------------------------------------------------------
    
    for (map<string, vector<string> >::iterator it = hdd_status_map.begin(); it != hdd_status_map.end(); it++)
    {
        int raw_val_index = it->second.size()-1;
        string field = it->first;
        if (field.compare("Raw_Read_Error_Rate") == 0){
            HDD_raw_error = (it->second)[raw_val_index];
        }
        if (field.compare("Spin_Up_Time") == 0){
            HDD_pw_status = "00";
        }
        if (field.compare("Start_Stop_Count") == 0){
            HDD_start_stop_count = (it->second)[raw_val_index];
        }
        if (field.compare("Power_On_Hours") == 0){
            HDD_used_time = (it->second)[raw_val_index];
        }
        if (field.compare("Calibration_Retry_Count") == 0){
            HDD_calib_count = (it->second)[raw_val_index];
        }
        if (field.compare("Offline_Uncorrectable") == 0){
            HDD_bad_block_num = (it->second)[raw_val_index];
        }
        if (field.compare("UDMA_CRC_Error_Count") == 0){
            HDD_CRC_error_count = (it->second)[raw_val_index];
        }
        if (field.compare("Multi_Zone_Error_Rate") == 0){
            HDD_write_error_rate = (it->second)[raw_val_index];
        }
    }

    //检索hdparm信息
    HDPARM:
    path = "/jukebox/log/hdd/hdd_info_" + dev_names[dev_name_index] + ".log";
    cmd  = "hdparm -I /dev/" + dev_names[dev_name_index] +" > " + path;

    system(cmd.c_str());

    f = open_file(path.c_str());
    if (!f)
	{
		printf("open file %s failed\n",path.c_str());
		goto SQLCMD;
	}

	while(get_line(f))
	{
        if (strlen(sz) == 0 || sz[0] == '\n'){
            continue;
        }
        string fields = sz;
        printf("fields = %s\n",fields.c_str());

        pos = stringfind(fields, "Serial Number:");
        if (pos >= 0){
            HDD_SN = fields.substr(pos+14);
            HDD_SN = stringtrim(HDD_SN);
            continue;
        }

        pos = stringfind(fields, "device size with M = 1024*1024:");
        if (pos >= 0){
            HDD_size = fields.substr(pos+31);
            HDD_size = stringtrim(HDD_size);
            continue;
        }

        pos = stringfind(fields, "not   locked");
        if (pos >= 0){
            HDD_locked_status = "00";
            continue;
        }

        pos = stringfind(fields, "Master password revision code =");
        if (pos >= 0){
            HDD_Key = fields.substr(pos+31);
            HDD_Key = stringtrim(HDD_Key);
            continue;
        }
	};

    fclose(f);
    //---------------------------------------------------------------------

    //更新数据库
    SQLCMD:
    sql = sql_head;
    sql.append("(");
    sql += "'"+HDD_SN+"','"+HDD_sys_SN+"','"+HDD_size+"',";
    sql += "'"+HDD_used_size+"','"+HDD_pw_status+"','"+HDD_used_time+"',";
    sql += "'"+HDD_raw_error+"','"+HDD_start_stop_count+"','"+HDD_write_error_rate+"',";
    sql += "'"+HDD_CRC_error_count+"','"+HDD_calib_count+"','"+HDD_locked_status+"',";
    sql += "'"+HDD_Key+"','"+HDD_bad_block_num+"','"+HDD_error_info+"',";
    sql += "'"+HDD_online_status+"'";
    sql.append(")");

    res = mysql_query(_mysql, sql.c_str());

    if (!res){
        printf("inserted %lu rows\n", (unsigned long)mysql_affected_rows(_mysql));
    } else {
        fprintf(stderr, "insert error %d:%s\n",mysql_errno(_mysql), mysql_error(_mysql));
    }
    //---------------------------------------------------------------------

    dev_name_index++;
    if (dev_name_index < 24){
        goto SMARTCTL;
    }

    END:
    mysql_close(_mysql);
    return 0;
}
#endif