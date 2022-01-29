#include "svr_comm_dir.h"
#include "crc_log.h"
#include "svr_utils.h"

#include <dirent.h>
#include <fcntl.h>
#include <uuid/uuid.h>

int 
SvrDir::DuplicateDir(char *dir, char *ddir)
{
    DIR *           Dp;             //打开目录指针 
	struct dirent   pre_enty ;      //文件目录结构体 
    struct dirent * enty = NULL; 
    struct stat     stat_buf;       //详细文件信息结构体 

    //打开指定的目录，获得目录指针 
    if ( (dir) && (NULL == (Dp = opendir(dir))) ) 
    { 
        CRCLog_Error("SvrDir::DuplicateDir: CAN NOT open dir <%s>", dir);
        return -1; 
    } 

	//遍历这个目录下的所有文件 
    while(1) 
    { 
		char path [PATH_MAX];
		char dpath[PATH_MAX];

		if (readdir_r(Dp,&pre_enty,&enty))
		{
			break;
 		}
		if (enty == NULL)
		{
			break;
		}
        
		//源目录 
		if (dir[strlen(dir)-1] == '/')
		{
			sprintf (path,"%s%s", dir, enty->d_name);
		}else{
			sprintf (path,"%s/%s", dir, enty->d_name);
        }

		//目标目录
		if (ddir[strlen(ddir)-1] == '/')
		{
			sprintf (dpath,"%s%s", ddir, enty->d_name);
		}else{
			sprintf (dpath,"%s/%s", ddir, enty->d_name);
        }

		//src dir stat
		if (lstat(path,&stat_buf) !=0)
		{
			CRCLog_Error("SvrDir::DuplicateDir: fail lstat %s",path);
			perror("lstat:");
		}

		if (S_ISBLK(stat_buf.st_mode))
		{
			CRCLog_Info("SvrDir::DuplicateDir: B %s",path); 
		}else if (S_ISCHR(stat_buf.st_mode))
		{
			CRCLog_Info("SvrDir::DuplicateDir: C %s",path);
		}else if (S_ISDIR(stat_buf.st_mode))
		{
			//当前目录和上一目录过滤掉 
            if (0 == strcmp(".",enty->d_name) || 
				0 == strcmp("..",enty->d_name)) 
            { 
                continue; 
            }

			//输出当前目录名 
		    CRCLog_Debug("SvrDir::DuplicateDir: D,%s",path); 
		    CRCLog_Debug("SvrDir::DuplicateDir: DEST,D,%s",dpath); 
            
			if (access(dpath,F_OK)!=0)
			{
				if (mkdir(dpath,stat_buf.st_mode|S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP ) !=0)
				{
					perror("mkdir:");
				}
			}
			
			//继续递归调用    
			int ret = DuplicateDir(path,dpath);
			if (0 != ret){
				closedir(Dp); 
				Dp = NULL;
				return ret;
			}
		}else if (S_ISFIFO(stat_buf.st_mode))
		{
			CRCLog_Info("SvrDir::DuplicateDir: S_ISFIFO D %s",path); 
		}else if (S_ISREG(stat_buf.st_mode))
		{
            CRCLog_Debug("SvrDir::DuplicateDir: R,%s",path); 
            CRCLog_Debug("SvrDir::DuplicateDir: DEST,R,%s",dpath); 
            CRCLog_Debug("SvrDir::DuplicateDir: R,%s,%llu",path,stat_buf.st_size); 
			
			if (access(dpath,F_OK)!=0)
			{
				int fd = 0;
				if ((fd =  open(dpath,O_RDWR|O_CREAT,stat_buf.st_mode|S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP ))==-1)
				{
					perror("open:");
					if(Dp){closedir(Dp);}
					Dp=NULL; 
					return -2;
				}
				if (stat_buf.st_size > 0)
				{
					if (0 != ftruncate(fd,stat_buf.st_size)){
						perror("ftruncate:");
						close(fd);
						fd=0; 
						if(Dp){closedir(Dp);}
						Dp=NULL;  
						return -3;
					}
				}
				if (fd){  
					close(fd);
					fd=0;
				}
			}
            //sprintf(cmd_buff,"truncate --size=%d  '/data/tmp/%s'",stat_buf.st_size,path+len);
            //system(cmd_buff);
		
		}else if (S_ISLNK(stat_buf.st_mode))
		{
			CRCLog_Info("SvrDir::DuplicateDir: L %s",path);
		}else if (S_ISSOCK(stat_buf.st_mode))
		{
			CRCLog_Info("SvrDir::DuplicateDir: S %s",path);
		}
        //if (pre_enty)
        //{
        //	free(pre_enty);
        //	pre_enty = NULL;
        //}
    } 
	
	//关闭文件指针 
    if (Dp){ 
    	closedir(Dp);
    	Dp=NULL;
    }

	return 0;
}

