#ifndef _SVR_UTILS_H_
#define _SVR_UTILS_H_

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

namespace SVRUTILS
{
    #define MAX_SIZE    1024
    #define MAXMEDIA    12*50

    static char MAINPATH[MAX_SIZE];
    static char LOGPATH[MAX_SIZE];
    static char LOGSUBPATH[MAX_SIZE];
    static char TMPPATH[MAX_SIZE];
    static char CACHEPATH[MAX_SIZE];
    static char MIRRORPATH[MAX_SIZE];
    static char DINFOPATH[MAX_SIZE];
    static char MAPPATH[MAX_SIZE];
    static char TRASHPATH[MAX_SIZE];
    static char XMLPATH[MAX_SIZE];
    static char DELAYPATH[MAX_SIZE];
    static char FTPPATH[MAX_SIZE];

    static unsigned char DISC_CHECK_BAD_TIMES[MAXMEDIA] = {0};

    static int SystemExec( char* szCmd)
    {
        int status=-1;
        if (szCmd)
        {
            printf("%s\n", szCmd);
            status = system(szCmd);
        }else{
            return -1;
        }
        if(WIFEXITED(status))
        {
            return 0;
        }else if(WIFSIGNALED(status))
        {
            printf("abnormal termination,signal number =%d:cmd:%s\n",
                WTERMSIG(status),szCmd);
            return -2;
        }else{
            printf("abnormal termination,exit status =%d:cmd:%s\n",
                status,szCmd);
            return -3;
        }
    }

    static int CheckDirIsExist(char *dir){
        int ret  = -1;
        if (access(dir,F_OK)!=0)
        {
            if (mkdir(dir,S_IRWXU|S_IROTH|S_IRWXG) !=0)
            {
                perror("mkdir:");
                ret =  -1;
            }else{
                ret  = 0;
            }
        }else{
            ret = 0;
        }
        return ret ;
    }

    static char* crc_strncpy(char* dest, const char* src, size_t destlen)
    {
        size_t ns   = strlen(src);
        if (destlen>ns){
            strncpy(dest, src, ns);
            dest[ns] = '\0';
        } else {
            strncpy(dest, src, destlen-1);
            dest[destlen-1] = '\0';
        }
        return dest;
    }

    static bool GetMainPath(char* err, size_t len)
    {
        int i;
        char cmd_buff[256];
        char current_absolute_path[MAX_SIZE] = {0};

        //获取当前程序绝对路径
        int cnt = readlink("/proc/self/exe", current_absolute_path, MAX_SIZE);
        if (cnt < 0 || cnt >= MAX_SIZE)	
        {
            crc_strncpy(err,"readlink", len);
            return false;
        }
        //获取当前目录绝对路径，即去掉程序名
        for (i = cnt; i >= 0; --i)	
        {
            if (current_absolute_path[i] == '/')
            {
                current_absolute_path[i+1] = '\0';
                break;
            }	
        }
        
        printf("current absolute path:%s\n", current_absolute_path);
        crc_strncpy(MAINPATH,current_absolute_path, MAX_SIZE);
        
        snprintf(LOGPATH,sizeof(LOGPATH),"%slog",MAINPATH);
        if (0 != CheckDirIsExist(LOGPATH))      {crc_strncpy(err,"mkdir LOGPATH FAILED",len);    return false;}

        snprintf(TMPPATH,sizeof(TMPPATH),"%stmp",MAINPATH);
        if (0 != CheckDirIsExist(TMPPATH))      {crc_strncpy(err,"mkdir TMPPATH FAILED",len);    return false;}
        
        snprintf(CACHEPATH,sizeof(CACHEPATH),"%scache",MAINPATH);
        if (0 != CheckDirIsExist(CACHEPATH))    {crc_strncpy(err,"mkdir CACHEPATH FAILED",len);  return false;}
        
        snprintf(MIRRORPATH,sizeof(MIRRORPATH),"%smirror",MAINPATH);
        if (0 != CheckDirIsExist(MIRRORPATH))   {crc_strncpy(err,"mkdir MIRRORPATH FAILED",len); return false;}

        /* touch file for nfs judge is connect for app */
        snprintf(cmd_buff,sizeof(cmd_buff),"touch %s/.FSOK",MIRRORPATH);
        SystemExec(cmd_buff);

        snprintf(DINFOPATH,sizeof(DINFOPATH),"%sinfo",MAINPATH);
        if (0 != CheckDirIsExist(DINFOPATH))    {crc_strncpy(err,"mkdir DINFOPATH FAILED",len);  return false;}
        
        snprintf(TRASHPATH,sizeof(TRASHPATH),"%strash",MAINPATH);
        if (0 != CheckDirIsExist(TRASHPATH))    {crc_strncpy(err,"mkdir TRASHPATH FAILED",len);  return false;}

        snprintf(DELAYPATH,sizeof(DELAYPATH),"%sdelay",MAINPATH);
        if (0 != CheckDirIsExist(DELAYPATH))    {crc_strncpy(err,"mkdir DELAYPATH FAILED",len);  return false;}

        snprintf(FTPPATH,sizeof(FTPPATH),"%smidasftp",MAINPATH);
        if (0 != CheckDirIsExist(FTPPATH))      {crc_strncpy(err,"mkdir FTPPATH FAILED",len);    return false;}
        
        snprintf(MAPPATH,sizeof(MAPPATH),"%s/controll.map",DINFOPATH);
        snprintf(XMLPATH,sizeof(XMLPATH),"%sconfig.xml",MAINPATH);
        
        return true;
    }

}

#endif //!_SVR_UTILS_H_