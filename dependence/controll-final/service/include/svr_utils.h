#ifndef _SVR_UTILS_H_
#define _SVR_UTILS_H_

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h>
#include <pthread.h>

namespace SVRUTILS
{
    #define MAX_SIZE        1024
    #define MAXMEDIA        12*50
    #define facto           98llu
    #define ROUDNUP(x,y)    (((x+(y-1))/y)*y)

    /* type size */
    const unsigned long long    BD128SIZE =	(128001769472l  * facto) / 100llu;
    const unsigned long long    BD100SIZE =	(100103356416l  * facto) / 100llu;
    const unsigned long long	BD50SIZE  =	(50050629632l   * facto) / 100llu;
    const unsigned long long	BD25SIZE  =	(25025314816l   * facto) / 100llu;
    const unsigned long long	DVD4SIZE  =	(4706074624l    * facto) / 100llu;
    const unsigned long long	CDROMSIZE =	(629145600l     * facto) / 100llu;

    static char MAINPATH[MAX_SIZE];
    static char LOGPATH[MAX_SIZE];
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

    static char *StrRpl(const char *in, char *out, int outlen, const char *src, const char *dst);

    static int GetMD5ValueOfFile(const char* filename, char* result, int len)
    {
        char cmd[1024];
        char line[1024];
        char result_file[1024];
        snprintf(result_file, sizeof(result_file), "/tmp/md5sum-%ld.txt", pthread_self());

        if(len <= 32){
            printf("result space should be larger than 32, but is %d\n", len);
            return -1;
        }

        char filename2[1024];

        StrRpl(filename, filename2, 1024, "\"", "\\\"");

        snprintf(cmd,sizeof(cmd), "md5sum \"%s\" > %s", filename2, result_file);
        if(SystemExec(cmd) != 0){
            printf("md5sum failed,%s\n", filename);
            return -2;
        }

        FILE* f = fopen(result_file, "r");
        if(f == NULL){
            printf("open result failed,%s\n", result_file);
            return -3;
        }
        memset(line, 0, 1024);
        fgets(line, 1024, f);
        if (f){
            fclose(f);
            f=NULL;
        }
        if(strlen(line) < 32){
            printf("the result is incorrect,%s\n", result);
            return -4;
        }

        line[32] = '\0';

        memset(result,0,len);   
        strncpy(result, line,len-1);     

        remove(result_file);

        return 0;
    }

    static char *StrRpl(const char *in, char *out, int outlen, const char *src, const char *dst)  
    {  
        const char *p = in;
        char *pOut = out;
        //unsigned int  len = outlen - 1;  
        int src_len, dst_len;
        memset(out, 0, outlen);

        // 这几段检查参数合法性  
        if((NULL == src) || (NULL == dst) || (NULL == in) || (NULL == out))  
        {  
            return NULL;  
        }  
        
        if((strcmp(in, "") == 0) || (strcmp(src, "") == 0))  
        {
            memset(out,0,outlen);  
                strncpy(out, in,outlen-1);  
            return out;  
        }

        if(outlen <= 0)  
        {  
            return NULL;  
        }

        src_len = strlen(src);
        dst_len = strlen(dst);

        while(*p != '\0')
        {  
            if(strncmp(p, src, src_len) == 0)  
            {
                if(dst_len + pOut > out + outlen - 1){
                    printf("out buffer overflow, %d, %ld.\n", outlen, pOut-out+dst_len);
                    return NULL;
                }
                
                strncpy(pOut, dst,dst_len);

                //printf("replace %s with %s, %s\n", src, dst, out);

                pOut += dst_len;
                p += src_len;  
            }  
            else  
            {
                *pOut = *p;  
    
                p++;
                pOut ++;
            }
        }
    
        return out;  
    }

    static int CheckMd5(const char* root)
    {
        int ret;
        char listfile[PATH_MAX];
        snprintf(listfile,sizeof(listfile), "%s/.midas_file_list", root);

        if(access(listfile, F_OK) != 0){
            printf("List file not Found %s\n", listfile);
            return 1;
        }

        FILE* f = fopen(listfile, "rb");
        if(f == NULL){
            printf("open %s failed\n",listfile); 
            return 2;
        }

        char sub_path[PATH_MAX];
        while(1){
            if(NULL == fgets(sub_path,sizeof(sub_path),f)){
                break;
            }
            char* p = NULL;
            char full_path[PATH_MAX];
            char md5value1[128];
            char md5value2[128];
            //long lSize1, lSize2;

            if (strlen(sub_path)>0)
            {
                /* cut media type \n */
                p = strchr(sub_path,'\n');
                if (p) *p= '\0';
                p = strchr(sub_path,'\r');
                if (p)	*p= '\0';
            }

            p = strrchr(sub_path, ',');
            if(p == NULL){
                printf("invalid row %s\n", sub_path);
                ret = 2;
                goto LAB_RET;
            }

            memset(md5value1,0,sizeof(md5value1));     
            strncpy(md5value1, p+1,sizeof(md5value1)-1);  

            *p= '\0';

            p = strrchr(sub_path, ',');
            if(p == NULL){
                printf("invalid row %s\n", sub_path);
                ret = 2;
                goto LAB_RET;
            }

            //lSize1 = strtol(p+1, NULL, 10 );

            *p= '\0';

            p = strrchr(sub_path, ',');
            if(p == NULL){
                printf("invalid row %s\n", sub_path);
                ret = 2;
                goto LAB_RET;
            }
            
            *p= '\0';

            *(p-1) = '\0';

            char sub_path2[1024];
            StrRpl(&sub_path[1], sub_path2, 1024, "\"\"", "\"");

            snprintf(full_path,sizeof(full_path), "%s/%s", root, sub_path2);

            //printf("%s,%s\n", full_path, md5value1);

            if(0 != GetMD5ValueOfFile(full_path,md5value2, sizeof(md5value2))){
                printf("Get md5 value failed %s\n", full_path);
                ret = 2;
                goto LAB_RET;
            }

            if(strcmp(md5value1, md5value2) != 0){
                printf("MD5 values not match %s,%s,%s\n", full_path, md5value1, md5value2);
                ret = 2;
                goto LAB_RET;
            }
        }

        ret = 0;
        
    LAB_RET:
        if(f != NULL){
            fclose(f);
            f=NULL;
        }
        return ret;
    }

}

#endif //!_SVR_UTILS_H_