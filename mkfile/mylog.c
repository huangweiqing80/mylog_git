#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include <sys/types.h>
#include <unistd.h>
#include <malloc.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>


#include <syslog.h>
#include <sys/klog.h>

#include <pthread.h>

#define BUF_SIZE 1000
#define Log_Path "/home/Log_hwq/"
#define KLOG_SIZE_BUFFER   10

char new_file[50];
char buf[BUF_SIZE];
void makeFile();
void *write_log(void *arg);



void *write_log(void *arg)
{
	int readSize = 0,saveSize = 0;
	int klog_buf_len = 0;
	int open_fd;
	makeFile();
	open_fd = open(new_file,O_RDWR | O_CREAT,0755);
	while(1)
	{
		/*klog_buf_len = klogctl(KLOG_SIZE_BUFFER , 0, 0);
		if (klog_buf_len <= 0) {
        		perror("klogctl no dataup");
    		}*/
		readSize = klogctl(2,buf+saveSize,BUF_SIZE-saveSize);
		printf("%d\n",readSize);
		//write(open_fd,buf,BUF_SIZE);
		if(0>=readSize)
			perror("klogctl error");
		else
		{
			
			saveSize += readSize; 
			if(saveSize>=BUF_SIZE)
			{
				//write_logfile();
				write(open_fd,buf,BUF_SIZE);
				printf("now write log to file\n");
				readSize = 0;
				saveSize = 0;
				memset(buf,0,sizeof(buf));
			}
		}
	}
}
void makeFile()
{
	time_t ctime;
	struct tm *tm;
	
	memset(buf,'\0',sizeof(buf));
	umask(0);//屏蔽创建文件权限
	int fd = mkdir(Log_Path,777);	
	if((fd < 0) && (errno != EEXIST))
	{
		perror("mkdir file erro");
	}

	ctime = time(NULL);
	
	tm = localtime(&ctime);
	sprintf(new_file,"%s%2.2d_%2.2d:%2.2d:%2.2d",Log_Path,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);//将时间转换为字符串
	int fd_newfile = creat(new_file,777);//创建目标文件
}

int main()
{
	pthread_t writelog_tid;
	pthread_create(&writelog_tid,NULL,write_log,NULL);
	pthread_join(writelog_tid,NULL);
	return 0;
}