int 
SvrDir::ParseR(FILE *dfile, char*ddir, char*prefix, int splitcnt, char*labe_prefix)
{
	char rfile_line[4096];
	char ddir_name[4096];
	char split_index_c[12];
	int  i,split_index=0;

	if (!dfile&&!ddir)
	{
		return -1;
	}

	//check dest dir
	if (access(ddir,F_OK)!=0)
	{
		if (mkdir(ddir,S_IXUSR|S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP ) != 0)
		{
			perror("mkdir:");
			return -1;
		}
	}

	if ( (ddir[strlen(ddir)-1] == '/') && (strlen(ddir)>0) )
	{
		size_t i;
		for (i = 0;i<strlen(ddir);i++)
		{
			CRCLog_Info("SvrDir::ParseR: ddir %c",ddir[i]);
		}
		
	}
	if (splitcnt > 1)
	{
		for (i = 0;i<splitcnt;i++)
		{
			sprintf(ddir_name,"%s/%s(%d-%d)",ddir,labe_prefix,splitcnt,i+1);
		}
	}
	
	memset(split_index_c,0,sizeof(split_index_c));
	fseek(dfile,0,SEEK_SET);
	while(!feof(dfile))
    {
		if (!fgets(rfile_line,sizeof(rfile_line),dfile))
		{
			break;
		}
		if (!strncmp(rfile_line,"R,",2))
		{
			//printf("%s",dfile_line+2+strlen(prefix));
			/* del /n */ 
			if (rfile_line[strlen(rfile_line)-1] == '\n')
			{
				rfile_line[strlen(rfile_line)-1] = 0;
				if (splitcnt > 1)
				{
					strncpy(split_index_c,strchr(rfile_line,',')+1,4);
					split_index = atoi(split_index_c)+1;
					sprintf(ddir_name,"%s/%s(%d-%d)",ddir,labe_prefix,splitcnt,split_index);
				}else{
					sprintf(ddir_name,"%s/%s",ddir,labe_prefix);
				}
				if (smv(rfile_line+7,ddir_name,prefix)!=0)
				{
					CRCLog_Error("SvrDir::ParseR: CAN NOT CREATE %s ,ddirname %s, prefix %s",rfile_line,ddir_name,prefix);
					break;
					return -1;
				}
			}
		}	
	}
	return 0;    
}

int
SvrDir::ParseD(FILE *dfile,char*ddir,char*prefix)
{
	char dfile_line[4096];
	if (!dfile&&!ddir)
	{
		return -1;
	}
	/* check dest dir */
	if (access(ddir,F_OK)!=0)
	{
		if (mkdir(ddir,S_IXUSR|S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP ) !=0)
		{
			perror("mkdir:");
			return -1;
		}
	}
	fseek(dfile,0,SEEK_SET);
	while(!feof(dfile))
    {
		if (!fgets(dfile_line,sizeof(dfile_line),dfile))
		{
			break;
		}
		if (!strncmp(dfile_line,"D,",2))
		{
			//printf("%s",dfile_line+2+strlen(prefix));
			/* del /n */ 
			char *p =NULL;
			p = strchr(dfile_line,'\n');
			if (p)
			{
				*p = '\0';
				if (smkdir(dfile_line+2+strlen(prefix),ddir)!=0)
				{
					CRCLog_Error("SvrDir::ParseD: CAN NOT CREATE %s ",dfile_line);
					break;
					return -1;
				}
			}
		}	
	}
	return 0;    
}

