#include "svr_machine_printer.h"
#include "svr_utils.h"

int 
SvrPrinter::GetPrintStatus()
{
	char ret_buf[1024];
	char cmd_buf[1024];
	FILE *pfile = NULL;

	system("echo 0 >/tmp/hp.list");
	sprintf(cmd_buf,"hp-info </tmp/hp.list 1>/tmp/hp.info 2>/dev/null");
	system(cmd_buf);
	pfile=fopen("/tmp/hp.info","r");
	if (pfile ==NULL)
	{
		return -1;
	}
	m_printer.status = 0;
	while(fgets(ret_buf,sizeof(ret_buf),pfile))
    {
	    //	printf("retburr = %s\n",ret_buf);
		if (strstr(ret_buf,"agent1-level "))
		{
			m_printer.tri_color_ink_level =  atoi(ret_buf+15);
			//printf("XPrint.tri_color_ink_level %d \n",m_printer.tri_color_ink_level);
		}else if (strstr(ret_buf,"agent2-level "))
		{
			m_printer.black_color_ink_level =  atoi(ret_buf+15);
		    //	printf("XPrint.black_color_ink_level %d \n",m_printer.black_color_ink_level);
		}else if (strstr(ret_buf,"cups-printers"))
		{
			sprintf(m_printer.model,"%s",strchr(ret_buf,'['));
		    //	printf("XPrint.model %s \n",m_printer.model);
		}else if (strstr(ret_buf,"Idle")||(strstr(ret_buf,"In power save mode")))
		{
			m_printer.status = 1;
		    //	printf("XPrint.status %d \n",m_printer.status);
		}
		else if(strstr(ret_buf,"error-state ")){
			m_printer.error_state =  atoi(ret_buf+14);
		    //	printf("XPrint.error_state %d \n",m_printer.error_state);
		}
		else if(strstr(ret_buf,"status-code ")){
			m_printer.status_code =  atoi(ret_buf+14);
		    //	printf("XPrint.error_state %d \n",m_printer.error_state);
		}
		else if(strstr(ret_buf,"status-desc ")){
			strcpy(m_printer.status_desc, ret_buf+14);
		    //	printf("XPrint.error_state %d \n",m_printer.error_state);
		}

	}
	if (pfile)
	{
		fclose(pfile);
	}
	return 0;    
}

int 
SvrPrinter::GetPrintStatusEx(const char* doc_name)
{
	char        cmd_buff[1024];
	char        job_name[512];
	const char  sh_file[]    = "/jukebox/check_print_job.sh";
	const char  sh_content[] = "VAR=$(lpq |grep $1 | wc -l)\nexit $VAR\n";
	char        data[1024];
	int         gen_sh = 0;

	if(access(sh_file, 0) != 0){
		gen_sh = 1;
	} else {
		FILE* f = fopen(sh_file,"rb");
		if(f != NULL){
			memset(data, 0, sizeof(data));
			fread(data, 1, 1024, f);
			fclose(f);
			if(strcmp(data, sh_content) != 0){
				gen_sh = 1;
			}
		}
		else{
			gen_sh = 1;
		}
	}

	if(gen_sh)
    {
		FILE* f = fopen(sh_file,"wb");
		if(f == NULL){
			printf("create sh script [%s] failed\n",cmd_buff);
			return -1;
		}

		if(1 != fwrite(sh_content, strlen(sh_content), 1, f)){
			fclose(f);
			printf("write sh script [%s] failed\n",cmd_buff);
			return -2;
		}
		fclose(f);

		sprintf(cmd_buff, "chmod 777 %s", sh_file);
		SVRUTILS::SystemExec(cmd_buff);
	}

	strcpy(job_name, doc_name);

	if(strlen(job_name) > 31){
		job_name[31] = '\0';
	}

	sprintf(cmd_buff,"%s \"%s\"", sh_file, job_name);
	int ret = SVRUTILS::SystemExec(cmd_buff);
	printf("%s,%d\n",cmd_buff,ret);
	return ret;    
}