unsigned long long 
SvrDir::ConvertMediaTypeToSplitSize(char *cdtype)
{
	unsigned long long split_size = 0;
	if (!cdtype)
	{
		return 0;
	}
	if (!strncmp(cdtype,"BD100",5))
	{
		split_size = SVRUTILS::BD100SIZE;
	}else if (!strncmp(cdtype,"BD50",4))
	{	
		split_size = SVRUTILS::BD50SIZE;
	}
	else if (!strncmp(cdtype,"BD25",4))
	{	
		split_size = SVRUTILS::BD25SIZE;
	}
	else if (!strncmp(cdtype,"DVD",3))
	{	
		split_size = SVRUTILS::DVD4SIZE;
	}
	else if (!strncmp(cdtype,"CDROM",5))
	{	
		split_size = SVRUTILS::CDROMSIZE;
	}else
	{
		split_size = SVRUTILS::DVD4SIZE;
	}
	return split_size;    
}

int 
SvrDir::SplitDir(char *srcdir, char *destdir, char*label, char*cdtype)
{
	int fcnt=0;
	unsigned long long i;
	uuid_t uuid;
	unsigned long long size = 0;
	FILE *dfile=NULL;
	FILE *rfile=NULL;
	char dest_path_dir[4096],uuid_str[36+1];
	char dfile_tmp[512];
	char rfile_tmp[512];
	unsigned long long spsize;
	
	unsigned long long spcnt=0,spbase=0;
	uuid_generate(uuid);
	uuid_unparse(uuid, uuid_str);

	//get split size
	spsize = ConvertMediaTypeToSplitSize(cdtype);
	
	sprintf(dfile_tmp,"/tmp/D%s.csv",uuid_str);
	sprintf(rfile_tmp,"/tmp/R%s.csv",uuid_str);

	dfile = fopen(dfile_tmp,"w+");
	rfile = fopen(rfile_tmp,"w+");
	if (SummaryDir(srcdir,&fcnt,&size,dfile,rfile,spsize,&spcnt,&spbase))
	{
		return -1;
	}
	CRCLog_Info("SvrDir::SplitDir: %s ,file cnt:%d,sum :%llu ,spcnt %llu \n",srcdir,fcnt,size,spcnt+1);
	if (spcnt > 0)
	{	
		for (i = 0;i<spcnt+1;i++)
		{
			sprintf(dest_path_dir,"%s/%s(%d-%d)",destdir,label,(int)spcnt+1,(int)i+1);
			ParseD(dfile,dest_path_dir,srcdir);
		}
		ParseR(rfile,destdir,srcdir,spcnt+1,label);
	}
	if (dfile){
		fclose(dfile);
		dfile=NULL;
	}
	if (rfile){
		fclose(rfile);
		rfile=NULL;
	}
	return spcnt;   
}

int 
SvrDir::SummaryDir(char* dir,
				   int * fcnt, unsigned long long *bsize,
				   FILE *d_file, FILE *r_file,
				   unsigned long long splitsize, unsigned long long *splitcnt, unsigned long long *split_base) 
{
	DIR *Dp;                    //打开目录指针 
	struct dirent pre_enty;     //文件目录结构体 
    struct dirent *enty=NULL; 
    struct stat stat_buf;       //详细文件信息结构体 

    if (!bsize&&!fcnt)
    {
		return -1;
    }

	//打开指定的目录，获得目录指针 
    if((dir)&&(NULL == (Dp = opendir(dir)))) 
    { 
        CRCLog_Error("SvrDir::SummaryDir: CAN NOT OPEN dir:%s\n",dir); 
		return -1; 
    } 
	
	//遍历这个目录下的所有文件 
    while(1) 
    { 
		char path[4096];

		if (readdir_r(Dp,&pre_enty,&enty))
		{
			break;
 		}
		if (enty == NULL)
		{
			break;
		}

		//源目录
		if (dir[strlen(dir)-1] == '/')
		{
			sprintf (path,"%s%s", dir, enty->d_name);
		}else{
			sprintf (path,"%s/%s", dir, enty->d_name);
        }
		
		/* src dir stat */
		if (lstat(path,&stat_buf) !=0)
		{
		    CRCLog_Error("SvrDir::SummaryDir: fail lstat %s\n",path);
			perror("lstat:");
		}
		if (S_ISBLK(stat_buf.st_mode))
		{
			CRCLog_Info("SvrDir::SummaryDir: B %s\n",path); 
		}else if (S_ISCHR(stat_buf.st_mode))
		{
			CRCLog_Info("SvrDir::SummaryDir: C %s\n",path);
		}else if (S_ISDIR(stat_buf.st_mode))
		{
			//当前目录和上一目录过滤掉 
            if (0 == strcmp(".",enty->d_name) || 
				0 == strcmp("..",enty->d_name)) 
            { 
                continue; 
            }
			//输出当前目录名 
			CRCLog_Debug("SvrDir::SummaryDir: D,%s\n",path); 
			if (d_file)
			{
				fprintf(d_file,"D,%s\n",path);
			}
			//继续递归调用    
			SummaryDir(path,fcnt,bsize,d_file,r_file,splitsize,splitcnt,split_base); 
		}else if (S_ISFIFO(stat_buf.st_mode))
		{
			CRCLog_Info("SvrDir::SummaryDir: F %s\n",path); 
		}else if (S_ISREG(stat_buf.st_mode))
		{
			unsigned long long  true_byte=0;
		    CRCLog_Debug("SvrDir::SummaryDir: R,%s\n",path); 
			//check overlap size
			if ((stat_buf.st_size > (long long)splitsize)&&(r_file))
			{
				printf("XStorageSumDir:%s is too big ,can't split!\n",path);
				return -1;
			}
			//round size 4096
			true_byte = ROUDNUP(stat_buf.st_size,4096l);

			//total size
			(*bsize)+=true_byte;
			(*fcnt) += 1;
			
		    CRCLog_Debug("SvrDir::SummaryDir: R,%s,%llu,truebyte:%llu\n",path,stat_buf.st_size,true_byte); 
			if (r_file)
			{
				if ((true_byte + *split_base) > splitsize )
				{
					(*splitcnt)++;
					(*split_base) = true_byte;
				}else{
					(*split_base) +=true_byte;
				}
				fprintf(r_file,"R,%04llu,%s\n",*splitcnt,path);
				CRCLog_Debug("SvrDir::SummaryDir: R,%04llu,%s,%llu,%llu,%llu\n",*splitcnt,path,*bsize,*split_base,true_byte);

			}
		
		}else if (S_ISLNK(stat_buf.st_mode))
		{
			CRCLog_Info("SvrDir::SummaryDir: L %s\n",path);
		}else if (S_ISSOCK(stat_buf.st_mode))
		{
			CRCLog_Info("SvrDir::SummaryDir: S %s\n",path);
		}
    } 
	
	//关闭文件指针 
    if (Dp){closedir(Dp);}
	if (r_file)
	{
		fflush(r_file);
	}
	if (d_file)
	{
		fflush(d_file);
	}
	return 0;
}

int 
SvrDir::smkdir(char *dirname,char *ddir)
{
	char workdir[8192];
	char *token = NULL;
	char fullpath[8192];
	if (!dirname&&!ddir)
	{
		return -1;
	}
	/* check path */
	if (ddir[0]!='/')
	{
		CRCLog_Info("SvrDir::mkdir: %s :Must absolute Path!",ddir);
		return -1;
	}
	/* generate dest dir */
	if (access(ddir,F_OK)!=0)
	{
		if (mkdir(ddir,S_IXUSR|S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP ) !=0)
		{
			perror("mkdir:");
			return -1;
		}
	}

	/* must copy ,strtok will destory orign string */
	memset(workdir,0,sizeof(workdir));   
	strncpy(workdir,dirname,sizeof(workdir)-1);   
	/* generate full path */
	memset(fullpath,0,sizeof(fullpath));   
	strncpy(fullpath,ddir,sizeof(fullpath)-1);   
	if (fullpath[strlen(fullpath)-1] != '/')
	{
		strcat(fullpath,"/");
	}
	
	CRCLog_Debug("SvrDir::mkdir: before :\n%s \n",workdir);
	/* first token */
	token = strtok( workdir, "/" );
	if (token)
	{
		CRCLog_Debug("SvrDir::mkdir: %s ",token);
		/* link token to fullpath */
		strcat(fullpath,token);
		if (fullpath[strlen(fullpath)-1] != '/')
		{
			strcat(fullpath,"/");
		}
		/* mk token path */
		if (access(fullpath,F_OK)!=0)
		{
			if (mkdir(fullpath,S_IXUSR|S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP ) !=0)
			{
				perror("mkdir:");
				return -1;
			}
			
		}
	    CRCLog_Debug("SvrDir::mkdir: fullpath :%s",fullpath);
	}
	/* next token */
	while( token != NULL )
	{
		token = strtok( NULL, "/");
		if (token)
		{
		    CRCLog_Debug("SvrDir::mkdir: %s ",token);
			/* link token to fullpath */
			strcat(fullpath,token);
			if (fullpath[strlen(fullpath)-1] != '/')
			{
				strcat(fullpath,"/");
			}
			/* mk token path */
			if (access(fullpath,F_OK)!=0)
			{
				if (mkdir(fullpath,S_IXUSR|S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP ) !=0)
				{
					perror("mkdir:");
					return -1;
				}
			}
		    CRCLog_Debug("SvrDir::mkdir: fullpath :%s",fullpath);
		}

	}
    CRCLog_Debug("SvrDir::mkdir: end\n")
	return 0;
}

int 
SvrDir::smv(char *rname, char *ddir,char *prefix)
{
	char fullpath[8192];
	if (!rname&&!ddir&&!prefix)
	{
		return -1;
	}
	/* check path */
	if (ddir[0]!='/')
	{
	    CRCLog_Debug("SvrDir::mv: %s :Must absolute Path!\n",ddir);
		return -1;
	}
	/* check path */
	if (rname[0]!='/')
	{
	    CRCLog_Debug("SvrDir::mv: %s :Must absolute Path!\n",rname);
		return -1;
	}
	/* generate dest dir */
	if (access(ddir,F_OK)!=0)
	{
		if (mkdir(ddir,S_IXUSR|S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP ) !=0)
		{
			perror("mkdir:");
			return -1;
		}
	}
	/* generate full path */
	memset(fullpath,0,sizeof(fullpath));    //V3.2.1 20180614
	strncpy(fullpath,ddir,sizeof(fullpath)-1);   //V3.2.1 20180614
	if (fullpath[strlen(fullpath)-1] != '/')
	{
		strcat(fullpath,"/");
	}
	/* cat the rname R,xxxx */
	strcat(fullpath,rname+strlen(prefix));
    CRCLog_Debug("SvrDir::mv: mv %s %s\n",rname,fullpath);
	if (access(fullpath,F_OK)!=0)
	{
		if (rename(rname,fullpath))
		{
			CRCLog_Error("SvrDir::mv: rname %s,fullpath %s\n",rname,fullpath);
			perror("rename:");
			return -1;
		}else
		return 0  ;
	}else{
		CRCLog_Info("SvrDir::mv: fullpath %s alread exist!\n",fullpath);
		return 0;
	}
	    
}